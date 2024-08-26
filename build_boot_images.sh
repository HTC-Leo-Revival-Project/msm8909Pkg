#!/bin/bash

case $1 in
    Leo)
        cat BootShim/BootShim.bin $WORKSPACE/Build/HtcLeo/DEBUG_GCC/FV/QSD8250_UEFI.fd > ImageResources/Leo/bootpayload.bin

        mkbootimg --kernel ImageResources/Leo/bootpayload.bin --base 0x11800000 --kernel_offset 0x00008000 -o ImageResources/Leo/uefi.img

        # NBH creation
        make -C Tools/nbgen clean all
        make -C Tools/yang clean all

        cd ImageResources/Leo/
        ../../Tools/nbgen/nbgen os.nb
        cd ../../
        Tools/yang/yang -F ImageResources/Leo/LEOIMG.nbh -f ImageResources/Leo/logo.nb,ImageResources/Leo/os.nb -t 0x600,0x400 -s 64 -d PB8110000 -c 11111111 -v EDK2 -l WWE
        ;;
    Schubert)
        cat BootShim/BootShim.bin $WORKSPACE/Build/HtcSchubert/DEBUG_GCC/FV/QSD8250_UEFI.fd > ImageResources/$1/bootpayload.bin

        mkbootimg --kernel ImageResources/$1/bootpayload.bin --base 0x20000000 --kernel_offset 0x00008000 -o ImageResources/$1/uefi.img
        ;;
    Gold)
        cat BootShim/BootShim.bin $WORKSPACE/Build/HtcGold/DEBUG_GCC/FV/QSD8250_UEFI.fd > ImageResources/$1/bootpayload.bin

        mkbootimg --kernel ImageResources/$1/bootpayload.bin --base 0x20000000 --kernel_offset 0x00008000 -o ImageResources/$1/uefi.img
        ;;
    *)
        echo "Bootimages: Invalid platform ($1)"
        exit 1
        ;;
esac
exit 0
