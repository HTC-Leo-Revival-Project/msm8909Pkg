#!/bin/bash
if [ $1 = "Passion" ] || [ $1 = "Bravo" ] || [ $1 = "Leo" ]; then
    cd BootShim
    make UEFI_BASE=0x2C000000 UEFI_SIZE=0x00100000
    rm BootShim.elf
    cd ..
elif [ $1 = "Schubert" ]; then
    cd BootShim
    make UEFI_BASE=0x2C000000 UEFI_SIZE=0x00100000
    rm BootShim.elf
    cd ..
else
    echo "BootShim: Invalid platform"
fi
