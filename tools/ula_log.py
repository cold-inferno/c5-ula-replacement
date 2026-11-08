#!/usr/bin/env python3
"""Decode the ride log of firmware V1202+ (layout: ridelog.h, RL_LAYOUT 1).

  ula_log.py usbasp [m328p|m88]     read the EEPROM with the USBasp and decode
  ula_log.py file   <eeprom.hex>    decode an EEPROM dump (Intel HEX)
  ula_log.py serial [port]          read via the bootloader (switch the C5 off
                                    and on when asked), then start the firmware
  add --csv <file> to also write the black box samples as CSV
"""
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fwcrc  # noqa: E402

HDR, EVT, EVT_SIZE, BB_SAMPLES, BB_SAMPLE = 16, 32, 8, 60, 5
BB_SIZE = 6 + BB_SAMPLES * BB_SAMPLE

EVENTS = {
    0x01: 'start: power-on', 0x02: 'start: BROWN-OUT (supply voltage dip)',
    0x03: 'start: WATCHDOG (firmware hung)', 0x04: 'start: external reset',
    0x05: 'start: cause unknown (bootloader < V1044)',
    0x10: 'low battery warning', 0x11: 'VOLTAGE ERROR (motor cut)',
    0x12: 'current warning (> limit, motor cut)', 0x13: 'CURRENT ERROR (overload)',
    0x14: 'temperature warning', 0x15: 'TEMPERATURE ERROR (motor cut)',
    0x16: 'THERMISTOR READING ~0 (disconnected?)', 0x20: 'black box saved',
}
FLAGS = [(0x01, 'relay'), (0x02, 'button'), (0x04, 'pedal'), (0x08, 'VOLT'),
         (0x10, 'CURR'), (0x20, 'TEMP'), (0x40, 'THERM')]


def volt(v8):
    return None if v8 == 0 else (v8 + 400 + 32) / 48


def amp(c8):
    return c8 * 2 * 140 / 523


def decode(ee, csv_path=None):
    size = len(ee)
    s = ee[:12]
    u16 = lambda b, o: b[o] | (b[o + 1] << 8)
    print(f'EEPROM {size} bytes. Settings: ' + ' '.join(f'{x:02x}' for x in s))
    print(f'  ride log setting: {"ON" if s[11] == 1 else "off"}')
    h = ee[HDR:HDR + 16]
    if h[0:2] != b'LG':
        print('No ride log present (log never switched on, or cleared).')
        return
    if h[2] != 1:
        print(f'Unknown log layout {h[2]}')
        return
    ride, starts, total = h[4], [u16(h, 6 + 2 * i) for i in range(4)], u16(h, 14)
    bb_addr = size - BB_SIZE
    n = (bb_addr - EVT) // EVT_SIZE
    print(f'Current ride number: {ride}   events written in total: {total} (room for {n})')
    print(f'Starts: power-on {starts[0]}, BROWN-OUT {starts[1]}, WATCHDOG {starts[2]}, external reset {starts[3]}')

    print('\nEvents (oldest first):')
    cnt = min(total, n)
    first = (total - cnt) % n if total else 0
    if not cnt:
        print('  none')
    for k in range(cnt):
        a = EVT + ((first + k) % n) * EVT_SIZE
        r = ee[a:a + EVT_SIZE]
        t, code, rid, spd, v8, c8, temp = u16(r, 0), r[2], r[3], r[4], r[5], r[6], r[7]
        line = f'  ride {rid:3d}  {t // 60:3d}:{t % 60:02d}  {EVENTS.get(code, f"code 0x{code:02x}")}'
        if code >= 0x10 and code != 0x20:
            vv = volt(v8)
            line += (f'   [{spd / 2:.1f} km/h, {vv:.2f} V, {amp(c8):.0f} A, {temp} °C]' if vv
                     else f'   [{spd / 2:.1f} km/h, battery <8.3 V, {amp(c8):.0f} A, {temp} °C]')
        print(line)

    bb = ee[bb_addr:bb_addr + BB_SIZE]
    print('\nBlack box:', end=' ')
    if bb[0] != 0xA5:
        print('empty (no unexpected restart recorded)')
        return
    cnt, up = bb[3], u16(bb, 4)
    print(f'{cnt} s before restart "{EVENTS.get(bb[1], hex(bb[1]))}" in ride {bb[2]} '
          f'(last sample at {up // 60}:{up % 60:02d} after power-on)')
    rows = []
    for i in range(cnt):
        smp = bb[6 + i * BB_SAMPLE:6 + (i + 1) * BB_SAMPLE]
        t = up - (cnt - 1 - i)
        flags = ','.join(n for b, n in FLAGS if smp[4] & b)
        vv = volt(smp[1])
        rows.append((t, smp[0] / 2, vv, amp(smp[2]), smp[3], flags))
    print('   time   km/h   volt    amp  °C  flags')
    for t, spd, vv, a, temp, flags in rows:
        print(f'  {t // 60:3d}:{t % 60:02d} {spd:5.1f} {(f"{vv:6.2f}" if vv else "   n/a")} {a:6.0f} {temp:3d}  {flags}')
    if csv_path:
        with open(csv_path, 'w') as f:
            f.write('seconds,speed_kmh,battery_v,current_a,temp_c,flags\n')
            for t, spd, vv, a, temp, flags in rows:
                f.write(f'{t},{spd},{vv if vv else ""},{a:.1f},{temp},{flags}\n')
        print(f'black box written to {csv_path}')


def read_usbasp(part):
    with tempfile.TemporaryDirectory() as d:
        out = os.path.join(d, 'ee.hex')
        r = subprocess.run(['avrdude', '-c', 'usbasp', '-p', part, '-n',
                            '-U', f'eeprom:r:{out}:i'], capture_output=True, text=True)
        if r.returncode:
            raise SystemExit(r.stderr[-800:])
        mem = fwcrc.read_hex(out)
    size = 1024 if part == 'm328p' else 512
    return bytes(mem.get(i, 0xFF) for i in range(size))


def read_serial(port):
    import ula_monitor as m
    import time
    fd = m.open_port(port)
    print('Switch the C5 off and on now (waiting up to 30 s)...')
    m.enter_bootloader(fd, 30)

    def rd(off):
        os.write(fd, bytes([ord('r'), off & 0xFF, off >> 8]))
        t_end = time.time() + 1
        b = b''
        while not b and time.time() < t_end:
            b = m.read_some(fd, 0.05)
        if not b:
            raise SystemExit(f'no answer reading offset {off}')
        return b[0]
    head = bytes(rd(i) for i in range(32))
    size = head[HDR + 3] * 256 if head[HDR:HDR + 2] == b'LG' and head[HDR + 3] in (2, 4) else 32
    print(f'reading {size} bytes...')
    ee = head + bytes(rd(i) for i in range(32, size))
    os.write(fd, b's')
    os.close(fd)
    return ee


def main(argv):
    csv = None
    if '--csv' in argv:
        i = argv.index('--csv')
        csv = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    if not argv:
        print(__doc__)
        return 2
    if argv[0] == 'usbasp':
        decode(read_usbasp(argv[1] if len(argv) > 1 else 'm328p'), csv)
    elif argv[0] == 'file' and len(argv) > 1:
        mem = fwcrc.read_hex(argv[1])
        size = 1024 if max(mem) >= 512 else 512
        decode(bytes(mem.get(i, 0xFF) for i in range(size)), csv)
    elif argv[0] == 'serial':
        import ula_monitor as m
        decode(read_serial(argv[1] if len(argv) > 1 else m.find_port()), csv)
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
