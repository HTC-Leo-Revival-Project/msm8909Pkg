#
#  Copyright (c) 2018, Linaro Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = HtcLeo
  PLATFORM_GUID                  = 28f1a3bf-193a-47e3-a7b9-5a435eaab2ee
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platforms/$(PLATFORM_NAME)/$(PLATFORM_NAME)Pkg.fdf

  DEFINE USE_SCREEN_FOR_SERIAL_OUTPUT = 0

!include HtcLeoPkg/CommonDsc.dsc.inc

[PcdsFixedAtBuild.common]
  # System Memory (576MB)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x11800000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x1E800000
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x01000000

  gHtcLeoPkgTokenSpaceGuid.PcdMipiFrameBufferAddress|0x02A00000
  gHtcLeoPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|480
  gHtcLeoPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|800