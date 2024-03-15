#!/bin/bash
# based on the instructions from edk2-platform
rm -rf ImageResources/*.img ImageResources/Tools/*.bin
set -e
export PACKAGES_PATH=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
export WORKSPACE=$PWD/workspace
. ../edk2/edksetup.sh
GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p HtcLeoPkg/Platforms/HtcLeo/HtcLeoPkg.dsc

chmod +x build_boot_shim.sh
./build_boot_shim.sh

cat BootShim/BootShim.bin workspace/Build/HtcLeo/DEBUG_GCC/FV/QSD8250_UEFI.fd >>ImageResources/Tools/bootpayload.bin

mkbootimg --kernel ImageResources/Tools/bootpayload.bin --base 0x11800000 --kernel_offset 0x00008000 -o ImageResources/uefi_leo.img

# NBH creation
if [ ! -f ImageResources/Tools/nbgen ]; then
	gcc -std=c99 ImageResources/Tools/nbgen.c -o ImageResources/Tools/nbgen
fi
# We're using a prebuild yang binary for now, since the compiled version doesn't seem to produce a valid NBH for Leo
if [ ! -f ImageResources/Tools/yang ]; then
	gcc ImageResources/Tools/yang/nbh.c ImageResources/Tools/yang/nbhextract.c ImageResources/Tools/yang/yang.c -o ImageResources/Tools/yangbin
fi

cd ImageResources/Tools
./nbgen os.nb
./yang -F ../LEOIMG.nbh -f logo.nb,os.nb -t 0x600,0x400 -s 64 -d PB8110000 -c 11111111 -v EDK2 -l WWE
rm *.bin
rm os.nb
cd ../../
