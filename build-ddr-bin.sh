#!/bin/sh

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -Os -ffreestanding -nostdlib -ffunction-sections -fdata-sections -Wl,-T iram_3400.lds -Wl,--gc-sections crt0.S ddr_init.c -o ddr_init.elf
~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-objcopy -O binary ddr_init.elf ddr_init.bin
