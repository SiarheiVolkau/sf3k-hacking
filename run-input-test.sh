#!/bin/sh

set -e

./build-input-test.sh

APP=input-test.bin

echo "Uploading $APP"
./hcusbtool.py upload 0xbfe03400 $APP
echo "Launching ..."
./hcusbtool.py run --no-return 0xbfe03400
