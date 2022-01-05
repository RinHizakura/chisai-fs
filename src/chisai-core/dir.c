#include "chisai-core/dir.h"
#include "utils/log.h"

void dir_init(dir_t *dir)
{
    memset(dir, 0, sizeof(dir_t));
}

void dir_save(dir_t *dir, device_t *d, size_t offset)
{
    device_data_save(d, offset, dir, sizeof(dir_t));
}

void dir_load(dir_t *dir, device_t *d, size_t offset)
{
    device_data_load(d, offset, dir, sizeof(dir_t));
}

chisai_size_t dir_file_inode_idx(dir_t *dir, const char *file_path)
{
    /* check if the parent directory have file of same name already */
    for (unsigned int i = 0; i < dir->size; i++) {
        if (strcmp(dir->node[i].name, file_path) == 0) {
            return dir->node[i].idx;
        }
    }
    return 0;
}

bool dir_insert(dir_t *dir, const char *file_path, chisai_size_t inode_idx)
{
    if (dir->size >= CHISAI_FILE_PER_DIR)
        return false;

    strcpy(dir->node[dir->size].name, file_path);
    dir->node[dir->size++].idx = inode_idx;
    return true;
}

chisai_size_t dir_remove(dir_t *dir, const char *file_path)
{
    for (unsigned int i = 0; i < dir->size; i++) {
        if (strcmp(dir->node[i].name, file_path) == 0) {
            chisai_size_t inode_idx = dir->node[i].idx;
            dir->size--;
            dir->node[i] = dir->node[dir->size];
            return inode_idx;
        }
    }
    return 0;
}
