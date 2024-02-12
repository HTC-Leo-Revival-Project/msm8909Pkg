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

#define LSENSOR_POLL_PROMESHUTOK   1000

HTCLEO_MICROP_PROTOCOL *gMicroP = NULL;
EFI_EVENT m_CallbackTimer = NULL;
EFI_EVENT EfiExitBootServicesEvent      = (EFI_EVENT)NULL;

static UINT16 lsensor_adc_table[10] = 
{
	0, 5, 20, 70, 150, 240, 330, 425, 515, 590
};

static struct lsensor_data 
{
	UINT32 old_level;
	INTN enabled;
	INTN opened;
} the_data;

// range: adc: 0..1023, value: 0..9
static void map_adc_to_level(UINT32 adc, UINT32 *value)
{
	INTN i;

	if (adc > 1024) {
		*value = 5; // set some good value at error
		return;
	}

	for (i = 9; i >= 0; i--) {
		if (adc >= lsensor_adc_table[i]) {
			*value = i;
			return;
		}
	}
	*value = 5;  // set some good value at error
}

INTN lightsensor_read_value(UINT32 *val)
{
	INTN ret;
	UINT8 data[2];

	if (!val)
		return -1;

	ret = microp_i2c_read(MICROP_I2C_RCMD_LSENSOR, data, 2);
	if (ret < 0) {
		return -1;
	}

	*val = data[1] | (data[0] << 8);
	
	return 0;
}

// static enum handler_return lightsensor_poll_function(struct timer *timer, time_t now, void *arg)
// {
// 	struct lsensor_data* p = &the_data;
// 	UINT32 adc = 0, level = 0;

// 	if (!p->enabled)
// 		goto error;
      
// 	lightsensor_read_value(&adc);
// 	map_adc_to_level(adc, &level);
// 	if (level != the_data.old_level) {
// 		the_data.old_level = level;
// 		htcleo_panel_set_brightness(level == 0 ? 1 : level);
// 	}
// 	timer_set_oneshot(timer, 500, lightsensor_poll_function, NULL);

// error:
// 	return INTN_RESCHEDULE;
// }


static INTN lightsensor_enable(void)
{
	struct lsensor_data* p = &the_data;
	INTN rc = -1;

	if (p->enabled)
		return 0;
	
	rc = gMicroP->CapellaCM3602Power(LS_PWR_ON, 1);
	if (rc < 0)
		return -1;
	
	the_data.old_level = -1;
	p->enabled = 1;

	return 0;
}

static INTN lightsensor_disable(void)
{
	struct lsensor_data* p = &the_data;
	INTN rc = -1;

	if (!p->enabled)
		return 0;
	
	rc = capella_cm3602_power(LS_PWR_ON, 0);
	if (rc < 0)
		return -1;

	p->enabled = 0;
	//timer_cancel(&lsensor_poll_timer);
	
	return 0;
}


static INTN lightsensor_open(void *arg)
{
	INTN rc = 0;
	
	//mutex_acquire(&api_lock);
	
	if (the_data.opened)
		rc = -1;

	the_data.opened = 1;
	
	//mutex_release(&api_lock);
	
	return rc;
}

static INTN lightsensor_release(void *arg)
{
	//mutex_acquire(&api_lock);
	
	the_data.opened = 0;
	
	//mutex_release(&api_lock);
	
	return 0;
}

INTN htcleo_lsensor_probe(void)
{  
	INTN ret = -1;    
	
	the_data.old_level = -1;
	the_data.enabled=0;
	the_data.opened=0;
	
	ret = lightsensor_enable();
	if (ret) {
		DEBUG((EFI_D_ERROR, "lightsensor_enable failed\n"));
		return -1;
	}

	// enter_critical_section();
	// timer_initialize(&lsensor_poll_timer);
	// timer_set_oneshot(&lsensor_poll_timer, 0, lightsensor_poll_function, NULL);
	// exit_critical_section();

    //toDo poll lightsensor here with a timer


	
	return ret;
}

VOID PollSensor(IN EFI_EVENT Event, IN VOID *Context){
	DEBUG((EFI_D_ERROR, "TIMER\n"));
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

  INTN ret = htcleo_lsensor_probe();
  if (ret == -1){
	DEBUG((EFI_D_ERROR, "Probing light sensor failed !!!\n"));
	MicroSecondDelay(5000);
  }else {
	DEBUG((EFI_D_ERROR, "Probing light sensor success !!!\n"));
	MicroSecondDelay(10000);
  }

  for(;;);

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