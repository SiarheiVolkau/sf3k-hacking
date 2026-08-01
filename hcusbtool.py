#!/usr/bin/env python3

import argparse
import signal
import sys
import time
import struct
import errno

import usb.core
import usb.util

VID = 0x1CBE
PID = 0x0005

TIMEOUT = 1000

def wait_device():
    print(f"Waiting for {VID:04x}:{PID:04x} (Ctrl+C to abort)...")

    while True:
        dev = usb.core.find(idVendor=VID, idProduct=PID)
        if dev is not None:
            print("Device connected.")
            time.sleep(0.5) # for proper discovery by linux
            return dev
        time.sleep(0.1)


def open_device():
    dev = wait_device()

    try:
        if dev.is_kernel_driver_active(0):
            dev.detach_kernel_driver(0)
    except (NotImplementedError, usb.core.USBError):
        pass

    dev.set_configuration()
    cfg = dev.get_active_configuration()
    intf = cfg[(0, 0)]

    ep_out = None
    ep_in = None
    for ep in intf:
        if usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_OUT:
            ep_out = ep
        else:
            ep_in = ep

    if ep_out is None or ep_in is None:
        raise RuntimeError("Bulk endpoints not found")

    print(f"Interface : {intf.bInterfaceNumber}")
    print(f"EP OUT    : 0x{ep_out.bEndpointAddress:02x}")
    print(f"EP IN     : 0x{ep_in.bEndpointAddress:02x}")
    print(f"OUT size  : {ep_out.wMaxPacketSize}")
    print(f"IN size   : {ep_in.wMaxPacketSize}")

    return dev, ep_out, ep_in


def send_packet(ep_out, payload):
    if len(payload) > ep_out.wMaxPacketSize:
        raise ValueError("packet larger than 512 bytes")
    ep_out.write(payload, TIMEOUT)

def recv_packet(ep_in, timeout=TIMEOUT):
    return bytes(ep_in.read(ep_in.wMaxPacketSize, timeout))

def write_mem_region(addr, data):
    dev, ep_out, ep_in = open_device()
    pos = 0
    size = len(data)
    req = struct.pack(
        "<IIII",
        0x1991A0A1, # uHeadFlag: 0x1991A0A1 means memory write
        addr,       # uFlagType: start address of the memory region to write to
        size,       # uTypeLen:  length of the data to write
        0,          # uTypeCrc:  TODO calculate CRC
    );
    send_packet(ep_out, req)
    try:
        while pos < size:
            chunksz = min(size - pos, ep_out.wMaxPacketSize)
            send_packet(ep_out, data[pos:pos+chunksz])
            pos += chunksz
        crc_raw = recv_packet(ep_in)
        if len(crc_raw) != 4:
            raise RuntimeError("invalid CRC reply")
        crc, = struct.unpack("<I", crc_raw)
        print(f"CRC on device: 0x{crc:08x}")
        # TODO compare CRCs
    except usb.core.USBError as e:
        print("USB error:", e)
        print("written: ", pos)
        print("size: ", size)
        raise e


def read_mem_region(addr, size):
    dev, ep_out, ep_in = open_device()
    data = bytearray()
    try:
        while size > 0:
                chunksz = min(size, ep_in.wMaxPacketSize)
                req = struct.pack(
                        "<IIII",
                        0x1992B0B1, # uHeadFlag: 0x1992B0B1 means memory read back
                        addr,       # uFlagType: start address of the region to read
                        chunksz,    # uTypeLen:  length of the data to read back, it seems like it doesn't allow > wMaxPacketSize bytes at once. so in loop
                        0,          # uTypeCrc:  not relevant in this context
                );
                send_packet(ep_out, req)
                chunk = bytes(recv_packet(ep_in))
                if len(chunk) != chunksz:
                    print("Data length error");
                    return errno.EINVAL, b''
                data.extend(chunk)
                addr += chunksz
                size -= chunksz
    except usb.core.USBError as e:
        print("USB error:", e)
        return e.errno, b''
    return 0, data

def cmd_download(addr, size, filename):
    err, data = read_mem_region(addr, size)
    if err != 0:
        raise RuntimeError("read_mem_region failed")
    with open(filename, "wb") as file:
            file.write(data)

def cmd_upload(addr, filename):
    with open(filename, "rb") as file:
        data = file.read()
        write_mem_region(addr, data)

def cmd_run(addr, noreturn=False, timeout=TIMEOUT):
    dev, ep_out, ep_in = open_device()

    req = struct.pack(
        "<IIII",
        0x1993C0C1, # uHeadFlag: 0x1993C0C1 means execute code
        addr,       # uFlagType: start address of the BootROM
        0,          # uTypeLen:  not relevant in this context
        0,          # uTypeCrc:  not relevant in this context
    );
    send_packet(ep_out, req)
    if not noreturn:
        data = recv_packet(ep_in, timeout)
        if len(data) != 4:
            raise RuntimeError("execute code failed: invalid readback size")
        addr_back = struct.unpack("<I", data)[0]
        if addr_back != addr:
            raise RuntimeError("execute code failed: readback address mismatch")

def main():
    signal.signal(signal.SIGINT, lambda *_: sys.exit(1))

    parser = argparse.ArgumentParser()

    sub = parser.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("download")
    p.add_argument("address")
    p.add_argument("size")
    p.add_argument("filename")

    p = sub.add_parser("upload")
    p.add_argument("address")
    p.add_argument("filename")

    p = sub.add_parser("run")
    p.add_argument('--return', action=argparse.BooleanOptionalAction, default=True)
    p.add_argument("address")
    p.add_argument("timeout", nargs="?", default=str(TIMEOUT))

    args = parser.parse_args()

    if args.cmd == "download":
        cmd_download(int(args.address, 0), int(args.size,0), args.filename)
        print("done")
    elif args.cmd == "upload":
        cmd_upload(int(args.address, 0), args.filename)
        print("done")
    elif args.cmd == "run":
        cmd_run(int(args.address, 0), not getattr(args, "return"), int(args.timeout, 0))
        print("done")
    else:
        raise RuntimeError("unknown command")

if __name__ == "__main__":
    main()
