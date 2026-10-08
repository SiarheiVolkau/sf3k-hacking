#!/bin/sh

set -e

# can be omitted
./build-ddr-bin.sh

# init ddr
./hcusbtool.py upload 0xbfe03400 ddr_init.bin
./hcusbtool.py run 0xbfe03400

# Verify that it is working by uploading some
# file to ddr memory region and read it back
# then compare. last "download" command used
# to detect memory wrap around 128Mb.
# First two hashes MUST be the same, while
# the last one MUST differ.

TESTFILE="spinor-dumps/SF3000-E-V1.0(2026.01.06)/mtd0.dump"
# a 32Mb random file made via:
# dd if=/dev/urandom of=random_32Mb bs=1M count=32
#TESTFILE="random_32Mb"
FILESIZE=$(stat -L -c "%s" $TESTFILE)
./hcusbtool.py upload 0xa0000000 $TESTFILE
./hcusbtool.py download 0xa0000000 $FILESIZE verify1.dump
#./hcusbtool.py download 0xa8000000 $FILESIZE verify2.dump
md5sum $TESTFILE verify*.dump
rm verify*.dump
