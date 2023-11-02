/** @file

  Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __TLMM_GPIO_H__
#define __TLMM_GPIO_H__

#define TLMM_GPIO_GUID                                             \
  {                                                                            \
    0x2c898318, 0x41c1, 0x4309,                                                \
    {                                                                          \
      0x89, 0x8a, 0x2f, 0x55, 0xc8, 0xcf, 0x0b, 0x87                           \
    }                                                                          \
  }

//
// Protocol interface structure
//
typedef struct _TLMM_GPIO TLMM_GPIO;

//
// Data Types
//
typedef UINTN TLMM_GPIO_PIN;

//
// Function Prototypes
//
typedef
UINTN
(*TLMM_GPIO_GET)(
  //IN  TLMM_GPIO       *This,
  IN  TLMM_GPIO_PIN   Gpio//,
  //OUT UINTN           *Value
  );

/*++

Routine Description:

  Gets the state of a GPIO pin

Arguments:

  This  - pointer to protocol
  Gpio  - which pin to read
  Value - state of the pin

Returns:

  EFI_SUCCESS - GPIO state returned in Value

--*/

typedef
VOID
(*TLMM_GPIO_SET)(
  //IN TLMM_GPIO      *This,
  IN TLMM_GPIO_PIN  Gpio,
  IN UINTN          Mode
  );

/*++

Routine Description:

  Sets the state of a GPIO pin

Arguments:

  This  - pointer to protocol
  Gpio  - which pin to modify
  Mode  - mode to set

Returns:

  EFI_SUCCESS - GPIO set as requested

--*/

struct _TLMM_GPIO {
  TLMM_GPIO_GET         Get;
  TLMM_GPIO_SET         Set;
};

extern EFI_GUID  gTlmmGpioProtocolGuid;

/*typedef struct _GPIO_CONTROLLER          GPIO_CONTROLLER;
typedef struct _PLATFORM_GPIO_CONTROLLER PLATFORM_GPIO_CONTROLLER;

struct _GPIO_CONTROLLER {
  UINTN    RegisterBase;
  UINTN    GpioIndex;
  UINTN    InternalGpioCount;
};

struct _PLATFORM_GPIO_CONTROLLER {
  UINTN              GpioCount;
  UINTN              GpioControllerCount;
  GPIO_CONTROLLER    *GpioController;
};

extern EFI_GUID  gPlatformGpioProtocolGuid;*/

#endif