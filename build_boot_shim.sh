#!/bin/bash
make -C BootShim clean
make -C BootShim UEFI_BASE=0x2C000000 UEFI_SIZE=0x00100000
