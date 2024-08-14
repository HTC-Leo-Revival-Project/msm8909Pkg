#!/bin/bash
# based on the instructions from edk2-platform
set -e
export PACKAGES_PATH=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
export WORKSPACE=$PWD/workspace
. ../edk2/edksetup.sh
# not actually GCC5; it's GCC7 on Ubuntu 18.04.
GCC5_ARM_PREFIX=arm-linux-gnueabihf- build -s -n 0 -a ARM -t GCC5 -p HtcLeoPkg/HtcLeoPkg.dsc

#mkbootimg --kernel workspace/Build/QSD8250/DEBUG_GCC5/FV/QSD8250_UEFI.fd --base 0x04000000 --kernel_offset 0x00008000 -o uefi.img
mkbootimg --kernel workspace/Build/QSD8250/DEBUG_GCC5/FV/QSD8250_UEFI.fd --ramdisk recovery-clockwork-6.0.5.0-vision.img-ramdisk.gz --kernel_offset 0x00008000 --ramdisk_offset 0x01000000 --second_offset 0x00f00000 --pagesize 2048 --cmdline no_console_suspend=1 --base 0x04000000 -o uefi.img
rm -r workspace/Build
