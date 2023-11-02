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

#include <Chipset/gpio.h>
#include <Chipset/iomap.h>
#include <Chipset/clock.h>

#include <Protocol/GpioTlmm.h>

static struct msm_gpio_isr_handler
{
	 int_handler handler;
	 void *arg;
	 bool pending;
} gpio_irq_handlers[NR_MSM_GPIOS];

static gpioregs *find_gpio(unsigned n, unsigned *bit)
{
	gpioregs *ret = 0;
	if (n > GPIO_REGS[ARRAY_SIZE(GPIO_REGS) - 1].end)
		goto end;

	for (unsigned i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		ret = GPIO_REGS + i;
		if (n >= ret->start && n <= ret->end) {
			*bit = 1 << (n - ret->start);
			break;
		}
	}

end:
	return ret;
}

int gpio_config(unsigned n, unsigned flags)
{
	gpioregs *r;
	unsigned b = 0;
	unsigned v;

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

void gpio_set(unsigned n, unsigned on)
{
	gpioregs *r;
	unsigned b = 0;
	unsigned v;

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

int gpio_get(unsigned n)
{
	gpioregs *r;
	unsigned b = 0;

	if ((r = find_gpio(n, &b)) == 0)
		return 0;
		
	gpio_config(n, GPIO_INPUT);

	return (readl(r->in) & b) ? 1 : 0;
}

void config_gpio_table(UINT32 *table, int len)
{
	int n;
	unsigned id;
	for (n = 0; n < len; n++) {
		id = table[n];
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	}
}

/*void msm_gpio_set_owner(unsigned gpio, unsigned owner)
{

	gpioregs *r;
	unsigned b = 0;
	unsigned v;

	if ((r = find_gpio(gpio, &b)) == 0)
		return;

	v = readl(r->owner);
	if (owner == MSM_GPIO_OWNER_ARM11) {
		writel(v | b, r->owner);
	} else {
		writel(v & (~b), r->owner);
	}
}*/

static void msm_gpio_update_both_edge_detect(unsigned gpio)
{
	gpioregs *r;
	unsigned b = 0;

	if ((r = find_gpio(gpio, &b)) == 0)
		return;

	int loop_limit = 100;
	unsigned pol, val, val2, intstat;
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

static int msm_gpio_irq_clear(unsigned gpio)
{
	gpioregs *r;
	unsigned b = 0;

	if ((r = find_gpio(gpio, &b)) == 0)
		return -1;

	writel(b, r->int_clear);
	
	return 0;
}

static void msm_gpio_irq_ack(unsigned gpio)
{
	msm_gpio_irq_clear(gpio);
	msm_gpio_update_both_edge_detect(gpio);
}

static enum handler_return msm_gpio_isr(void *arg)
{
	unsigned s, e;
	gpioregs *r;

	for (unsigned i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		r = GPIO_REGS + i;
		s = readl(r->int_status);
		e = readl(r->int_en);
		for (int j = 0; j < 32; j++)
			if (e & s & (1 << j)) {
				msm_gpio_irq_ack(r->start + j);
				gpio_irq_handlers[r->start + j].pending = 1;
		}
	}
	
	for (unsigned i = 0; i < ARRAY_SIZE(gpio_irq_handlers); i++) {
		if (gpio_irq_handlers[i].pending) {
			gpio_irq_handlers[i].pending = 0;
			if (gpio_irq_handlers[i].handler)
				gpio_irq_handlers[i].handler(gpio_irq_handlers[i].arg);
		}
	}
	
	return INT_RESCHEDULE;
}

void msm_gpio_init(void)
{
	for (unsigned i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		writel(-1, GPIO_REGS[i].int_clear);
		writel(0, GPIO_REGS[i].int_en);
	}

	register_int_handler(INT_GPIO_GROUP1, msm_gpio_isr, NULL);
	register_int_handler(INT_GPIO_GROUP2, msm_gpio_isr, NULL);

	unmask_interrupt(INT_GPIO_GROUP1);
	unmask_interrupt(INT_GPIO_GROUP2);
}

/*void msm_gpio_deinit(void)
{
	for (unsigned i = 0; i < ARRAY_SIZE(GPIO_REGS); i++) {
		writel(-1, GPIO_REGS[i].int_clear);
		writel(0, GPIO_REGS[i].int_en);
	}
	
	mask_interrupt(INT_GPIO_GROUP1);
	mask_interrupt(INT_GPIO_GROUP2);
}*/

EFI_STATUS
EFIAPI
GpioDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;

	msm_gpio_init();
	DEBUG((EFI_D_INFO, "Gpio init done!\n"));

	return Status;
}