# R36S clone - HC16xx E3100 board: NOR dump + stock kernel info

Board: **HC16E3100V20** (string in NOR; DTB model "Hichip hc16xx", `hc1600a@dbE3100v20`),
sold as "R36S V2.7", 640x480 screen. Stock firmware: H.OS / cubegm.

## NOR flash dump
Dumped from the MTD partitions on the running console (stock kernel).
Chip size 512 KiB, erase size 4 KiB.

| File | MTD | Name | Size | sha256 |
|---|---|---|---|---|
| mtd0.dump | mtd0 | nor (whole chip) | 0x80000 | 992d09d8cdd4e0b65cbe7de817c5fa4558c8b842685283d33401c6149474d419 |
| mtd1.dump | mtd1 | boot | 0x61000 | eecb6c242282c9a9ac1a6e6d7e8e932267e6d13385bf3df7984aef9f41ecd0c7 |
| mtd2.dump | mtd2 | dtb | 0x9000 | e35839d2e769071f75b8403c0e3cfe376126584658c775cf32d5cdad6d714984 |
| mtd3.dump | mtd3 | eromfs | 0x6000 | f49c978686b0415d779763bfcb1f643d7203199f285660090032701d936f7b43 |
| mtd4.dump | mtd4 | persistentmem | 0x10000 | df07a06f2d9ec5892bd691edbd8a027854b30cdfacf63888c0c4f33e0d3b8977 |
| spl.bin | - | first 16 KiB of mtd0 | 0x4000 | 6b8c8625c01f2dffee2079e28053214ad656f28b5cfc5c7baffe2b4b7d1c7be9 |

### SPL compared with this repo's spl.bin (SF3000)
Only 2 bytes differ:

| Offset | SF3000 (repo) | this E3100 |
|---|---|---|
| 0x21 | 0xab | 0xa6 |
| 0x40 | 0x01 | 0x02 |

The bytes patched by `patch_mtd_enable_hcprogrammer_on_usb0_usb1.c`
(0x30-0x31, 0x1c90-0x1c93, 0x24cc-0x24cf) are identical to the repo's spl.bin.
**Not patched yet** - the dump is the untouched factory NOR.

## Stock kernel info (`stock-kernel/`)
- `system-report.txt`: cpuinfo, meminfo, mounts, input/screen/serial/USB/MTD devices, processes and the full boot log
  (Linux 4.4.186-release, `Memory: 36368K/48708K available`)
- `kallsyms.txt`, `interrupts.txt`, `iomem.txt`
- `../stock-dtb-from-nor.dts`: the DTB from the `dtb` partition, decompiled

## USB host on the stock kernel (for reference)
Both MUSB ports (18844000.usb, 18850000.usb) work as host with the stock 4.4 kernel.
Tested with out-of-tree modules built against the hclinux-2024.02.y.2 SDK:
USB keyboard + mouse, CH340/PL2303 serial, RTL8821CU Wi-Fi/BT dongle (BL-WN650BT, 0bda:c820).

### Wi-Fi freeze
It wasn't the driver. The stock `/bin/hcdaemon` checks `/proc/net/dev` every 60 s and when it
sees an interface other than `lo` it writes the name to `/dev/ZZd2C`, and that freezes the console.
Bind-mounting an empty file over `/proc/<hcdaemon pid>/net/dev` (or just SIGSTOP on hcdaemon)
fixes it. Wi-Fi has been running for hours since (morrownr 8821cu driver).
`poweroff` gives the same blue screen because the mount goes away. Stock powers off with
`/mnt/sdcard/cubegm/powergpio` instead (from TreeFrogUI's shutdown.sh).

### Bluetooth
Works now. Firmware upload with the 4.19 btrtl failed with `command 0xfc20 tx timeout`.
The 8821C firmware is 139 chunks and the 7-bit chunk index hits 0x80 (= last chunk) at chunk 128.
The fix from 5.2: `index = (i > 0x7f) ? (i & 0x7f) + 1 : i`.
After a failed upload the chip stays stuck until you unplug the dongle, a reboot doesn't reset it.

## Teardown
Photos in `teardown/`. Microscope + phone close-ups in `closeups/`.
What's on the board:
- PCB silkscreen: **R36S-V2.7 (2026.01.09)**
- SoC: HC16xx in LQFP (top marking is just a logo)
- DDR: **NANYA NT5CB128M16HP-EK** (note: HP, not FP like the SF3000's NT5CB128M16FP-EK -
  maybe why SPL byte 0x40 differs)
- NOR: **UC25HD40** (SOP-8, U3/U7)
- Crystal Y1: 24.000 MHz
- JTAG pads labelled TDO / TCLK / TDI near the SoC and the LCD connector
- LCD: 30-pin FPC, flex marked "NV-IPS"
- Buttons VOL+ / VOL- / POWER / RST on the edges, mini-HDMI at the top, BAT connector JP9, speaker SPK
