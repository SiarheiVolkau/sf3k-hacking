#!/bin/sh

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -Os -ffreestanding -nostdlib -ffunction-sections -fdata-sections -Wl,-T kseg1.lds -Wl,--gc-sections crt0.S linux_boot.c -o linux-boot.elf
~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-objcopy -O binary linux-boot.elf linux-boot.bin
