#!/usr/bin/env bash

loopdev=$(cat loopdev.txt)
sudo chmod a+rw ${loopdev}
./build/chisai -d ${loopdev} mount
