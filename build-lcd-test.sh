#!/bin/sh

set -e

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -Os -ffreestanding -nostdlib -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wl,-T kseg1.lds -Wl,--gc-sections crt0.S lcd_test.c -o lcd-test.elf
~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-objcopy -O binary lcd-test.elf lcd-test.bin
