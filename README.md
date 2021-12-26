# chisai-fs

## Introduction

The chisai-fs(which uses the pronounce of word 小さい) is a simple filsystem. Its design
is originated from [gotenkfs](https://github.com/carlosgaldino/gotenksfs) with
a few changes to avoid extra library. It doesn't have high efficiency and
there's also a lot of limitation to play with it. It is only built for learning
purposes and aims to have better readability as possible.

Currently, it is mounted in the Linux user-space by
[libfuse](https://github.com/libfuse/libfuse). This simplifies our effort for the
development of chisai-fs since we can therefore get the convenience of
the standard library, and it is also very easy to debug. However, once the filesystem
is completed, we may try to install it in kernel in the form of the kernel module.

## Usage

First of all, you should complile the program with `make` in the top
directory of chisai-fs.
```
make
```

We then format the block device which we are going to install chisai-fs.
We can use a file-backed block device with Linux's loop devices.
* For the name of ${loopdev}, you can use `losetup -a | grep disk.img | cut -d: -f1`
to find it
* **WARNING!** Beware that this operation will erase data on the block device!
Please make sure you format the correct one
```
dd if=/dev/zero of=disk.img bs=4096 count=1024
sudo losetup -fP disk.img
sudo ./build/chisai --format ${loopdev}
```

Then we can create a mountpoint and mount chisai-fs on it.
```
mkdir mount
sudo ./build/chisai ${loopdev} mount
```

If there's no message like `Error happens in fuse_main!`, we can do some
simple operation to play with it now.
* WHY simple operation? Because not every function are completed now :(
```
sudo -i
cd ${path-of-chisai-fs}/mount
echo hi > "hi.txt"
ls
cat hi.txt
```

Once you want to quit it, you can unmount and detach the loop device.
```
sudo losetup -d ${loopdev}
sudo umount -f mount
```

## Limitation and TODO to fix it
- [ ] We can not rewrite file since truncate operation is not implemented now
- [ ] We can not rename file since rename operation is not implemented now
- [ ] We naively assume a block should not always be larger than 512 bytes,
and each directory is limited to fit in a single block. For this reason, the
numbers of file or directory which can put under a directory is limited. The
length of name for each file or directory are also limited.
- [ ] The information of file's last access time, last modification time and
last status change time is actually wrong value
- [ ] The information of file's owner is actually wrong value
- [ ] No error recovery if the filesystem crash during its work

## Reference
* [littlefs-fuse](https://github.com/littlefs-project/littlefs-fuse)
* [gotenksfs](https://github.com/carlosgaldino/gotenksfs)
