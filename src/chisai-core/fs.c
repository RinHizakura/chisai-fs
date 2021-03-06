#include "chisai-core/fs.h"
#include <linux/stat.h>
#include <stdbool.h>
#include "utils/align.h"
#include "utils/assert_.h"
#include "utils/log.h"
#include "utils/minmax.h"

#define pow2(v) ((v) * (v))

static unsigned long BLKGRP_SIZE = 0;

static inline size_t fs_inode_to_offset(filesystem_t *fs,
                                        chisai_size_t inode_idx)
{
    // find the offset of inode in block device
    /* remember that we have the same numbers of data block and inode for each
     * group */
    unsigned int grp_idx = (inode_idx - 1) / fs->sb.data_block_per_groups;
    unsigned int bitvec_idx = (inode_idx - 1) % fs->sb.data_block_per_groups;
    unsigned int blk_size = fs->sb.block_size;

    size_t offset = blk_size +              /* super block */
                    grp_idx * BLKGRP_SIZE + /* previous block group */
                    2 * blk_size +          /* inode bitmap and data bitmap */
                    INODE_SIZE * bitvec_idx;
    return offset;
}

static inline size_t fs_data_to_offset(filesystem_t *fs, chisai_size_t data_idx)
{
    // find the offset of data block in block device
    unsigned int grp_idx = (data_idx - 1) / fs->sb.data_block_per_groups;
    unsigned int bitvec_idx = (data_idx - 1) % fs->sb.data_block_per_groups;
    unsigned int blk_size = fs->sb.block_size;

    size_t offset = blk_size +              /* super block */
                    grp_idx * BLKGRP_SIZE + /* previous block group */
                    2 * blk_size +          /* inode bitmap and data bitmap */
                    INODE_SIZE * fs->sb.data_block_per_groups + /* inodes */
                    blk_size * bitvec_idx;
    return offset;
}

static chisai_size_t fs_inode_alloc(filesystem_t *fs)
{
    chisai_size_t inode_idx = blkgrps_inode_alloc(fs->blk_grps);
    if (inode_idx == 0)
        return NO_INODE;
    fs->sb.free_inodes--;

    return inode_idx;
}

static chisai_size_t fs_data_alloc(filesystem_t *fs)
{
    chisai_ssize_t data_idx = blkgrps_data_alloc(fs->blk_grps);
    if (data_idx == 0)
        return 0;
    fs->sb.free_blocks--;

    return data_idx;
}

static void fs_inode_release(filesystem_t *fs, chisai_size_t inode_idx)
{
    blkgrps_inode_release(fs->blk_grps, inode_idx);
    fs->sb.free_inodes++;
}

static void fs_data_release(filesystem_t *fs, chisai_size_t data_idx)
{
    blkgrps_data_release(fs->blk_grps, data_idx);
    fs->sb.free_blocks++;
}

static void fs_save_inode(filesystem_t *fs,
                          inode_t *inode,
                          chisai_size_t inode_idx)
{
    size_t offset = fs_inode_to_offset(fs, inode_idx);
    inode_save(inode, &fs->d, offset);
}

static void fs_save_dir(filesystem_t *fs, dir_t *dir, chisai_size_t data_idx)
{
    size_t offset = fs_data_to_offset(fs, data_idx);
    dir_save(dir, &fs->d, offset);
}

static void fs_create_root(filesystem_t *fs)
{
    // create an inode instance
    inode_t root_inode;
    inode_init(&root_inode);
    inode_set_mode(&root_inode,
                   S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO));  // 0777
    inode_set_nlink(&root_inode, 2);  // the two links include . and ..

    // create a directory instance
    dir_t root_dir;
    dir_init(&root_dir);

    // allocate a data block for root, and store the directory information
    chisai_size_t data_idx = fs_data_alloc(fs);
    assert_eq(data_idx, 1);
    fs_save_dir(fs, &root_dir, data_idx);

    // before we allocate inode, we preserved inode number 1 for bad block
    chisai_size_t inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, BADBLK_INODE);

    // FIXME: update inode for the modified time
    /* allocate an inode for root, add the allocated data block to the inode,
     * and store the inode information */
    inode_idx = fs_inode_alloc(fs);
    assert_eq(inode_idx, ROOT_INODE);
    inode_add_block(&root_inode, data_idx);
    fs_save_inode(fs, &root_inode, inode_idx);
}

static bool fs_find_inode(filesystem_t *fs,
                          chisai_size_t inode_idx,
                          inode_t *inode)
{
    if (!blkgrps_inode_exist(fs->blk_grps, inode_idx))
        return false;

    /* FIXME: some repeat calculation could happen between
     * blkgrp_inode_exist and fs_inode_to_offset */
    size_t offset = fs_inode_to_offset(fs, inode_idx);
    inode_load(inode, &fs->d, offset);
    return true;
}

static bool fs_find_dir(filesystem_t *fs, chisai_size_t data_idx, dir_t *dir)
{
    if (!blkgrps_data_exist(fs->blk_grps, data_idx))
        return false;

    /* FIXME: some repeat calculation could happen between
     * blkgrp_data_exist and fs_data_to_offset */
    size_t offset = fs_data_to_offset(fs, data_idx);
    dir_load(dir, &fs->d, offset);
    return true;
}

static chisai_size_t fs_path_to_inode(filesystem_t *fs,
                                      const char *path,
                                      inode_t *inode)
{
    chisai_size_t inode_idx = ROOT_INODE;
    if (!fs_find_inode(fs, inode_idx, inode))
        return NO_INODE;

    /* FIXME:
     * Since the argument __path is const, we need to copy a new one for strtok.
     * However, the copied string won't be used if the path is root(\) */
    char *duppath = strdup(path);

    // walk the parent directory from root
    char *token = strtok(duppath, "/");
    while (token != NULL) {
        // 1. try to get directory structure from inode
        dir_t dir;
        if (!fs_find_dir(fs, inode->direct_blks[0], &dir)) {
            fs->d.free(duppath);
            return NO_INODE;
        }

        // 2. get inode index of next component from directory
        inode_idx = dir_file_inode_idx(&dir, token);

        // 3. load next component's inode from its inode idx
        if (!inode_idx || !fs_find_inode(fs, inode_idx, inode)) {
            fs->d.free(duppath);
            return NO_INODE;
        }

        token = strtok(NULL, "/");
    }
    fs->d.free(duppath);
    return inode_idx;
}

void fs_init(filesystem_t *fs)
{
    unsigned int blk_size;
    // load back the superblock
    superblock_load(&fs->sb, &fs->d);
    blk_size = fs->sb.block_size;

    assert_eq(BLKGRP_SIZE, 0);
    BLKGRP_SIZE =
        (2 * blk_size) + (blk_size * BYTE_BITS) * (INODE_SIZE + blk_size);

    // load back the block group metadata
    fs->blk_grps = fs->d.malloc(sizeof(block_group_t) * fs->sb.groups);
    blkgrps_load(fs->blk_grps, &fs->d, fs->sb.block_size, fs->sb.groups);

    // check the magic number in superblock
    if (fs->sb.magic != MAGIC)
        die("Magic number=%x in superblock is not expected=%x!\n", fs->sb.magic,
            MAGIC);

    // create root directory
    if (!blkgrps_inode_exist(fs->blk_grps, ROOT_INODE))
        fs_create_root(fs);

    assert_le(sizeof(inode_t), INODE_SIZE);
    assert_le(sizeof(dir_t), blk_size);
}

int fs_get_metadata(filesystem_t *fs,
                    const char *path,
                    struct chisai_file_info *info)
{
    memset(info, 0, sizeof(struct chisai_file_info));

    chisai_size_t idx = fs_path_to_inode(fs, path, &info->inode);
    if (idx == NO_INODE)
        return CHISAI_ERR_ENOENT;

    info->idx = idx;
    return CHISAI_ERR_OK;
}

static int __fs_get_dir(filesystem_t *fs,
                        const char *path,
                        inode_t *dir_inode,
                        dir_t *dir)
{
    chisai_size_t inode_idx, data_idx;

    inode_idx = fs_path_to_inode(fs, path, dir_inode);
    if (inode_idx == NO_INODE)
        return CHISAI_ERR_ENOENT;

    // the index of directory data block
    data_idx = dir_inode->direct_blks[0];
    if (!fs_find_dir(fs, data_idx, dir))
        return CHISAI_ERR_CORRUPT;

    return CHISAI_ERR_OK;
}

int fs_get_dir(filesystem_t *fs, const char *path, struct chisai_dir_info *dir)
{
    memset(dir, 0, sizeof(struct chisai_dir_info));

    inode_t dir_inode;
    return __fs_get_dir(fs, path, &dir_inode, &dir->dir);
}

int fs_get_data(__attribute__((unused)) filesystem_t *fs,
                struct chisai_dir_info *dir,
                struct chisai_file_info *info,
                char *name)
{
    memset(info, 0, sizeof(struct chisai_file_info));

    // info of current working directory(.)
    if (dir->pos == 0) {
        info->inode.mode = S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO);
        strcpy(name, ".");
        dir->pos += 1;
        return 1;  // return 1 means we still have next file to read
    }
    // info of parent directory(..)
    else if (dir->pos == 1) {
        info->inode.mode = S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO);
        strcpy(name, "..");
        dir->pos += 1;
        return 1;
    } else if (dir->pos - 2 < dir->dir.size) {
        info->inode.mode = S_IFDIR | (S_IRWXU | S_IRWXG | S_IRWXO);
        strcpy(name, dir->dir.node[dir->pos - 2].name);
        dir->pos += 1;
        return 1;
    }
    return CHISAI_ERR_OK;
}

static int fs_path_to_parent(filesystem_t *fs,
                             const char *path,
                             inode_t *parent_inode,
                             dir_t *parent_dir,
                             char *__file_path)
{
    /* 1. Get path of the new directory itself and its parent directory. We
     * apply a small trick here to get parent_path and file_path easily, which
     * will lead them to share the same heap memory. It means that when we free
     * parent_path, file_path is also freed in the same place */
    char *parent_path = strdup(path);
    char *file_path = strrchr(parent_path, '/') + 1;
    *(file_path - 1) = '\0';
    strcpy(__file_path, file_path);
    info("path_to_parent: parent directory %s of %s \n", file_path,
         *parent_path ? parent_path : "(root)");

    /* 2. The file name can't exceed the maximum size */
    if (strlen(file_path) > CHISAI_FILE_LEN) {
        fs->d.free(parent_path);
        return CHISAI_ERR_ENAMETOOLONG;
    }

    /* 3. Find parent directory from its path. */
    int ret = __fs_get_dir(fs, parent_path, parent_inode, parent_dir);
    fs->d.free(parent_path);
    return ret;
}

int fs_mkdir(filesystem_t *fs, const char *path, mode_t mode)
{
    int ret;
    char file_path[CHISAI_FILE_LEN];
    dir_t parent_dir;
    inode_t parent_inode;

    /* 1. Get parent directory structure and the new file name from path */
    ret = fs_path_to_parent(fs, path, &parent_inode, &parent_dir, file_path);
    if (ret != CHISAI_ERR_OK)
        return ret;
    if (dir_file_inode_idx(&parent_dir, file_path))
        return CHISAI_ERR_EEXIST;

    /* 2. Allocate resource for the new directory */
    chisai_size_t new_inode_idx, new_data_idx;
    new_data_idx = fs_data_alloc(fs);
    if (new_data_idx == 0)
        return CHISAI_ERR_ENOMEM;
    new_inode_idx = fs_inode_alloc(fs);
    // TODO: free allocated data block before return
    if (new_inode_idx == NO_INODE)
        return CHISAI_ERR_ENOMEM;

    /* 3. Update the parent dir structure */
    dir_insert(&parent_dir, file_path,
               new_inode_idx);  // FIXME: recovery if insert failed
    fs_save_dir(fs, &parent_dir, parent_inode.direct_blks[0]);

    /* 4. Update the new dir inode and structure */
    inode_t new_inode;
    dir_t new_dir;

    dir_init(&new_dir);
    inode_init(&new_inode);
    inode_set_mode(&new_inode, S_IFDIR | mode);
    inode_set_nlink(&new_inode, 2);
    inode_set_size(
        &new_inode,
        0x1000);  // FIXME: the size of directory should not be hardcoded

    inode_add_block(&new_inode, new_data_idx);
    fs_save_inode(fs, &new_inode, new_inode_idx);
    fs_save_dir(fs, &new_dir, new_data_idx);
    return CHISAI_ERR_OK;
}

int fs_create_file(filesystem_t *fs,
                   const char *path,
                   mode_t mode,
                   struct chisai_file_info *file)
{
    int ret;
    char file_path[CHISAI_FILE_LEN];
    dir_t parent_dir;
    inode_t parent_inode;

    /* 1. Get parent directory structure and the new file name from path */
    ret = fs_path_to_parent(fs, path, &parent_inode, &parent_dir, file_path);
    if (ret != CHISAI_ERR_OK)
        return ret;
    if (dir_file_inode_idx(&parent_dir, file_path))
        return CHISAI_ERR_EEXIST;

    /* 2. Allocate resource for the new file */
    chisai_size_t new_inode_idx;
    new_inode_idx = fs_inode_alloc(fs);
    if (new_inode_idx == NO_INODE)
        return CHISAI_ERR_ENOMEM;

    /* 3. Update the parent dir structure */
    dir_insert(&parent_dir, file_path,
               new_inode_idx);  // FIXME: recovery if insert failed
    fs_save_dir(fs, &parent_dir, parent_inode.direct_blks[0]);

    /* 4. Update the new file inode and structure */
    file->idx = new_inode_idx;
    inode_init(&file->inode);
    inode_set_mode(&file->inode, mode);
    fs_save_inode(fs, &file->inode, new_inode_idx);
    return CHISAI_ERR_OK;
}

int fs_rename_file(filesystem_t *fs, const char *from, const char *to)
{
    /* FIXME: The rename operetion could be optimized if 'from' and 'to'
     * are under the same folder */
    int ret;
    char file_path_from[CHISAI_FILE_LEN], file_path_to[CHISAI_FILE_LEN];
    dir_t parent_dir_from, parent_dir_to;
    inode_t parent_inode_from, parent_inode_to;
    chisai_size_t inode_idx_from;

    /* take the inode from 'from' */
    ret = fs_path_to_parent(fs, from, &parent_inode_from, &parent_dir_from,
                            file_path_from);
    if (ret != CHISAI_ERR_OK)
        return ret;
    inode_idx_from = dir_file_inode_idx(&parent_dir_from, file_path_from);
    if (inode_idx_from == 0)
        return CHISAI_ERR_ENOENT;
    if (!dir_remove(&parent_dir_from, file_path_from))
        return CHISAI_ERR_CORRUPT;
    fs_save_dir(fs, &parent_dir_from, parent_inode_from.direct_blks[0]);

    /* the inode ownership is moved to 'to'
     *
     * FIXME: we should add back the removed file if fail */
    ret = fs_path_to_parent(fs, to, &parent_inode_to, &parent_dir_to,
                            file_path_to);
    if (ret != CHISAI_ERR_OK)
        return ret;
    if (!dir_insert(&parent_dir_to, file_path_to, inode_idx_from))
        return CHISAI_ERR_CORRUPT;
    fs_save_dir(fs, &parent_dir_to, parent_inode_to.direct_blks[0]);

    return CHISAI_ERR_OK;
}

static int fs_find_blk(filesystem_t *fs,
                       inode_t *inode,
                       off_t off,
                       chisai_size_t *blk_idx)
{
    unsigned int blk_size = fs->sb.block_size;
    unsigned int blk_num = off / blk_size;
    unsigned int blk_idxs_per_block = blk_size / sizeof(chisai_size_t);

    if (blk_num < DIRECT_BLKS_NUM) {
        *blk_idx = inode->direct_blks[blk_num];
    } else if (blk_num < DIRECT_BLKS_NUM + blk_idxs_per_block) {
        // 1 level indirection
        unsigned int indir_off = (blk_num - DIRECT_BLKS_NUM);
        device_data_load(&fs->d,
                         (indir_off * sizeof(chisai_size_t)) +
                             fs_data_to_offset(fs, inode->indirect_blk),
                         blk_idx, sizeof(chisai_size_t));
    } else if (blk_num < DIRECT_BLKS_NUM + blk_idxs_per_block +
                             pow2(blk_idxs_per_block)) {
        // TODO: 2 level indriection
        return CHISAI_ERR_CORRUPT;
    } else {
        return CHISAI_ERR_CORRUPT;
    }

    if (*blk_idx == 0)
        return CHISAI_ERR_ENOENT;

    // if blk_idx is valid, then the offset in such data block is calculated
    return CHISAI_ERR_OK;
}

static int fs_alloc_blk(filesystem_t *fs,
                        inode_t *inode,
                        off_t off,
                        chisai_size_t *blk_idx)
{
    unsigned int blk_size = fs->sb.block_size;
    unsigned int blk_num = off / blk_size;
    unsigned int blk_idxs_per_block = blk_size / sizeof(chisai_size_t);

    *blk_idx = fs_data_alloc(fs);

    if (*blk_idx == 0)
        return CHISAI_ERR_ENOMEM;

    if (blk_num < DIRECT_BLKS_NUM) {
        inode->direct_blks[blk_num] = *blk_idx;
    } else if (blk_num < DIRECT_BLKS_NUM + blk_idxs_per_block) {
        // 1 level indirection

        /* FIXME: We use a very naive way to clear the block area by
         * writing many zero to fill the whole space. A better alternative would
         * be using mmap + memset. */
        if (inode->indirect_blk == 0) {
            chisai_size_t zero = 0;
            inode->indirect_blk = *blk_idx;
            for (unsigned int i = 0; i < blk_idxs_per_block; i++) {
                device_data_save(&fs->d,
                                 i * sizeof(chisai_size_t) +
                                     fs_data_to_offset(fs, inode->indirect_blk),
                                 &zero, sizeof(chisai_size_t));
            }

            // the pre-allocated block is used for indirect block, allcating a
            // new one
            *blk_idx = fs_data_alloc(fs);
        }
        unsigned int indir_off = (blk_num - DIRECT_BLKS_NUM);
        device_data_save(&fs->d,
                         indir_off * sizeof(chisai_size_t) +
                             fs_data_to_offset(fs, inode->indirect_blk),
                         blk_idx, sizeof(chisai_size_t));
    } else if (blk_num < DIRECT_BLKS_NUM + blk_idxs_per_block +
                             pow2(blk_idxs_per_block)) {
        // TODO: 2 level indriection
        return CHISAI_ERR_CORRUPT;
    } else {
        return CHISAI_ERR_CORRUPT;
    }

    return CHISAI_ERR_OK;
}

int fs_write_file(filesystem_t *fs,
                  struct chisai_file_info *file,
                  const char *buf,
                  size_t size,
                  off_t off)
{
    unsigned int blk_size = fs->sb.block_size;
    unsigned int old_file_sz = inode_get_size(&file->inode);
    int ret;
    chisai_size_t blk_idx;
    off_t off_inblk;
    size_t total, wlen;

    bool overwrite = old_file_sz > off;

    /* find the first block */
    total = 0;
    while (total < size) {
        ret = fs_find_blk(fs, &file->inode, off, &blk_idx);
        if (ret == CHISAI_ERR_ENOENT) {
            info("write: block allocation\n");
            ret = fs_alloc_blk(fs, &file->inode, off, &blk_idx);
        }
        if (ret == CHISAI_ERR_ENOMEM)
            break;

        info("write: to block index: %d\n", blk_idx);

        off_inblk = off - ALIGN_DOWN(off, blk_size);
        wlen = min(size - total, (size_t) blk_size - off_inblk);
        device_data_save(&fs->d, off_inblk + fs_data_to_offset(fs, blk_idx),
                         buf + total, wlen);
        total += wlen;
        off += wlen;
    }
    if (ret != CHISAI_ERR_ENOMEM)
        assert_eq(total, size);

    inode_set_size(&file->inode, overwrite ? total : total + old_file_sz);
    fs_save_inode(fs, &file->inode, file->idx);
    return total;
}

int fs_read_file(filesystem_t *fs,
                 struct chisai_file_info *file,
                 char *buf,
                 size_t size,
                 off_t off)
{
    unsigned int blk_size = fs->sb.block_size;
    int ret;
    chisai_size_t blk_idx;
    off_t off_inblk;
    size_t total, rlen;

    total = 0;
    size = min(size, (size_t) file->inode.size);
    // assume the required will never size
    while (total < size) {
        ret = fs_find_blk(fs, &file->inode, off, &blk_idx);
        if (ret != CHISAI_ERR_OK)
            return ret;

        info("read: from block index: %d\n", blk_idx);

        off_inblk = off - ALIGN_DOWN(off, blk_size);
        rlen = min(size - total, (size_t) blk_size - off_inblk);
        device_data_load(&fs->d, off_inblk + fs_data_to_offset(fs, blk_idx),
                         buf + total, rlen);

        total += rlen;
        off += rlen;
    }

    return total;
}

int fs_truncate_file(filesystem_t *fs,
                     struct chisai_file_info *file,
                     off_t trct_size)
{
    // FIXME: the unused block should be released
    unsigned int blk_size = fs->sb.block_size;
    inode_t *inode = &file->inode;
    blkcnt_t new_blkcnt = trct_size / blk_size;
    blkcnt_t old_blkcnt = inode_get_blkcnt(inode);

    // FIXME: only truncate to smaller size is enabled now
    assert_ge(file->inode.size, trct_size);

    for (blkcnt_t idx = old_blkcnt - 1; idx > (new_blkcnt - 1); idx--) {
        assert_ne(inode->direct_blks[idx], 0);
        fs_data_release(fs, inode->direct_blks[idx]);
        inode->direct_blks[idx] = 0;
    }

    inode_set_size(inode, trct_size);
    fs_save_inode(fs, inode, file->idx);
    return CHISAI_ERR_OK;
}

int fs_remove_file(filesystem_t *fs, const char *path)
{
    int ret;
    char file_path[CHISAI_FILE_LEN];
    dir_t parent_dir;
    inode_t parent_inode;

    /* 1. Get parent directory structure and the new file name from path */
    ret = fs_path_to_parent(fs, path, &parent_inode, &parent_dir, file_path);
    if (ret != CHISAI_ERR_OK)
        return ret;

    /* FIXME: We actually iterate the directory structure twice. That's not
     * a good pratice. */
    chisai_size_t inode_idx = dir_remove(&parent_dir, file_path);
    inode_t inode;
    ret = fs_find_inode(fs, inode_idx, &inode);
    assert(ret == true);

    /* FIXME: Every allocated block for this file should be released */
    fs_data_release(fs, inode.direct_blks[0]);
    fs_inode_release(fs, inode_idx);
    fs_save_dir(fs, &parent_dir, parent_inode.direct_blks[0]);
    return CHISAI_ERR_OK;
}

void fs_destroy(filesystem_t *fs)
{
    blkgrps_destroy(fs->blk_grps);
    fs->d.free(fs->blk_grps);
}
