#!/usr/bin/env bash

rm -rf disk.img
dd if=/dev/zero of=disk.img bs=4096 count=1024
sudo losetup -fP disk.img
loopdev=$(losetup -a | grep disk.img | cut -d: -f1)
sudo ./build/chisai format ${loopdev}
sudo losetup -d ${loopdev}
