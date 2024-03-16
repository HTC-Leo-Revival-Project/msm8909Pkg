/** @file
 *
 *  Copyright (c) 2023, J0SH1X <aljoshua.hell@gmail.com>.
 *  Orginal code by cedesmith and all lk authors. All rights reserved.
 *
 *  This program and the accompanying materials
 *  are licensed and made available under the terms and conditions of the BSD License
 *  which accompanies this distribution.  The full text of the license may be found at
 *  http://opensource.org/licenses/bsd-license.php
 *
 *  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 *
 **/

#ifndef _HTCLEO_VIBRATE_H_
#define _HTCLEO_VIBRATE_H_

#include <Library/DebugLib.h>
#include <Library/qcom_qsd8250_iomap.h>
#include <Device/Gpio.h>
#include <Library/qcom_lk.h>
#include <Library/qcom_qsd8250_timer.h>
#include <Library/gpio.h>

/**
 * @brief Enables or disables the vibrator on the HTC Leo device
 *
 * @param enable Set to non-zero value to enable vibrator, zero to disable
 *
 * @retval None
 **/
VOID EFIAPI htcleo_vibrate(unsigned enable);

/**
 * @brief Vibrates the HTC Leo device once
 *
 * @retval None
 **/
VOID EFIAPI htcleo_vibrate_once();

/**
 * @brief Vibrates the HTC Leo device for a specified duration
 *
 * @param msecs Duration of the vibration in milliseconds
 *
 * @retval None
 **/
VOID EFIAPI htcleo_vibrate_timer(unsigned msecs);

#endif // _HTCLEO_VIBRATE_H_
