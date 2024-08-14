#!/bin/bash

# Function to build the UEFI image
build_uefi_img() {
    local platform=$1
    local base_addr=$2

    cat BootShim/BootShim.bin workspace/Build/Htc$platform/DEBUG_GCC/FV/QSD8250_UEFI.fd >> ImageResources/$platform/bootpayload.bin
    dd if=/dev/zero bs=1 count=1 of=ImageResources/$platform/ramdisk
    mkbootimg --kernel ImageResources/$platform/bootpayload.bin --base $base_addr --kernel_offset 0x00008000 -o ImageResources/$platform/uefi.img
    rm -rf ImageResources/$platform/ramdisk
}

# Handle the Leo platform separately
if [ "$1" == 'Leo' ]; then
    build_uefi_img "Leo" "0x11800000"

    # NBH creation
    if [ ! -f ImageResources/Leo/nbgen ]; then
        gcc -std=c99 ImageResources/Leo/nbgen.c -o ImageResources/Leo/nbgen
    fi

    if [ ! -f ImageResources/Leo/yang ]; then
        gcc ImageResources/Leo/yang/nbh.c ImageResources/Leo/yang/nbhextract.c ImageResources/Leo/yang/yang.c -o ImageResources/Leo/yangbin
    fi

    cd ImageResources/Leo/
    ./nbgen os.nb
    ./yang -F LEOIMG.nbh -f logo.nb,os.nb -t 0x600,0x400 -s 64 -d PB8110000 -c 11111111 -v EDK2 -l WWE
    cd ../../
    
# Handle the other platforms (Schubert, Vision, Gold)
elif [[ "$1" == 'Schubert' || "$1" == 'Gold' ]]; then
    build_uefi_img "$1" "0x20000000"
elif [[ "$1" == 'Vision' ]]; then
    dd if=/dev/zero bs=1 count=1 of=ImageResources/$1/ramdisk
    mkbootimg --kernel workspace/Build/HtcVision/DEBUG_GCC/FV/QSD8250_UEFI.fd --ramdisk ImageResources/Vision/recovery-clockwork-6.0.5.0-vision.img-ramdisk.gz --kernel_offset 0x00008000 --ramdisk_offset 0x01000000 --second_offset 0x00f00000 --pagesize 2048 --cmdline no_console_suspend=1 --base 0x04000000 -o ImageResources/$1/uefi.img
    rm -rf ImageResources/$1/ramdisk
else
    echo "Bootimages: Invalid platform"
fi