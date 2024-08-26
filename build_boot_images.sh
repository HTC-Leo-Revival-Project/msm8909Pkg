#!/bin/bash

echo "Building bootpayload.bin"
case $1 in
    Leo|Schubert|Gold)
        make -C BootShim UEFI_BASE=0x2C000000 UEFI_SIZE=0x00100000 PLATFORM=$1
        ;;
    *)
        echo "[!] Not building bootpayload.bin for target $1"
        ;;
esac

echo "Building Android BootImage"
case $1 in
    Leo)
        mkbootimg --kernel ImageResources/$1/bootpayload.bin --base 0x11800000 --kernel_offset 0x00008000 -o ImageResources/$1/uefi.img
        ;;
    Schubert|Gold)
        mkbootimg --kernel ImageResources/$1/bootpayload.bin --base 0x20000000 --kernel_offset 0x00008000 -o ImageResources/$1/uefi.img
        ;;
    *)
        echo "[!] Not building Android BootImage for target $1"
        ;;
esac

echo "Building NBH"
case $1 in
    Leo)
        # NBH creation
        make -C Tools/nbgen clean all
        make -C Tools/yang clean all

        cd ImageResources/Leo/
        ../../Tools/nbgen/nbgen os.nb
        cd ../../
        Tools/yang/yang -F ImageResources/Leo/LEOIMG.nbh -f ImageResources/Leo/logo.nb,ImageResources/Leo/os.nb -t 0x600,0x400 -s 64 -d PB8110000 -c 11111111 -v EDK2 -l WWE
        ;;
    *)
        echo "[!] Not building NBH for target $1"
esac

exit 0
