/* @file
 * Port of little-kernel QSD8250 GPIO driver for UEFI
 *
 * Copyright (C) 2008, Google Inc. All rights reserved.
 * Copyright (C) 2011, htc-linux.org 
 * Copyrught (C) 2012, shantanu gupta <shans95g@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>

#include <Library/LKEnvLib.h>
#include <Library/reg.h>
#include <Library/adm.h>
#include <Library/gpio.h>
#include <Library/pcom.h>

#if SOC == 1
#include <Chipset/iomap_qsd8250.h>
#include <Chipset/irqs_qsd8250.h>
#include <Chipset/gpio_qsd8250.h>
#elif SOC == 2
#include <Chipset/iomap_msm7230.h>
#include <Chipset/irqs_msm7230.h>
#include <Chipset/gpio_msm7230.h>
#endif

#include <Chipset/clock.h>

#include <Protocol/GpioTlmm.h>
#include <Protocol/HardwareInterrupt.h>

// Cached copy of the Hardware Interrupt protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

static gpioregs GPIO_REGS[] = {
	{
	.out 		= GPIO_OUT_0,
	.in 		= GPIO_IN_0,
	.int_status = GPIO_INT_STATUS_0,
	.int_clear 	= GPIO_INT_CLEAR_0,
	.int_en 	= GPIO_INT_EN_0,
	.int_edge 	= GPIO_INT_EDGE_0,
	.int_pos 	= GPIO_INT_POS_0,
	.oe 		= GPIO_OE_0,
	.owner 		= 0,
	.start 		= 0,
	.end 		= 15,
	},
	{
	.out 		= GPIO_OUT_1,
	.in 		= GPIO_IN_1,
	.int_status = GPIO_INT_STATUS_1,
	.int_clear 	= GPIO_INT_CLEAR_1,
	.int_en 	= GPIO_INT_EN_1,
	.int_edge 	= GPIO_INT_EDGE_1,
	.int_pos 	= GPIO_INT_POS_1,
	.oe 		= GPIO_OE_1,
	.owner 		= 0,
	.start	 	= 16,
	.end 		= 42,
	},
	{
	.out 		= GPIO_OUT_2,
	.in 		= GPIO_IN_2,
	.int_status = GPIO_INT_STATUS_2,
	.int_clear 	= GPIO_INT_CLEAR_2,
	.int_en 	= GPIO_INT_EN_2,
	.int_edge 	= GPIO_INT_EDGE_2,
	.int_pos 	= GPIO_INT_POS_2,
	.oe 		= GPIO_OE_2,
	.owner 		= 0,
	.start 		= 43,
	.end 		= 67,
	},
	{
	.out 		= GPIO_OUT_3,
	.in 		= GPIO_IN_3,
	.int_status = GPIO_INT_STATUS_3,
	.int_clear 	= GPIO_INT_CLEAR_3,
	.int_en 	= GPIO_INT_EN_3,
	.int_edge 	= GPIO_INT_EDGE_3,
	.int_pos 	= GPIO_INT_POS_3,
	.oe 		= GPIO_OE_3,
	.owner 		= 0,
	.start 		= 68,
	.end 		= 94
	},
	{
	.out 		= GPIO_OUT_4,
	.in 		= GPIO_IN_4,
	.int_status = GPIO_INT_STATUS_4,
	.int_clear 	= GPIO_INT_CLEAR_4,
	.int_en 	= GPIO_INT_EN_4,
	.int_edge 	= GPIO_INT_EDGE_4,
	.int_pos 	= GPIO_INT_POS_4,
	.oe 		= GPIO_OE_4,
	.owner 		= 0,
	.start 		= 95,
	.end 		= 103,
	},
	{
	.out 		= GPIO_OUT_5,
	.in 		= GPIO_IN_5,
	.int_status = GPIO_INT_STATUS_5,
	.int_clear 	= GPIO_INT_CLEAR_5,
	.int_en 	= GPIO_INT_EN_5,
	.int_edge 	= GPIO_INT_EDGE_5,
	.int_pos 	= GPIO_INT_POS_5,
	.oe 		= GPIO_OE_5,
	.owner 		= 0,
	.start 		= 104,
	.end 		= 121,
	},
	{
	.out        = GPIO_OUT_6,
	.in         = GPIO_IN_6,
	.int_status = GPIO_INT_STATUS_6,
	.int_clear  = GPIO_INT_CLEAR_6,
	.int_en     = GPIO_INT_EN_6,
	.int_edge   = GPIO_INT_EDGE_6,
	.int_pos    = GPIO_INT_POS_6,
	.oe         = GPIO_OE_6,
	.owner      = 0,
	.start      = 122,
	.end        = 152,
	},
	{
	.out        = GPIO_OUT_7,
	.in         = GPIO_IN_7,
	.int_status = GPIO_INT_STATUS_7,
	.int_clear  = GPIO_INT_CLEAR_7,
	.int_en     = GPIO_INT_EN_7,
	.int_edge   = GPIO_INT_EDGE_7,
	.int_pos    = GPIO_INT_POS_7,
	.oe         = GPIO_OE_7,
	.owner      = 0,
	.start      = 153,
	.end        = 164,
	},
};

static
gpioregs
*find_gpio(UINTN n, UINTN *bit)
{
	gpioregs *ret = 0;
	if (n > GPIO_REGS[ARRAY_SIZE(GPIO_REGS) - 1].end)
		goto end;

	for (UINTN i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		ret = GPIO_REGS + i;
		if (n >= ret->start && n <= ret->end) {
			*bit = 1 << (n - ret->start);
			break;
		}
	}

end:
	return ret;
}

UINTN
gpio_config(UINTN n, UINTN flags)
{
	gpioregs *r;
	UINTN b = 0;
	UINTN v;

	if ((r = find_gpio(n, &b)) == 0)
		return -1;

	v = readl(r->oe);
	if (flags & GPIO_OUTPUT) {
		writel(v | b, r->oe);
	} else {
		writel(v & (~b), r->oe);
	}
	
	return 0;
}

VOID
gpio_set(UINTN n, UINTN on)
{
	gpioregs *r;
	UINTN b = 0;
	UINTN v;

	if ((r = find_gpio(n, &b)) == 0)
		return;
		
	gpio_config(n, GPIO_OUTPUT);

	v = readl(r->out);
	if (on) {
		writel(v | b, r->out);
	} else {
		writel(v & (~b), r->out);
	}
}

UINTN
gpio_get(UINTN n)
{
	gpioregs *r;
	UINTN b = 0;

	if ((r = find_gpio(n, &b)) == 0)
		return 0;
		
	gpio_config(n, GPIO_INPUT);

	return (readl(r->in) & b) ? 1 : 0;
}

VOID
config_gpio_table(UINT32 *table, int len)
{
	int n;
	UINTN id;
	for (n = 0; n < len; n++) {
		id = table[n];
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	}
}

static void msm_gpio_update_both_edge_detect(UINTN gpio)
{
	gpioregs *r;
	UINTN b = 0;

	if ((r = find_gpio(gpio, &b)) == 0)
		return;

	int loop_limit = 100;
	UINTN pol, val, val2, intstat;
	do {
		val = readl(r->in);
		pol = readl(r->int_pos);
		pol |= ~val;
		writel(pol, r->int_pos);
		intstat = readl(r->int_status);
		val2 = readl(r->in);
		if (((val ^ val2) & ~intstat) == 0)
			return;
	} while (loop_limit-- > 0);
	DEBUG((EFI_D_INFO, "%s, failed to reach stable state %x != %x\n", __func__,val, val2));
}

static int msm_gpio_irq_clear(UINTN gpio)
{
	gpioregs *r;
	UINTN b = 0;

	if ((r = find_gpio(gpio, &b)) == 0)
		return -1;

	writel(b, r->int_clear);
	
	return 0;
}

static void msm_gpio_irq_ack(UINTN gpio)
{
	msm_gpio_irq_clear(gpio);
	msm_gpio_update_both_edge_detect(gpio);
}

VOID
EFIAPI
MsmGpioIsr (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
  )
{
	UINTN s, e;
	gpioregs *r;

	for (UINTN i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		r = GPIO_REGS + i;
		s = readl(r->int_status);
		e = readl(r->int_en);
		for (int j = 0; j < 32; j++)
			if (e & s & (1 << j)) {
				msm_gpio_irq_ack(r->start + j);
		}
	}
	
	// Handler part stripped
}

/**
 Protocol variable definition
 **/
TLMM_GPIO  gGpio = {
  gpio_get,
  gpio_set,
  gpio_config
};

EFI_STATUS
EFIAPI
GpioDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  EFI_HANDLE  Handle = NULL;

  //
  // Make sure the Gpio protocol has not been installed in the system yet.
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gTlmmGpioProtocolGuid);

  // Find the interrupt controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  ASSERT_EFI_ERROR (Status);

  // Clear
  for (UINTN i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
	MmioWrite32(GPIO_REGS[i].int_clear, -1);
	MmioWrite32(GPIO_REGS[i].int_en, 0);
  }

  // Install interrupt handler
  Status = gInterrupt->RegisterInterruptSource(gInterrupt, INT_GPIO_GROUP1, MsmGpioIsr);
  ASSERT_EFI_ERROR (Status);
  Status = gInterrupt->RegisterInterruptSource(gInterrupt, INT_GPIO_GROUP2, MsmGpioIsr);
  ASSERT_EFI_ERROR (Status);

  // Install the Tlmm GPIO Protocol onto a new handle
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gTlmmGpioProtocolGuid,
                  &gGpio,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
  }

	return Status;
}