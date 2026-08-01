#!/bin/sh

set -e

~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-gcc -Os -mips32 -ffreestanding -nostdlib -ffunction-sections -fdata-sections -Wl,-T iram_3400.lds -Wl,--gc-sections crt0.S cpuinfo.c -o cpuinfo.elf
~/toolchains/mips32el--glibc--stable-2018.02-2/bin/mipsel-linux-objcopy -O binary cpuinfo.elf cpuinfo.bin

# run cpuinfo
./hcusbtool.py upload 0xbfe03400 cpuinfo.bin
./hcusbtool.py run 0xbfe03400

# grab saved registers
# 24 is the sizeof struct cpu_info
./hcusbtool.py download 0xbfe03f00 24 cpuinfo.dump
hexdump -C cpuinfo.dump

# Already captured dump has:
# PRId    = 0x0001974c (MIPS 74K rev 4.12)
# Config0 = 0x80240483 (Arch type MIPS32, Arch revision Rel2/MIPS32R2, Little-Endian)
# Config1 = 0xbea3519f (Has FPU, has EJTAG, has MIPS16e, has PerfCnt, D-cache 32kb, I-cache 32kb, 64 TLB records)
# Config2 = 0x80000000 (No L2 cache)
# Config3 = 0x00002c20 (Has CP0 UserLocal, has DSP&DSP2, no CTXTC, no VEIC, has VInt, no Small Pages, no CDMM, no Multithreading, no SmartMIPS)
# Config7 = 0x80030800 (FPU is at half speed, Prefetch 1 extra cache lines on I-cache miss)


