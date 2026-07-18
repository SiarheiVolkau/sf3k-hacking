#!/usr/bin/env python3

import sys
import struct

XOR_KEY = 0x7E3F9C2D

XOR_START = 0x0E14
XOR_END   = 0x3210      # exclusive

def deobfuscate(data):
    if len(data) < XOR_END:
        raise ValueError(
            f"File too small ({len(data)} bytes), expected at least {XOR_END:#x}"
        )

    buf = bytearray(data)

    for off in range(XOR_START, XOR_END, 4):
        word = struct.unpack_from("<I", buf, off)[0]
        word ^= XOR_KEY
        struct.pack_into("<I", buf, off, word)

    return buf


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} spl.bin spl-dexor.bin")
        return 1

    with open(sys.argv[1], "rb") as f:
        data = f.read()

    out = deobfuscate(data)

    with open(sys.argv[2], "wb") as f:
        f.write(out)

    print(f"Processed {len(out)} bytes")
    print(f"XOR: 0x{XOR_START:04X}-0x{XOR_END-1:04X}")
    print(f"Key: 0x{XOR_KEY:08X}")

if __name__ == "__main__":
    raise SystemExit(main())
