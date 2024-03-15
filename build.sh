#!/bin/bash
# based on the instructions from edk2-platform
rm -rf ImageResources/*.img ImageResources/Tools/*.bin
set -e
export PACKAGES_PATH=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
export WORKSPACE=$PWD/workspace

if [ $1 == 'Leo' ]; then
    echo "Building uefi for Leo"
	./build_uefi_leo.sh
	./build_boot_images.sh $1
elif [ $1 == 'Passion' ]; then
    echo "Building uefi for Passion"
	./build_uefi_passion.sh
	./build_boot_images.sh $1
elif [ $1 == 'All' ]; then
	echo "Building uefi for all platforms"

	./build_uefi_leo.sh
	./build_boot_images.sh Leo

	./build_uefi_passion.sh
	./build_boot_images.sh Passion
else
    echo "Invalid platform"
fi
