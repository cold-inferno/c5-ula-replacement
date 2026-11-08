#!/bin/bash
# Builds firmware and bootloader for both supported chips and copies the
# results to release/. Needs avr-gcc (Homebrew: osx-cross/avr/avr-gcc@14).
#
#   tools/build-all.sh
set -euo pipefail
cd "$(dirname "$0")/.."
[ -d /opt/homebrew/opt/avr-gcc@14/bin ] && export PATH=/opt/homebrew/opt/avr-gcc@14/bin:$PATH

FW_VERSION=$(sed -n 's/^#define FW_VERSION *\([0-9]*\)U.*/\1/p' global.h)
BL_VERSION=$(sed -n 's/^#define BL_VERSION *\([0-9]*\)U.*/\1/p' bootloader/bootload.c)
mkdir -p release

for MCU in atmega88 atmega328p; do
  case $MCU in atmega88) TAG=m88; BOOT=0x1C00; BOOTMAX=1024 ;; atmega328p) TAG=m328p; BOOT=0x7800; BOOTMAX=2048 ;; esac
  echo "=== $MCU ==="

  make clean >/dev/null
  make MCU=$MCU all >build-$TAG.log 2>&1 || { tail -30 build-$TAG.log; exit 1; }
  if grep -iE 'warning:|error:' build-$TAG.log; then echo "build produced warnings"; exit 1; fi
  python3 tools/fwcrc.py check ula_repl_crc.hex
  # black box RAM (.noinit) must not be touched by the bootloader's RAM use
  NOINIT=$(avr-objdump -h ula_repl.elf | awk '$2==".noinit"{print $4, $3}')
  cp ula_repl_crc.hex "release/ULA_firmware_V${FW_VERSION}_LiFePO4_${TAG}.hex"
  cp ula_repl.eep release/ULA_eeprom_defaults.eep
  make clean >/dev/null

  make -C bootloader clean >/dev/null
  make -C bootloader MCU=$MCU all >build-boot-$TAG.log 2>&1 || { tail -30 build-boot-$TAG.log; exit 1; }
  if grep -iE 'warning:|error:' build-boot-$TAG.log; then echo "bootloader build produced warnings"; exit 1; fi
  SIZE=$(avr-size -A bootloader/bootload.elf | awk '$1==".text"{t=$2} $1==".data"{d=$2} END{print t+d}')
  BSS_END=$(avr-nm bootloader/bootload.elf | awk '$3=="__bss_end"{print $1}')
  case $MCU in atmega88) RAMEND=0x4FF ;; atmega328p) RAMEND=0x8FF ;; esac
  python3 - "$NOINIT" "$BSS_END" "$RAMEND" <<'PY'
import sys
vma, size = sys.argv[1].split(); start = int(vma, 16) & 0xFFFF; end = start + int(size, 16)
bl_end = int(sys.argv[2], 16) & 0xFFFF; ramend = int(sys.argv[3], 16)
stack_reserve = 96          # bootloader stack depth is far below this
ok = start >= bl_end and end <= ramend + 1 - stack_reserve
print(f"black box RAM 0x{start:03X}-0x{end-1:03X}, bootloader RAM up to 0x{bl_end-1:03X} "
      f"+ stack below 0x{ramend:03X}: {'OK' if ok else 'OVERLAP'}")
sys.exit(0 if ok else 1)
PY
  echo "bootloader: $SIZE bytes at $BOOT (max $BOOTMAX)"
  [ "$SIZE" -le "$BOOTMAX" ] || { echo "bootloader too large"; exit 1; }
  # keep only the flash part (no EEPROM, no app version/CRC placeholders)
  avr-objcopy -O ihex -j .text -j .data bootloader/bootload.elf "release/bootloader_V${BL_VERSION}_${TAG}.hex"
  make -C bootloader clean >/dev/null
  rm -f build-$TAG.log build-boot-$TAG.log
done
ls -l release
