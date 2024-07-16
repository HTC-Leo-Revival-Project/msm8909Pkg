#!/bin/bash
cd BootShim
make UEFI_BASE=0x2C000000 UEFI_SIZE=0x00100000
rm BootShim.elf
cd ..
