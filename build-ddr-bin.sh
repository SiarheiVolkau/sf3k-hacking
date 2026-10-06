#!/bin/sh

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -Os -ffreestanding -nostdlib -ffunction-sections -fdata-sections -Wl,-T iram_3400.lds -Wl,--gc-sections crt0.S nanya_nt5cb128m16xp-ek.c -o ddr_init.elf
~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-objcopy -O binary ddr_init.elf ddr_init.bin
