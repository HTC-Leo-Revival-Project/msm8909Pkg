/** @file
 * Charging App
 *
 * Copyright (c) 2023, J0SH1X <aljoshua.hell@gmail.com>
 * Copyright (C) 2011-2012 Shantanu Gupta <shans95g@gmail.com>
 * Copyright (C) 2010-2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
 * Copyright (C) 2007-2011 htc-linux.org and XDANDROID project
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
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

#if SOC == 1
#include <Chipset/iomap_qsd8250.h>
#elif SOC == 2
#include <Chipset/iomap_msm7230.h>
#endif
#include <Chipset/irqs.h>
#include <Chipset/timer.h>
#include <Library/gpio.h>
#include <Library/DS2746.h>
#include <Library/hsusb.h>

#include <Device/Gpio.h>
#include <Device/microp.h>

#include <Protocol/GpioTlmm.h>
#include <Protocol/HtcLeoMicroP.h>

#define USB_STATUS        0xef20c

#define VOLTAGE_3700 0
#define VOLTAGE_3800 1
#define VOLTAGE_3900 2
#define VOLTAGE_4000 3
#define VOLTAGE_4100 4
#define VOLTAGE_4200 5

EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT EfiExitBootServicesEvent      = (EFI_EVENT)NULL;

// Cached copy of the Hardware Gpio protocol instance
TLMM_GPIO *gGpio = NULL;
HTCLEO_MICROP_PROTOCOL *gMicroP = NULL;

enum PSY_CHARGER_STATE {
	CHG_OFF,
	CHG_AC,
	CHG_USB_LOW,
	CHG_USB_HIGH,
	CHG_OFF_FULL_BAT,
};

enum PSY_CHARGER_STATE gState = CHG_OFF;

// TODO: Remove this function, make use of the USB protocol instead
void UlpiWrite(unsigned val, unsigned reg)
{
	unsigned timeout = 10000;
	
	/* initiate write operation */
	MmioWrite32(USB_ULPI_VIEWPORT, ULPI_RUN | ULPI_WRITE | ULPI_ADDR(reg) | ULPI_DATA(val));

	/* wait for completion */
	while ((MmioRead32(USB_ULPI_VIEWPORT) & ULPI_RUN) && (--timeout));
}

static void EFIAPI SetCharger(enum PSY_CHARGER_STATE state)
{
  gState = state;
	switch (state) {
		case CHG_USB_LOW:
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			break;
		case CHG_AC: case CHG_USB_HIGH:
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 0);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			break;
		case CHG_OFF_FULL_BAT:
		case CHG_OFF:
		default:	
			// 0 enable; 1 disable;
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_ENABLE, GPIO_OUTPUT);
			gGpio->Set(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, 1);
			gGpio->Config(HTCLEO_GPIO_BATTERY_CHARGER_CURRENT, GPIO_OUTPUT);
			/*gGpio->Set(HTCLEO_GPIO_POWER_USB, 0);
			gGpio->Config(HTCLEO_GPIO_POWER_USB, GPIO_OUTPUT);*/
			break;
	}
}

/*
 * koko: ..._chg_voltage_threshold is used along ds2746 driver
 *		 to detect when the battery is FULL.
 *		 Consider setting it to a value lower than DS2746_HIGH_VOLTAGE.
 *		 Avoiding full charge has benefits, and some manufacturers set
 *		 the charge threshold lower on purpose to prolong battery life.
 * ######################################################################
 * #			Typical charge characteristics of lithium-ion			#
 * ######################################################################
 * # Charge V/cell #   Capacity at	 # Charge time # Capacity with full #
 * #			   # cut-off voltage #			   #	 saturation     #
 * ######################################################################
 * #	 3.80	   #		60%		 #	 120 min   #		65%			#
 * #	 3.90	   #		70%		 #	 135 min   #		76%			#
 * #	 4.00	   #		75%		 #	 150 min   #		82%			#
 * #	 4.10	   #		80%		 #	 165 min   #		87%			#
 * #	 4.20	   #		85%		 #	 180 min   #	   100%			#
 * ######################################################################
 */
static UINT32 default_chg_voltage_threshold[] =
{
	3700,
	3800,
	3900,
	4000,
	4100,
	4200,//DS2746_HIGH_VOLTAGE
};

BOOLEAN  
CheckUsbStatus()
{
  return !!MmioRead32(MSM_SHARED_BASE + USB_STATUS);
}

BOOLEAN
IsAcOnline() 
{
	return !!((MmioRead32(USB_PORTSC) & PORTSC_LS) == PORTSC_LS);
}

VOID EFIAPI WantsCharging(
    IN EFI_EVENT Event, 
    IN VOID *Context)
{
  UINT32 voltage = 0;
  
  if (CheckUsbStatus())
  {
    voltage = ds2746_voltage(DS2746_I2C_SLAVE_ADDR);
    DEBUG((EFI_D_INFO, "ChargingApp: Battery Voltage is: %d\n", voltage));

    if(voltage < default_chg_voltage_threshold[VOLTAGE_4100]) {
      DEBUG((EFI_D_INFO, "ChargingApp: Voltage < default_threshold\n"));
      // If battery needs charging, set new charger state
      if (IsAcOnline()) {
        if (gState != CHG_AC ) {
          MmioWrite32(USB_USBCMD, 0x00080000);
          UlpiWrite(0x48, 0x04);
          SetCharger(CHG_AC);
        }
      } else {
        if (gState != CHG_USB_LOW ) {
          MmioWrite32(USB_USBCMD, 0x00080001);
          MicroSecondDelay(10);
          SetCharger(CHG_USB_LOW);
        }
      }
      DEBUG((EFI_D_INFO, "ChargingApp: Charging enabled\n"));
      // and turn LED amber
      gMicroP->LedSetMode(LED_AMBER);
    }
    else {
      // Battery is full
      DEBUG((EFI_D_INFO, "ChargingApp: Voltage >= default_threshold\n"));
      // Set charger state to CHG_OFF_FULL_BAT
      if (gState != CHG_OFF_FULL_BAT ) {
        MmioWrite32(USB_USBCMD, 0x00080001);
        MicroSecondDelay(10);
        SetCharger(CHG_OFF_FULL_BAT);
      }
      // and turn LED green
      gMicroP->LedSetMode(LED_GREEN);
    }
  }
  else {
    // Set charger state to CHG_OFF
    if (gState != CHG_OFF ) {
      MmioWrite32(USB_USBCMD, 0x00080001);
      MicroSecondDelay(10);
      SetCharger(CHG_OFF);
    }
    // and turn off led
    gMicroP->LedSetMode(LED_OFF);
  }
}

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  // Set charger state to CHG_OFF
    if (gState != CHG_OFF ) {
      MmioWrite32(USB_USBCMD, 0x00080001);
      MicroSecondDelay(10);
      SetCharger(CHG_OFF);
    }
    // and turn off led
    gMicroP->LedSetMode(LED_OFF);
}

EFI_STATUS 
EFIAPI 
ChargingDxeInit(
  IN EFI_HANDLE ImageHandle, 
  IN EFI_SYSTEM_TABLE *SystemTable) 
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Find the gpio controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
  ASSERT_EFI_ERROR (Status);

  // Find the MicroP protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHtcLeoMicropProtocolGuid, NULL, (VOID **)&gMicroP);
  ASSERT_EFI_ERROR (Status);

  // Install a timer to check for want charging every 100ms
  Status = gBS->CreateEvent (
                EVT_TIMER | EVT_NOTIFY_SIGNAL,  // Type
                TPL_NOTIFY,                     // NotifyTpl
                WantsCharging,                  // NotifyFunction
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