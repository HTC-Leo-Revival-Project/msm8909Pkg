#!/bin/bash
# based on the instructions from edk2-platform
rm -rf ImageResources/*.img ImageResources/Tools/*.bin
set -e
export PACKAGES_PATH=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
export WORKSPACE=$PWD/workspace

AvailablePlatforms=("Leo" "Passion" "Bravo" "All")
IsValid=0

while getopts d: flag
do
    case "${flag}" in
        d) device=${OPTARG};;
    esac
done

function _check_args(){
	local DEVICE="${1}"
	for Name in "${AvailablePlatforms[@]}"
	do
		if [[ $DEVICE == "$Name" ]]; then
			IsValid=1
			break;
		fi
	done
}

# based on https://github.com/edk2-porting/edk2-msm/blob/master/build.sh#L47 
function _build(){
if [[ " ${AvailablePlatforms[*]} " == *"$1"* ]]; then
	local DEVICE="${1}"
	shift
    echo "Building uefi for $DEVICE"
	source "../edk2/edksetup.sh"
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/Htc${DEVICE}/Htc${DEVICE}Pkg.dsc

	./build_boot_shim.sh
	./build_boot_images.sh $DEVICE
elif [ $1 == 'All' ]; then
	local DEVICE="${1}"
	shift
    echo "Building uefi for all platforms"
	source "../edk2/edksetup.sh"

	# TODO: Improve
	for PlatformName in "${AvailablePlatforms[@]}"
	do
		# Build
		GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/Htc${PlatformName}/Htc${PlatformName}Pkg.dsc
		./build_boot_shim.sh
		./build_boot_images.sh $PlatformName
	done
fi
}

_check_args "$device"
if [ $IsValid == 1 ]; then
	_build "$device"
else
	echo "Build: Invalid platform"
	echo "Available targets: "
	for Name in "${AvailablePlatforms[@]}"
	do
		echo " - "$Name
	done
fi
