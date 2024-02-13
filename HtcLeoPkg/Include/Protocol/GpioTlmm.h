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
  IN  TLMM_GPIO_PIN   Gpio
  );

/*++

Routine Description:

  Gets the state of a GPIO pin

Arguments:

  Gpio  - which pin to read

Returns:

    Value - state of the pin

--*/

typedef
VOID
(*TLMM_GPIO_SET)(
  IN TLMM_GPIO_PIN  Gpio,
  IN UINTN          Mode
  );

/*++

Routine Description:

  Sets the state of a GPIO pin

Arguments:

  Gpio  - which pin to modify
  Mode  - mode to set

--*/

typedef
UINTN
(*TLMM_GPIO_CONFIG)(
    IN TLMM_GPIO_PIN  Gpio,
    IN UINTN          Flags
    );

/*++

Routine Description:

  Configures a GPIO pin

Arguments:

  Gpio  - which pin to modify
  Flags - flags to set

--*/

typedef
VOID
(*TLMM_GPIO_CONFIG_TABLE)(
    IN UINT32  *Table,
    IN int     Len
    );

/*++

Routine Description:

  Configures a GPIO table

Arguments:

  Table - pointer to the gpio table
  Len - length

--*/


struct _TLMM_GPIO {
  TLMM_GPIO_GET          Get;
  TLMM_GPIO_SET          Set;
  TLMM_GPIO_CONFIG       Config;
  TLMM_GPIO_CONFIG_TABLE ConfigTable;
};

extern EFI_GUID  gTlmmGpioProtocolGuid;

#endif