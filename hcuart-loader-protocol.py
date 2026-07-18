#!/usr/bin/env python3

import struct
import time

import serial


class BootROM:
    MAX_TRANSFER = 131072

    def __init__(self, port, baudrate=115200, timeout=0.02):
        self.ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=timeout,
        )

    def close(self):
        self.ser.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()

    @staticmethod
    def _cmd_from_size(size):
        if size < 4 or size > BootROM.MAX_TRANSFER:
            raise ValueError("invalid transfer size")

        if size & 3:
            raise ValueError("size must be multiple of 4")

        shift = size.bit_length() - 3

        if (4 << shift) != size:
            raise ValueError("size must be exactly 4 << n")

        return shift

    @staticmethod
    def _largest_chunk(size):
        chunk = min(size, BootROM.MAX_TRANSFER)

        p = 4
        while (p << 1) <= chunk:
            p <<= 1

        return p

    def _send_u32(self, value):
        self.ser.write(struct.pack("<I", value))

    def ping(self):
        self.ser.reset_input_buffer()

        self.ser.write(b"\x60")

        deadline = time.monotonic() + 0.02

        while time.monotonic() < deadline:
            b = self.ser.read(1)
            if b == b"\x60":
                return True

        return False

    def wait_for_bootrom(self, timeout=10.0, interval=0.01):
        deadline = time.monotonic() + timeout

        while time.monotonic() < deadline:
            if self.ping():
                return True

            time.sleep(interval)

        return False

    def read(self, addr, size):
        shift = self._cmd_from_size(size)

        self.ser.write(bytes([0x30 | shift]))
        self._send_u32(addr)

        data = self.ser.read(size)

        if len(data) != size:
            raise IOError(
                f"short read ({len(data)} of {size} bytes)"
            )

        return data

    def write(self, addr, data):
        shift = self._cmd_from_size(len(data))

        self.ser.write(bytes([0x40 | shift]))
        self._send_u32(addr)

        written = self.ser.write(data)

        if written != len(data):
            raise IOError(
                f"short write ({written} of {len(data)} bytes)"
            )

    def call(self, addr):
        self.ser.write(b"\x50")
        self._send_u32(addr)

    def dump(self, addr, size):
        out = bytearray()

        while size:
            chunk = self._largest_chunk(size)

            out.extend(self.read(addr, chunk))

            addr += chunk
            size -= chunk

        return bytes(out)

    def upload(self, addr, data):
        offset = 0
        remaining = len(data)

        while remaining:
            chunk = self._largest_chunk(remaining)

            self.write(
                addr + offset,
                data[offset:offset + chunk]
            )

            offset += chunk
            remaining -= chunk


if __name__ == "__main__":

    with BootROM("/dev/ttyUSB0", 115200) as rom:

        print("Waiting for BootROM...")

        if not rom.wait_for_bootrom(timeout=30):
            raise RuntimeError("BootROM not detected")

        print("BootROM detected")

        #
        # Read 256 bytes
        #

        data = rom.read(0xBFE00000, 256)

        with open("dump.bin", "wb") as f:
            f.write(data)

        #
        # Dump 256 KiB
        #

        image = rom.dump(0xBFE00000, 256 * 1024)

        with open("image.bin", "wb") as f:
            f.write(image)

        #
        # Upload small blob
        #

        rom.write(
            0xBFE01000,
            b"\xAA\xBB\xCC\xDD"
        )

        #
        # Execute it
        #

        rom.call(0xBFE01000)
