#!/usr/bin/env bash

loopdev=$(cat loopdev.txt)
sudo ./build/chisai -d ${loopdev} mount
