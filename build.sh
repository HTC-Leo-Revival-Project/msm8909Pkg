#!/bin/bash
# based on the instructions from edk2-platform
set -e

# only set vars when unset (allow overriding them)
PACKAGES_PATH_DEFAULT=$PWD/../edk2:$PWD/../edk2-platforms:$PWD
WORKSPACE_DEFAULT=$PWD/workspace
export PACKAGES_PATH=${PACKAGES_PATH:-$PACKAGES_PATH_DEFAULT}
export WORKSPACE=${WORKSPACE:-$WORKSPACE_DEFAULT}

AvailablePlatforms=("Leo" "Schubert" "Gold")
DevicesToBuild=()
IsValid=0

while getopts d: flag
do
    case "${flag}" in
        d) device=$(echo ${OPTARG} | tr '[:upper:]' '[:lower:]');;
    esac
done

function _check_args(){
	local DEVICE="${1}"
	if [[ $DEVICE == 'all' ]]; then
		IsValid=1
		DevicesToBuild=(${AvailablePlatforms[@]})
	else
		for Name in "${AvailablePlatforms[@]}"
		do
			if [[ $DEVICE == $(echo "$Name" | tr '[:upper:]' '[:lower:]') ]]; then
				IsValid=1
				DevicesToBuild=("$Name")
				break;
			fi
		done
	fi
}

function _clean() {
	for PlatformName in "${AvailablePlatforms[@]}"
		do
			if [ $PlatformName != 'All' ]; then
				if [ -f ImageResources/$DEVICE/bootpayload.bin ]; then
					rm ImageResources/$DEVICE/bootpayload.bin
				fi
				if [ -f ImageResources/$DEVICE/os.nb ]; then
					rm ImageResources/$DEVICE/os.nb
				fi
				if [ -f ImageResources/$DEVICE/*.img ]; then
					rm ImageResources/$DEVICE/*.img
				fi
			fi
		if [ -f BootShim/BootShim.bin ]; then
			rm BootShim/BootShim.bin
		fi
		if [ -d workspace/Build ]; then
			rm -r workspace/Build
		fi
	done
}

# based on https://github.com/edk2-porting/edk2-msm/blob/master/build.sh#L47 
function _build(){
	local DEVICE="${1}"
	shift
	source "../edk2/edksetup.sh"
	
	# Clean artifacts if needed
	_clean
	echo "Artifacts removed"

    echo "Building uefi for $DEVICE"
	GCC_ARM_PREFIX=arm-none-eabi- build -s -n 0 -a ARM -t GCC -p Platforms/Htc${DEVICE}/Htc${DEVICE}Pkg.dsc
	mkdir -p "ImageResources/$DEVICE"
	./build_boot_shim.sh
	./build_boot_images.sh $DEVICE
}

_check_args "$device"
if [ $IsValid == 1 ]; then
	for Name in "${DevicesToBuild[@]}"
	do
		_build "$Name"
	done
else
	echo "Build: Invalid platform: $device"
	echo "Available targets: "
	for Name in "${AvailablePlatforms[@]}"
	do
		echo " - "$Name
	done
fi
