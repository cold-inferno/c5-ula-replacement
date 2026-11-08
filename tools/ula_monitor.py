#!/usr/bin/env python3
"""Talk to the C5 ULA replacement over a USB-serial adapter (macOS/Linux, no
extra Python packages needed).

  ula_monitor.py listen   [port] [seconds]   read-only: print and decode output
  ula_monitor.py settings [port]             wait for power-on, enter the
                                             bootloader and READ the settings
                                             (read commands only, then starts
                                             the firmware again)

port defaults to the first /dev/cu.usbserial* / /dev/ttyUSB* device.
Serial: 9600 baud, 8N1.
"""
import glob
import os
import select
import sys
import termios
import time

BAUD = termios.B9600


def find_port():
    ports = sorted(glob.glob('/dev/cu.usbserial*') + glob.glob('/dev/cu.SLAB*') +
                   glob.glob('/dev/ttyUSB*'))
    if not ports:
        raise SystemExit('No USB-serial adapter found (is it plugged in?)')
    return ports[0]


def open_port(path):
    fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    a = termios.tcgetattr(fd)
    a[0] = 0                                    # iflag: raw
    a[1] = 0                                    # oflag: raw
    a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    a[3] = 0                                    # lflag: raw
    a[4] = a[5] = BAUD
    a[6][termios.VMIN] = 0
    a[6][termios.VTIME] = 0
    termios.tcsetattr(fd, termios.TCSANOW, a)
    termios.tcflush(fd, termios.TCIOFLUSH)
    return fd


def read_some(fd, timeout):
    r, _, _ = select.select([fd], [], [], timeout)
    if not r:
        return b''
    try:
        return os.read(fd, 256)
    except BlockingIOError:
        return b''


# ---- conversions, identical to the firmware / configurator ----
def adc_to_volt(a):
    return (a + 32) / 48


def adc_to_amp(a):
    return a * 140 / 523


def adc_to_celsius(adc):
    if not adc:
        return 85
    ntc = (1024 * 1100) // adc - 1100
    for lim, deg in [(12500, 0), (9000, 5), (7500, 10), (6000, 15), (4700, 20), (3800, 25),
                     (3300, 30), (2500, 35), (2000, 40), (1700, 45), (1200, 50), (1000, 55),
                     (800, 60), (900, 65), (800, 70), (600, 75), (500, 80)]:
        if ntc >= lim:
            return deg
    return 85


COND = {1: 'voltage warning', 2: 'current warning', 4: 'temperature warning',
        8: 'VOLTAGE ERROR', 16: 'CURRENT ERROR', 32: 'TEMPERATURE ERROR'}


def decode(line):
    f = [x for x in line[1:].split('|') if x != '']
    try:
        f = [int(x) for x in f]
    except ValueError:
        return None
    if len(f) < 12:
        return None
    dist, speed, rpm, tmo, sw, temp, volt, vref, curr, ena, cond, t = f[:12]
    rest = f[12] if len(f) > 12 else None
    flags = ', '.join(n for b, n in COND.items() if cond & b) or 'OK'
    s = (f'{t/1000:7.1f}s  speed {speed/10:4.1f} km/h  cadence {rpm:3d} rpm '
         f'({"pedalling" if tmo < 2000 else "not pedalling"})  '
         f'batt {adc_to_volt(volt):5.2f} V (ADC {volt})')
    if rest is not None:
        s += f'  rest {adc_to_volt(rest):5.2f} V'
    s += (f'  motor {adc_to_amp(curr):3.0f} A  relay {"ON " if ena else "off"}  '
          f'button {"pressed" if sw > 300 else "released"} (ADC {sw})  '
          f'temp {adc_to_celsius(temp)} °C  vref ADC {vref}  status {flags}')
    return s


def enter_bootloader(fd, wait=30):
    """Send 'e' until the bootloader answers 'B', then confirm with a version
    query: the firmware's own start-up text contains a 'B' too ("BAT: ...").
    Returns the bootloader version."""
    end = time.time() + wait
    while time.time() < end:
        got = b''
        while time.time() < end and b'B' not in got:
            os.write(fd, b'e')
            got += read_some(fd, 0.04)
        if b'B' not in got:
            break
        time.sleep(0.15)
        termios.tcflush(fd, termios.TCIFLUSH)
        os.write(fd, b'v')
        ans = b''
        t_end = time.time() + 0.5
        while len(ans) < 2 and time.time() < t_end:
            ans += read_some(fd, 0.05)
        if len(ans) == 2 and 1000 <= ((ans[0] << 8) | ans[1]) <= 9999:
            termios.tcflush(fd, termios.TCIFLUSH)
            return (ans[0] << 8) | ans[1]
        termios.tcflush(fd, termios.TCIFLUSH)   # false alarm: keep trying
    raise SystemExit('Bootloader not found. Adapter TX must go to board RXD.')


def listen(port, seconds):
    fd = open_port(port)
    print(f'Listening on {port} at 9600 baud for {seconds} s (sending nothing)...')
    buf = b''
    total = 0
    end = time.time() + seconds
    while time.time() < end:
        chunk = read_some(fd, 0.2)
        total += len(chunk)
        buf += chunk
        while b'\n' in buf:
            raw, buf = buf.split(b'\n', 1)
            line = raw.decode('latin-1').strip('\r')
            if not line:
                continue
            if line.startswith('#'):
                d = decode(line)
                print(('  ' + d) if d else ('  [unparsed] ' + line))
            else:
                print('  C5> ' + line)
    if buf.strip():
        print('  [partial] ' + repr(buf[:120]))
    os.close(fd)
    print(f'Received {total} bytes.')
    if total == 0:
        print('Nothing received: is the C5 switched on, GND connected, and the adapter RX '
              'wired to the board TXD?')


def settings(port, wait=30):
    fd = open_port(port)
    print(f'Switch the C5 off and on now (waiting up to {wait} s)...')
    enter_bootloader(fd, wait)

    def ask(cmd, n, timeout=1.0):
        os.write(fd, cmd)
        out = b''
        t_end = time.time() + timeout
        while len(out) < n and time.time() < t_end:
            out += read_some(fd, 0.05)
        if len(out) < n:
            raise SystemExit(f'No answer to {cmd!r}')
        return out[:n]

    bl = ask(b'v', 2)
    fw = ask(b'g', 2)
    blv, fwv = (bl[0] << 8) | bl[1], (fw[0] << 8) | fw[1]
    print(f'Bootloader V{blv}, firmware V{fwv}')
    b = [ask(bytes([ord('r'), off, 0]), 1)[0] for off in range(12)]
    u16 = lambda o: b[o] | (b[o + 1] << 8)
    print('Raw settings:', ' '.join(f'{x:02x}' for x in b))
    mx = u16(3)
    print(f'  pedal assist     : {"on" if b[0] else "off"}')
    print(f'  wheel            : {u16(1)} mm')
    print(f'  min speed        : {u16(5)/10:.1f} km/h')
    print(f'  max speed        : {"no limit" if mx == 0 else f"{mx/10:.1f} km/h"}')
    print(f'  fan on at        : {u16(7)} °C')
    print(f'  magnets wheel/pedal: {b[9]} / {b[10]}')
    print(f'  ride log         : {"on" if b[11] == 1 else "off"}')
    s = ask(b's', 1)
    print('Firmware started' if s == b'S' else f'Start answer: {s!r}')
    os.close(fd)


def upload(port, hexfile, wait=30):
    """Upload an application firmware HEX through the bootloader (protocol of
    bootloader V1042/V1043: 'f' + 112 x (64 bytes + 8-bit sum), 'O' per block,
    'D' at the end). Leaves the device in the bootloader (no start)."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import fwcrc
    mem = fwcrc.read_hex(hexfile)
    if any(a >= fwcrc.APP_END for a in mem):
        raise SystemExit('HEX contains bootloader-area data: not an application file')
    if fwcrc.compute(mem) != fwcrc.stored(mem):
        raise SystemExit('HEX checksum is wrong: the firmware would refuse to run')
    image = bytes(mem.get(a, 0xFF) for a in range(fwcrc.APP_END))
    ver = image[fwcrc.MEMPOS_VER] | (image[fwcrc.MEMPOS_VER + 1] << 8)

    fd = open_port(port)
    print(f'Waiting for the bootloader (switch the C5 off and on if needed, up to {wait} s)...')
    blv = enter_bootloader(fd, wait)

    def rx(n, timeout=2.0):
        out = b''
        t_end = time.time() + timeout
        while len(out) < n and time.time() < t_end:
            out += read_some(fd, 0.05)
        return out

    print(f'Bootloader V{blv}')
    print(f'Uploading firmware V{ver} ({hexfile})')
    os.write(fd, b'f')
    time.sleep(0.05)
    t0 = time.time()
    for blk in range(112):
        data = image[blk * 64:(blk + 1) * 64]
        os.write(fd, data + bytes([sum(data) & 0xFF]))
        r = rx(1)
        if r != b'O':
            os.close(fd)
            raise SystemExit(f'block {blk} not confirmed (answer {r!r})')
        if blk % 16 == 15:
            print(f'  {blk + 1}/112 blocks')
    r = rx(1)
    if r != b'D':
        os.close(fd)
        raise SystemExit(f'no final confirmation (answer {r!r})')
    os.write(fd, b'g')
    fw = rx(2)
    print(f'Upload done in {time.time() - t0:.1f} s; bootloader now reports firmware '
          f'V{(fw[0] << 8) | fw[1]}' if len(fw) == 2 else f'version answer: {fw!r}')
    os.close(fd)


def main(argv):
    if argv and argv[0] == 'upload' and len(argv) >= 2:
        port = argv[2] if len(argv) > 2 else find_port()
        upload(port, argv[1])
        return 0
    if not argv or argv[0] not in ('listen', 'settings'):
        print(__doc__)
        return 2
    port = argv[1] if len(argv) > 1 and argv[1].startswith('/dev/') else find_port()
    if argv[0] == 'listen':
        secs = float(argv[-1]) if len(argv) > 1 and not argv[-1].startswith('/dev/') else 10
        listen(port, secs)
    else:
        settings(port)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
