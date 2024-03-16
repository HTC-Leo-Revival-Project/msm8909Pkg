#!/bin/bash
# based on the instructions from edk2-platform
rm -rf ImageResources/*.img ImageResources/Tools/*.bin
set -e
export PACKAGES_PATH=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
export WORKSPACE=$PWD/workspace

while getopts d: flag
do
    case "${flag}" in
        d) device=${OPTARG};;
    esac
done

# based on https://github.com/edk2-porting/edk2-msm/blob/master/build.sh#L47 
function _build(){
if [ "$1" = "Leo" ] || [ "$1" = "Passion" ] || [ "$1" = "Bravo" ]; then
	local DEVICE="${1}"
	shift
    echo "Building uefi for $1"
	source "../edk2/edksetup.sh"
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/Htc${DEVICE}/Htc${DEVICE}Pkg.dsc
	./build_boot_images.sh $1
elif [ $1 == 'All' ]; then
	local DEVICE="${1}"
	shift
    echo "Building uefi for all platforms"
	source "../edk2/edksetup.sh"

	# TODO: Improve

	# Leo
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/HtcLeo/HtcLeoPkg.dsc
	./build_boot_images.sh Leo

	# Passion
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/HtcPassion/HtcPassionPkg.dsc
	./build_boot_images.sh Passion

	# Bravo
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/HtcBravo/HtcBravoPkg.dsc
	./build_boot_images.sh Bravo
else
    echo "Build: Invalid platform"
fi
}

_build "$device"
