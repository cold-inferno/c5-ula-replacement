#!/usr/bin/env python3
"""Compute and embed the ULA replacement firmware CRC32 (replaces crc_calc.exe).

The firmware's self-test (safety_MemoryTest) walks flash bytes 0 .. MEMPOS_CRC-1,
treating unprogrammed bytes as 0xFF, with this bit-serial algorithm:

    crc = 0xFFFFFFFF
    for each byte b, for 8 bits (LSB first):
        if (crc >> 31) != (b & 1): crc = (crc << 1) ^ 0xEDB88320
        else:                       crc = (crc << 1)
        b >>= 1
    crc ^= 0xFFFFFFFF

and compares it with the little-endian UINT32 stored at MEMPOS_CRC. If they
differ, the firmware enters safe mode (motor relay permanently off).

Usage:
    fwcrc.py check  <firmware.hex>           print stored and computed CRC
    fwcrc.py patch  <in.hex> <out.hex>       write out.hex with correct CRC
"""
import sys

MEMPOS_VER = 0x1BFA
MEMPOS_CRC = 0x1BFC
APP_END = 0x1C00  # start of bootloader section
POLY = 0xEDB88320


def read_hex(path):
    mem = {}
    base = 0
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line.startswith(':'):
                continue
            raw = bytes.fromhex(line[1:])
            n, addr, rtype = raw[0], (raw[1] << 8) | raw[2], raw[3]
            data = raw[4:4 + n]
            if (sum(raw) & 0xFF) != 0:
                raise ValueError(f'bad checksum in {path}: {line}')
            if rtype == 0:
                for i, b in enumerate(data):
                    mem[base + addr + i] = b
            elif rtype == 2:
                base = ((data[0] << 8) | data[1]) << 4
            elif rtype == 4:
                base = ((data[0] << 8) | data[1]) << 16
            elif rtype == 1:
                break
    return mem


def write_hex(mem, path):
    addrs = sorted(mem)
    lines = []
    i = 0
    while i < len(addrs):
        start = addrs[i]
        chunk = [mem[start]]
        j = i + 1
        while j < len(addrs) and addrs[j] == start + len(chunk) and len(chunk) < 16:
            chunk.append(mem[addrs[j]])
            j += 1
        rec = bytes([len(chunk), (start >> 8) & 0xFF, start & 0xFF, 0]) + bytes(chunk)
        lines.append(':' + (rec + bytes([(-sum(rec)) & 0xFF])).hex().upper())
        i = j
    lines.append(':00000001FF')
    with open(path, 'w', newline='\r\n') as f:
        f.write('\n'.join(lines) + '\n')


def compute(mem):
    crc = 0xFFFFFFFF
    for a in range(MEMPOS_CRC):
        b = mem.get(a, 0xFF)
        for _ in range(8):
            if ((crc >> 31) & 1) != (b & 1):
                crc = ((crc << 1) ^ POLY) & 0xFFFFFFFF
            else:
                crc = (crc << 1) & 0xFFFFFFFF
            b >>= 1
    return crc ^ 0xFFFFFFFF


def stored(mem):
    return sum(mem.get(MEMPOS_CRC + i, 0xFF) << (8 * i) for i in range(4))


def main(argv):
    if len(argv) >= 2 and argv[0] == 'check':
        mem = read_hex(argv[1])
        top = max(a for a in mem if a < APP_END)
        ver = mem.get(MEMPOS_VER, 0xFF) | (mem.get(MEMPOS_VER + 1, 0xFF) << 8)
        c, s = compute(mem), stored(mem)
        print(f'version {ver}, app bytes used up to 0x{top:04X}, stored CRC 0x{s:08X}, '
              f'computed CRC 0x{c:08X} -> {"OK" if c == s else "MISMATCH"}')
        return 0 if c == s else 1
    if len(argv) >= 3 and argv[0] == 'patch':
        mem = read_hex(argv[1])
        if any(a >= APP_END for a in mem):
            raise SystemExit('input contains bootloader-area data; patch the application hex only')
        c = compute(mem)
        for i in range(4):
            mem[MEMPOS_CRC + i] = (c >> (8 * i)) & 0xFF
        write_hex(mem, argv[2])
        print(f'patched CRC 0x{c:08X} into {argv[2]}')
        return 0
    print(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
