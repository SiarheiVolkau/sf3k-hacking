#!/usr/bin/env python3

import sys

MASK32 = 0xffffffff

SEED_WORDS = (
    0x9D2EF3A7,
    0x5AC6814B,
    0x1492EF38,
    0x43B0D76C,
)

COUNTERS = (
    0x64B92A7D,
    0x953CF81E,
    0x41DE076A,
    0x5EC3258B,
)


def build_key_table(seed=SEED_WORDS, counters=COUNTERS):
    key = [0] * 132

    local = [0] * 64

    seed0 = seed[0] & MASK32
    seed1 = seed[1] & MASK32

    c = seed[2] & MASK32
    d = seed[3] & MASK32

    local[0] = seed0
    local[1] = seed1

    ctr_lo = 0
    ctr_hi = 0

    p = 0
    o = 0

    while ctr_lo != 31:

        a = local[p]
        b = local[p + 1]

        # key[0], key[1] always the same
        key[o + 0] = seed0
        key[o + 1] = seed1

        x0 = ((b << 24) | (a >> 8)) & MASK32
        x1 = ((a << 24) | (b >> 8)) & MASK32

        s = x0 + c
        carry = s >> 32
        s &= MASK32

        new_c = s ^ ctr_lo

        new_d = (x1 + d + carry) & MASK32
        new_d ^= ctr_hi

        rot_c = ((c << 3) | (d >> 29)) & MASK32
        rot_d = ((d << 3) | (c >> 29)) & MASK32

        c = rot_c ^ new_c
        d = rot_d ^ new_d

        local[p + 2] = new_c
        local[p + 3] = new_d

        key[o + 2] = c
        key[o + 3] = d

        ctr_lo = (ctr_lo + 1) & MASK32
        if ctr_lo == 0:
            ctr_hi = (ctr_hi + 1) & MASK32

        p += 2
        o += 4

    key[124] = seed0
    key[125] = seed1
    key[126] = c
    key[127] = d

    key[128:132] = counters

    return key


def scramble(key, data):
    """
    key  - array of 132 uint32
    data - bytearray to scramble/descramble
    """

    end = len(data)

    t5 = key[128]
    t7 = key[129]
    t2 = key[130]
    t4 = key[131]

    t3 = 0

    stream = bytearray(16)

    pos = 0

    while pos != end:

        if t3 == 0:

            te = 0

            v1 = t2
            t1 = t4
            v0 = t5
            a3 = t7

            while te != 128:

                r0 = ((a3 << 24) | (v0 >> 8)) & MASK32
                r1 = ((v0 << 24) | (a3 >> 8)) & MASK32

                s = r0 + v1
                carry = s >> 32
                s &= MASK32

                r1 = (r1 + t1 + carry) & MASK32

                v0 = key[te + 0] ^ s
                a3 = key[te + 1] ^ r1

                rot1 = ((v1 << 3) | (t1 >> 29)) & MASK32
                rot2 = ((t1 << 3) | (v1 >> 29)) & MASK32

                v1 = rot1 ^ v0
                t1 = rot2 ^ a3

                te += 4

            stream[0:4] = v0.to_bytes(4, "little")
            stream[4:8] = a3.to_bytes(4, "little")
            stream[8:12] = v1.to_bytes(4, "little")
            stream[12:16] = t1.to_bytes(4, "little")

            t2 = (t2 + 1) & MASK32

            if t2 == 0:
                t4 = (t4 + 1) & MASK32

                if t4 == 0:
                    t5 = (t5 + 1) & MASK32

                    if t5 == 0:
                        t7 = (t7 + 1) & MASK32

        #
        # XOR
        #
        data[pos] ^= stream[t3]

        pos += 1
        t3 = (t3 + 1) & 15

    key[128] = t5
    key[129] = t7
    key[130] = t2
    key[131] = t4

def crypt(data: bytes) -> bytes:
    key = build_key_table()

    buf = bytearray(data)
    scramble(key, buf)

    return bytes(buf)

def crypt_file(src: str, dst: str):
    key = build_key_table()

    with open(src, "rb") as f:
        data = bytearray(f.read())

    scramble(key, data)

    with open(dst, "wb") as f:
        f.write(data)

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_file> <output_file>")
        sys.exit(1)

    crypt_file(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
    main()
