#!/bin/sh

set -e

./build-ddr-bin.sh
./build-lcd-test.sh

APP=lcd-test.bin

echo "Init DDR"
./hcusbtool.py upload 0xbfe03400 ddr_init.bin
./hcusbtool.py run 0xbfe03400

echo "Uploading $APP"
./hcusbtool.py upload 0xa0000000 $APP
echo "Launching ..."
./hcusbtool.py run --no-return 0xa0000000
