#!/bin/sh

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -O2 -Wl,--dynamic-linker=/lib/ld.so.1 patch_mtd_enable_hcprogrammer_on_usb0_usb1.c -o patch_mtd
