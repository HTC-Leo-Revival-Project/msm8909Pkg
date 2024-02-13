/* board-htcleo-ts.c
 *
 * Copyright (c) 2010 Cotulla
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

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/LKEnvLib.h>
#include <Library/reg.h>
#include <Library/adm.h>
#include <Library/gpio.h>
#include <Library/pcom.h>

#include <Chipset/gpio.h>
#include <Chipset/interrupts.h>
#include <Chipset/iomap.h>
#include <Chipset/irqs.h>
#include <Chipset/clock.h>
#include <Chipset/msm_i2c.h>

#include <Protocol/DevicePath.h>
#include <Protocol/GpioTlmm.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/HtcLeoI2C.h>

#include <Device/Gpio.h>

#include "TouchDxe.h"

static signed char irq2gpio[32] = {
	-1, -1, -1, -1, -1, -1,  0,  1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1,  2,  3,  4,  5,  6,
	 7,  8,  9, 10, 11, 12, -1, -1,
};

// Cached copy of the i2c protocol
HTCLEO_I2C_PROTOCOL *gI2C = NULL;

// Cached copy of the gpio protocol
TLMM_GPIO *gGpio = NULL;

// Cached copy of the Hardware Interrupt protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

/*
 * TODO: Add into a lib (?)
 */
int gpio_to_irq(int gpio)
{
	int irq;

	for (irq = 0; irq < 32; irq++) {
		if (irq2gpio[irq] == gpio)
			return irq;
	}
	return -2;
}

static int ts_i2c_read_sec(uint8_t dev, uint8_t addr, size_t count, uint8_t *buf)
{
	struct i2c_msg msg[2];
	
	msg[0].addr  = dev;
	msg[0].flags = 0;
	msg[0].len   = 1;
	msg[0].buf   = &addr;

	msg[1].addr  = dev;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = count;
	msg[1].buf   = buf;
	
	for (retry = 0; retry <= MSM_I2C_READ_RETRY_TIMES; retry++) {
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_sec: retrying %p time\n", retry));
		if (gI2C->Xfer(msg, 2) >= 0) {
			DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_sec SUCCESS!\n"));
			break;
		}
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_sec FAIL!\n"));
		MicroSecondDelay(5);
	}
	if (retry > MSM_I2C_WRITE_RETRY_TIMES) {
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_sec FAILED!\n"));
		return 0;
	}
	return 1;
}

static int ts_i2c_read_master(size_t count, uint8_t *buf)
{
	struct i2c_msg msg[1];
	
	msg[0].addr  = TYPE_2A_DEVID;
	msg[0].flags = I2C_M_RD;
	msg[0].len   = count;
	msg[0].buf   = buf;
	
	for (retry = 0; retry <= MSM_I2C_READ_RETRY_TIMES; retry++) {
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_master: retrying %p time\n", retry));
		if (gI2C->Xfer(msg, 1) >= 0) {
			DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_master SUCCESS!\n"));
			break;
		}
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_master FAIL!\n"));
		MicroSecondDelay(5);
	}
	if (retry > MSM_I2C_WRITE_RETRY_TIMES) {
		DEBUG((EFI_D_ERROR, "TS: ts_i2c_read_master FAILED!\n"));
		return 0;
	}
	
	return 1;
}

static void htcleo_ts_reset(void)
{
	while (gGpio->Get(HTCLEO_GPIO_TS_POWER)) {
		gGpio->Set(HTCLEO_GPIO_TS_SEL, 1);
		gGpio->Set(HTCLEO_GPIO_TS_POWER, 0);
		gGpio->Set(HTCLEO_GPIO_TS_MULT, 0);
		gGpio->Set(HTCLEO_GPIO_H2W_CLK, 0);
		MicroSecondDelay(10);
	}
	gGpio->Set(HTCLEO_GPIO_TS_MULT, 1);
	gGpio->Set(HTCLEO_GPIO_H2W_CLK, 1);

	while (!gGpio->Get(HTCLEO_GPIO_TS_POWER)) {
		gGpio->Set(HTCLEO_GPIO_TS_POWER, 1);
	}
	MicroSecondDelay(20);
	while (gGpio->Get(HTCLEO_GPIO_TS_IRQ)) {
		MicroSecondDelay(10);
	}

	while (gGpio->Get(HTCLEO_GPIO_TS_SEL)) {
		gGpio->Set(HTCLEO_GPIO_TS_SEL, 0);
	}
	MicroSecondDelay(300);
	DEBUG((EFI_D_ERROR, "TS: reset done\n"));
}

static void htcleo_ts_detect_type(void)
{
	uint8_t bt[4];
	if (ts_i2c_read_sec(TYPE_68_DEVID, 0x00, 1, bt)) {
		DEBUG((EFI_D_ERROR, "TS: DETECTED TYPE 68\n"));
		ts_type = TOUCH_TYPE_68;
		return;
	}
	if (ts_i2c_read_sec(TYPE_B8_DEVID, 0x00, 1, bt)) {
		DEBUG((EFI_D_ERROR, "TS: DETECTED TYPE B8\n"));
		ts_type = TOUCH_TYPE_B8;
		return;
	}
	if (ts_i2c_read_master(4, bt) && bt[0] == 0x55 ) {
		DEBUG((EFI_D_ERROR, "TS: DETECTED TYPE 2A\n"));
		ts_type = TOUCH_TYPE_2A;
		return;
	}
	ts_type = TOUCH_TYPE_UNKNOWN;
}

static void htcleo_init_ts(void)
{
	uint8_t bt[6];
	struct i2c_msg msg;
	
	bt[0] = 0xD0;
	bt[1] = 0x00;
	bt[2] = 0x01;
		
	msg.addr  = TYPE_2A_DEVID;
	msg.flags = 0;
	msg.len   = 3;
	msg.buf   = bt;
	
	gI2C->Xfer(&msg, 1);
	DEBUG((EFI_D_ERROR, "TS: init\n"));
}

void htcleo_ts_deinit(void) {
	gGpio->Set(HTCLEO_GPIO_TS_POWER, 0);
}

//static int htcleo_ts_work_func(void *arg)
//static enum handler_return ts_poll_fn(struct timer *timer, time_t now, void *arg)
VOID
EFIAPI
TouchPollFunction (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
  )
{
	uint8_t buf[9];
	uint32_t ptcount = 0;
	uint32_t ptx[2];
	uint32_t pty[2];

	if (!ts_i2c_read_master(9, buf)) {
		DEBUG((EFI_D_ERROR, "TS: ReadPos failed\n"));
		goto error;
	}
	if (buf[0] != 0x5A) {
		DEBUG((EFI_D_ERROR, "TS: ReadPos wrmark\n"));
		goto error;
	}
	ptcount = (buf[8] >> 1) & 3;
	if (ptcount > 2) {
		ptcount = 2;
	}
	if (ptcount >= 1) {
		ptx[0] = MAKEWORD(buf[2], (buf[1] & 0xF0) >> 4);
		pty[0] = MAKEWORD(buf[3], (buf[1] & 0x0F) >> 0);
	}
	if (ptcount == 2) {
		ptx[1] = MAKEWORD(buf[5], (buf[4] & 0xF0) >> 4);
		pty[1] = MAKEWORD(buf[6], (buf[4] & 0x0F) >> 0);
	}
#if 1 //Set to 0 if ts driver is working(check splash.h too for virtual buttons logo)
	if (ptcount == 0) {
		DEBUG((EFI_D_ERROR, "TS: not pressed\n"));
	}
	else if (ptcount == 1) {
		DEBUG((EFI_D_ERROR, "TS: pressed1 (%d, %d)\n", ptx[0], pty[0]));
	}
	else if (ptcount == 2) {
		DEBUG((EFI_D_ERROR, "TS: pressed2 (%d, %d) (%d, %d)\n", ptx[0], pty[0], ptx[1], pty[1]));
	}
	else {
		DEBUG((EFI_D_ERROR, "TS: BUGGY!\n"));
	}
#else
	/*
	 * ...WIP...
	 * koko: Catch on screen buttons events.
	 *		 According to the screen area touched
	 *		 set the corresponding key_code
	 */
	unsigned key_code = 0;
	if (ptcount == 1) {
		if (pty[0] > 723) {
				 if ((ptx[0] < 90)) 					key_code = KEY_HOME;
			else if ((ptx[0] > 90) && (ptx[0] < 180))	key_code = KEY_VOLUMEUP;
			else if ((ptx[0] >180) && (ptx[0] < 300))	key_code = KEY_SEND;
			else if ((ptx[0] >300) && (ptx[0] < 390))	key_code = KEY_VOLUMEDOWN;
			else if ((ptx[0] >390))						key_code = KEY_BACK;
			else 										key_code = 0;
		}
		if (key_code)
			keys_set_state(key_code);
	}
#endif
	/*enter_critical_section();
	timer_set_oneshot(timer, 125, ts_poll_fn, NULL);
	exit_critical_section();*/

error:
	//enter_critical_section();
	gInterrupt->DisableInterruptSource(gInterrupt, gpio_to_irq(HTCLEO_GPIO_TS_IRQ));
	//exit_critical_section();
}

/* 
//static enum handler_return htcleo_ts_irq_handler(void *arg)
VOID
EFIAPI
TouchIrqHandler (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
  )
{
	//enter_critical_section();
	gInterrupt->EnableInterruptSource(gInterrupt, gpio_to_irq(HTCLEO_GPIO_TS_IRQ));
	//wait_queue_wake_one(&ts_work, 1, 0);
	//exit_critical_section();
	
	gInterrupt->EndOfInterrupt(gInterrupt, gpio_to_irq(HTCLEO_GPIO_TS_IRQ));
}
 */
static uint32_t touch_on_gpio_table[] =
{
	MSM_GPIO_CFG(HTCLEO_GPIO_TS_IRQ, 0, PCOM_GPIO_CFG_INPUT, PCOM_GPIO_CFG_PULL_UP, PCOM_GPIO_CFG_8MA),
};

EFI_STATUS
EFIAPI
TouchDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  //EFI_HANDLE  Handle = NULL;

  //
  // Make sure the Gpio protocol has not been installed in the system yet.
  //
  //ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gTlmmGpioProtocolGuid);

  // Find the interrupt controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  ASSERT_EFI_ERROR (Status);

  // Find the i2c protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHtcLeoI2CProtocolGuid, NULL, (VOID **)&gI2C);
  ASSERT_EFI_ERROR (Status);

  // Find the gpio controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
  ASSERT_EFI_ERROR (Status);

  //htcleo_ts_probe:
  gGpio->ConfigTable(touch_on_gpio_table, ARRAY_SIZE(touch_on_gpio_table));
  gGpio->Set(HTCLEO_GPIO_TS_SEL, 1);
  gGpio->Set(HTCLEO_GPIO_TS_POWER, 0);
  gGpio->Set(HTCLEO_GPIO_TS_MULT, 0);
  gGpio->Set(HTCLEO_GPIO_H2W_CLK, 0);
  gGpio->Config(HTCLEO_GPIO_TS_IRQ, GPIO_INPUT);

  MicroSecondDelay(100);

  htcleo_ts_reset();
  htcleo_ts_detect_type();
  if (ts_type == TOUCH_TYPE_B8) {
	DEBUG((EFI_D_ERROR, "TS: NOT SUPPORTED\n"));
	goto error;
  } else
  if (ts_type == TOUCH_TYPE_UNKNOWN) {
	DEBUG((EFI_D_ERROR, "TS: NOT DETECTED\n"));
	goto error;
  }
  htcleo_init_ts();

  DEBUG((EFI_D_ERROR, "TS: INITED\n"));
  MicroSecondDelay(10000000);

  //gInterrupt->RegisterInterruptSource(gInterrupt, gpio_to_irq(HTCLEO_GPIO_TS_IRQ), TouchIrqHandler);

	/*
	enter_critical_section();
	//register_int_handler(gpio_to_irq(HTCLEO_GPIO_TS_IRQ), htcleo_ts_irq_handler, NULL);
	timer_initialize(&poll_timer);
	timer_set_oneshot(&poll_timer, 0, ts_poll_fn, NULL);
	exit_critical_section();
	*/
	
  Status = EFI_SUCCESS;
  return Status;

  // Install the Tlmm GPIO Protocol onto a new handle
  /*Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gTlmmGpioProtocolGuid,
                  &gGpio,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
  }*/
	
error:
  DEBUG((EFI_D_ERROR, "TS: ERROR, DEINITING!\n"));
  MicroSecondDelay(100000);
  htcleo_ts_deinit();
  //wait_queue_destroy(&ts_work);

  for(;;) {};

  Status = -1;
  return Status;
}