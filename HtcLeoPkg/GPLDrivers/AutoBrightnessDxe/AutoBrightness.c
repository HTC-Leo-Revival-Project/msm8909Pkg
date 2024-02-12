/* 
* koko: Porting light sensor support for HTC LEO to LK
* 		Base code copied from Cotulla's board-htcleo-ls.c
* J0SH1X: Porting light sensor support for HTC LEO to EDK2
* 		Base code copied from kokos htcleo-ls.c
*
* Copyright (C) 2010 Cotulla
* Copyright (C) 2024 J0SH1X <aljoshua.hell@gmail.com>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Device/microp.h>
#include <Chipset/timer.h>
#include <Protocol/HtcLeoMicroP.h>



HTCLEO_MICROP_PROTOCOL *gMicroP = NULL;
EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT EfiExitBootServicesEvent      = (EFI_EVENT)NULL;



VOID PollSensor(IN EFI_EVENT Event, IN VOID *Context){
	DEBUG((EFI_D_INFO, "TIMER\n"));
}

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  // Set charger state to CHG_OFF
}

EFI_STATUS EFIAPI AutoBrightnessDxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable){
	EFI_STATUS Status = EFI_SUCCESS;


  // Find the MicroP protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHtcLeoMicropProtocolGuid, NULL, (VOID **)&gMicroP);
  ASSERT_EFI_ERROR (Status);

  // Install a timer to check for want charging every 100ms
  Status = gBS->CreateEvent (
                EVT_TIMER | EVT_NOTIFY_SIGNAL,  // Type
                TPL_NOTIFY,                     // NotifyTpl
                PollSensor,                  // NotifyFunction
                NULL,                           // NotifyContext
                &m_CallbackTimer                // Event
                );
  ASSERT_EFI_ERROR(Status);

  //
  // Program the timer event to be signaled every 100 ms.
  //
  Status = gBS->SetTimer (
                m_CallbackTimer,
                TimerPeriodic,
                EFI_TIMER_PERIOD_MILLISECONDS (100)
                );
  ASSERT_EFI_ERROR(Status);

  // Register for an ExitBootServicesEvent
  Status = gBS->CreateEvent(EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY, ExitBootServicesEvent, NULL, &EfiExitBootServicesEvent);
  ASSERT_EFI_ERROR(Status);

  return Status;
}