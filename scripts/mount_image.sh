#!/usr/bin/env bash

loopdev=$(cat loopdev.txt)
sudo ./build/chisai ${loopdev} mount
sudo losetup -d ${loopdev}
sudo umount -f mount
