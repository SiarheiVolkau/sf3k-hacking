#!/bin/sh

set -e

# can be omitted
./build-ddr-bin.sh

# init ddr
./hcusbtool.py upload 0xbfe03400 ddr_init.bin
./hcusbtool.py run 0xbfe03400

# verify that it is working by uploading some
# file to ddr memory region and read it back
# then compare
./hcusbtool.py upload 0xa0000000 mtd0.dump
./hcusbtool.py download 0xa0000000 $(stat -L -c "%s" mtd0.dump) mtd0.verify.dump
md5sum mtd0.dump mtd0.verify.dump
