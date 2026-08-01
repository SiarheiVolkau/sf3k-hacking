#!/bin/sh

set -e

# can be done once
#./build-ddr-bin.sh
#./build-linux-boot.sh

echo "Init DDR"
./hcusbtool.py upload 0xbfe03400 ddr_init.bin
./hcusbtool.py run 0xbfe03400
echo "Loading kernel ..."
./hcusbtool.py upload 0xa1000000 vmlinuz.bin
echo "Launching kernel ..."
./hcusbtool.py upload 0xa0000000 linux-boot.bin
./hcusbtool.py run --no-return 0xa0000000
