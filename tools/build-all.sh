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
  cp ula_repl_crc.hex "release/ULA_firmware_V${FW_VERSION}_LiFePO4_${TAG}.hex"
  cp ula_repl.eep release/ULA_eeprom_defaults.eep
  make clean >/dev/null

  make -C bootloader clean >/dev/null
  make -C bootloader MCU=$MCU all >build-boot-$TAG.log 2>&1 || { tail -30 build-boot-$TAG.log; exit 1; }
  if grep -iE 'warning:|error:' build-boot-$TAG.log; then echo "bootloader build produced warnings"; exit 1; fi
  SIZE=$(avr-size -A bootloader/bootload.elf | awk '$1==".text"{t=$2} $1==".data"{d=$2} END{print t+d}')
  echo "bootloader: $SIZE bytes at $BOOT (max $BOOTMAX)"
  [ "$SIZE" -le "$BOOTMAX" ] || { echo "bootloader too large"; exit 1; }
  # keep only the flash part (no EEPROM, no app version/CRC placeholders)
  avr-objcopy -O ihex -j .text -j .data bootloader/bootload.elf "release/bootloader_V${BL_VERSION}_${TAG}.hex"
  make -C bootloader clean >/dev/null
  rm -f build-$TAG.log build-boot-$TAG.log
done
ls -l release
