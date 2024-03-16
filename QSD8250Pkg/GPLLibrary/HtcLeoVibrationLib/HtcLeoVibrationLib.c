/** @file
*
*  Copyright (c) 2023, J0SH1X <aljoshua.hell@gmail.com>. orginal code by cedesmith and all lk authors. All rights reserved.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
**/

#include <Library/DebugLib.h>
#include <Library/gpio.h>
#include <Library/TimerLib.h>
#include <Protocol/GpioTlmm.h>

// Cached copy of the Hardware Gpio protocol instance
TLMM_GPIO *gGpio = NULL;

VOID EFIAPI htcleo_vibrate(unsigned enable) {
  gGpio->Set(HTCLEO_GPIO_VIBRATOR_ON, enable);
}

VOID EFIAPI htcleo_vibrate_once(){
  htcleo_vibrate(1);
  MicroSecondDelay(100);
  htcleo_vibrate(0);
}

EFI_EVENT m_CallbackTimer         = NULL;
EFI_EVENT m_ExitBootServicesEvent = NULL;

// Callback function to disable the GPIO after a certain time
VOID EFIAPI htcleo_diable_vibrat_timer(IN EFI_EVENT Event, IN VOID *Context)
{
  // Disable the GPIO
  htcleo_vibrate(0);
}

// Function to enable the GPIO and schedule the callback
VOID htcleo_vibrate_timer(unsigned msecs)
{
  htcleo_vibrate(1);
  EFI_STATUS Status;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, htcleo_diable_vibrat_timer, NULL,
      &m_CallbackTimer);

  ASSERT_EFI_ERROR(Status);

  Status = gBS->SetTimer(
      m_CallbackTimer, TimerPeriodic, EFI_TIMER_PERIOD_MILLISECONDS(msecs));

  ASSERT_EFI_ERROR(Status);
}

RETURN_STATUS
EFIAPI
VibrationLibConstructor (
  VOID
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Find the gpio controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
