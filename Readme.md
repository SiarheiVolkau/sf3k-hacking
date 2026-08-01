# SF3000 hacking 

All actions below were done on a single SF3000 of HW version SF3000-E-V1.0(2026.01.06), apply to your device with caution.

## Stock OS

Internally SF3000 has a small SPINOR flash, which is enough for some simple loader of the full system located on removable SD-card. Stock card reports 16Gb capacity and contents of it seems scrambled - no file system is recognized in Linux and total compressed image of it takes ~11Gb. So first action was replacing stock card contents by non-encrypted image from [backup](https://github.com/Q-ta-s/q-ta-s.github.io/releases) which is booting pretty well.
Stock OS contains Linux kernel image, device-tree blob, coprocessor image, Linux root file system and UI application(s).
RootFS is based on top of busybox, which, in absence of symlinks on FAT32 file system, looks weird but later it turns out helpful in hacking.
Making modifications to device tree is not feasible because there is validation somewhere and it simply won’t boot. Also it shows that there’s no hardware UART/console and most hardware blocks controlled by coprocessor firmware while Linux uses sort of shared memory IPC to coprocessor.

## TreeFrogUI

Is a replacement for stock UI which aimed to run retroarch cores on top. It works as vulnerability in library loader of stock UI and able to execute arbitrary code, including user controlled shell script called zhijack.sh located in cubegm folder, that script was heavily utilized for hacking device later. Refer to TreeFrogUI’s installation guide to get an SD card with that software.

## BootROM

First of all I have listed all nodes in /dev folder to see what is available and there were more than I expected - `/dev/mem` is here! However, there were no devmem tool in the rootfs, luckily busybox built with devmem support, so I just need rename any other busybox executable to devmem to make it work.

Having devmem is enough to completely dump the BootROM, on MIPS it usually located at 0xBFC00000 address (0x1FC00000 physical). Size is not known, so I’ve dump large enough chunk of 128kb, it turns out that size is 32kb.

Now I can examine resulting binary with Ghidra. So there is the steps BootROM is doing in order:
* relocate or mirror itself to 0x9FC00000 address.
* Clear caches.
* UART updater routine (switchable by some bit in a register)
* SPI NAND or SPI NOR loader, depending on some bit field contents.
* USB loader as the last boot source.

However, for neither of the update routine it doesn't seem to set a pin configuration so, I suspect that it is done earlier by coprocessor core or SoC has proper bootstrap pin configuration for that.

### UART updater

This routine awaits for 0x60 symbol on UART for about 100 ms, then goes to the next boot source. Since BootROM doesn’t configure any parameters of UART or pins I decided to not focus on hacking it. It will be hard to detect proper settings though. I just asked AI to decode protocol and write a host counterpart on python language because it might be partially utilized by other update routines. You can find hcuart-loader-protocol.py script in the repository.

### SPINAND loader

NAND loader assumes that read block size is 2048 bytes and erase block size is 128kb. So it loops over first 128 erase blocks; reading first 2k of each and looking for 0xAEEAAEEA (Little-Endian) pattern at offsets 0 and 4. If there’s none - go to the next boot source (USB).

### SPINOR loader 

This loader doesn’t search for any pattern as NAND does. It just loads first 16kb to 0xBFE00000 memory region, where internal SRAM of 16k size is located. So there’s no fallback to USB. Entry point it calls afterwards is at 0xBFE00800. Let’s call that 16kb of code&data as SPL (Second Platform Loader).

Choosing NAND or NOR is done by value of some register, I suspect it is set by coprocessor to a right value or hardware is smart enough to detect connected chip without CPU intervention (unlikely).

### USB loader

TBD

## SPL

SPL is located at SPINOR flash UC25HD40, luckily it can be dumped from Stock OS with TreeFrogUI by just adding
```bash
dd if=/dev/mtd0 of=/mnt/sdcard/mtd0.dump
```
to the [zhijack.sh](zhijack.sh). After booting the device once the file will be stored on SD-card.

I've extracted [spl.bin](spl.bin) from [mtd0.dump](mtd0.dump) by 
```bash
dd if=mtd0.dump of=spl.bin bs=1 count=16k
```
on my PC later.

SPL goals are:
* provide updating mechanism
* initialize DDR memory 
* load, decrypt and execute next loader (TPL)

Structure of SPL:
* some header with configuration bits @ offset 0
* Likely .bss section till offset 0x7FF
* Entry point with descrambler (de-xor-er) the rest of SPL code and data @ 0x800
* Rest of code and data
* Unused space filled with 0’s till 16kb size.

Ghidra shows that SPL has integrated descrambler for itself, so before proceeding I restored the algorithm as a python script [deobfuscate-spl.py](deobfuscate-spl.py) and applied it to SPL binary. Now Ghidra can decode SPL entirely.

Then SPL checks if USB update routine enabled for specific interface (configuration bits) and USB update is requested by firmware reset (`reset -u` executed for reset the device).
In our case USB update routine is disabled for both interfaces, so it must be enabled first.
For doing so I've wrote small utility called `patch_mtd` which makes binary patching of the SPL directly on device.
See [patch_mtd_emable_hcprogrammer_on_usb0_usb1.c](patch_mtd_emable_hcprogrammer_on_usb0_usb1.c) and [build-patch_mtd.sh](build-patch_mtd.sh) files. I don’t know which USB is used by SF3000 so I enabled both. After that device became active on USB for a short period of time during boot. WARNING: compare your `spl.bin` with [this](spl.bin) before executing `patch_mtd` on your device - they must be the same.

### USB updater protocol 

First of all it has enumeration bug in Linux which fixed by binary patching the SPL. And at the same time I’ve increased timeout of inactivity for easier hacking of the protocol.

The protocol is very simple, it supports only 3 commands:
* write block of RAM (position, size)
* read block of RAM (position, size)
* Execute code at (address)
that is enough to run arbitrary code on the device. See [hcusbtool.py](hcusbtool.py) in the repo - it implements host side counterpart to utilize that feature.
NOTE: don't try loading to/from cacheable regions.

### DRAM initialization 

Since USB protocol activated prior DDR initialization it is required to restore DDR initialization routine from SPL. With help of Ghidra I have restored that for mine device. It turns out to be implemented as a tiny command interpreter and a list/array of commands in the data section. Exact register manipulation was restored as a C source file [nanya_nt5cb128m16fp-ek.c](nanya_nt5cb128m16fp-ek.c) how to build it you can see in [build-ddr-bin.sh](build-ddr-bin.sh) file. 

### TPL loader

TPL loaded by SPL, it is encrypted by some obscure symmetric algorithm.
With help of AI, encoding algorithm was restored, see [decrypt_tpl.py](decrypt_tpl.py).

TPL's position on flash, size and load address are stored in the SPL header.
In my case it is relatively large (0x4AB20 bytes) binary which loaded into DRAM at 0xA9E70000.
On flash it stored right after SPL.

TPL itself does:
* set some obscure bits in CP0
* disables MMU, effectively enabling plain memory model.
* clears caches
* relocates rest of itself to a new place in cacheable memory.
... and many more I didn't decode because of no intention.

## Conclusion

With recovered USB uploader it is feasible to develop fully open Linux kernel for the device but lately it still needs to be integrated into booting process somehow. Fully open kernel gives opportunities to:
* use WiFi dongles (retroachievements)
* pick well developed userspace distro (opendingux beta will definitely work as it is designed for MIPSEL devices as well)
* Use USB for MTP or Ethernet (CDC-ECM/RNDIS) for transferring files.
* use external USB controllers, including wireless.
* use USB headphones.
