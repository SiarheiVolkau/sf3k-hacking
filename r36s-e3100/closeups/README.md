# Close-up photos

Board: R36S-V2.7 (2026.01.09). Numbered pics are from a USB microscope, phone_* from my phone.

## Chips
- SoC: no part number on it, just a logo (04)
- NOR: UC25HD40, 512KB, U3/U7 (07). Same size as mtd0.
- RAM: NANYA NT5CB128M16HP-EK, 256MB DDR3 (08). Stock kernel only gives Linux about 48MB of it.
- U14: ETA9740, charger + 5V boost (09)
- SOIC-8 next to TCLK/TDI has no marking (06, 21)
- Also on the board: 8002D speaker amp, TM8211 audio DAC

## Test pads
- TDO, TCLK, TDI near the SoC (06)
- 5 unlabeled round pads next to U1 (05). Haven't measured them yet.
- KEY_L1, KEY_L2, KEY_R1, KEY_R2 on the button side (phone_04)

## Other stuff
- HDMI routing: 01, 02, 03, 18
- Back of the board: 17, 20, 23, phone_01, phone_01b
- Bottom: TYPE-C DC (charge only), 3.5mm jack J3 (also marked CVBS), OTG USB-C for data (14, 15, 16, 19)
- Two microSD slots. The right one says 主TF, the left one has no label (phone_04). The DTB only has one mmc node.
- Battery label: Li-Po 3.8V 3000mAh, 5V 7.5W, adapter 5V 1.5A (phone_11)
- SoC side with the 24MHz crystal Y1 and the MIPI-LCD connector (phone_02, phone_03)
