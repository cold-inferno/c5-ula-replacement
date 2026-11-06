# Host (PC/Mac) unit tests for hardware-independent firmware logic.
# Usage:  make -C test
CC      ?= cc
CFLAGS  = -std=c99 -g -Wall -Wextra -fsanitize=address,undefined
# force the host type definitions (exact 16/32-bit types like on the AVR)
CFLAGS += -include host/xtypes.h -I..

test_pedelec: test_pedelec.c ../pedelec.c ../pedelec.h host/xtypes.h
	$(CC) $(CFLAGS) test_pedelec.c ../pedelec.c -o $@

run: test_pedelec
	./test_pedelec

.DEFAULT_GOAL := run
.PHONY: run clean
clean:
	rm -f test_pedelec
