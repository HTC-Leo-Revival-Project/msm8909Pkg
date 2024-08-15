/*
 * Copyright (c) 2009-2010, Google Inc. All rights reserved.
 * Copyright (c) 2009-2012, Code Aurora Forum. All rights reserved.
 * Copyright (c) 2011-2012, Shantanu Gupta <shans95g@gmail.com>
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
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
#include <Library/IoLib.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/LKEnvLib.h>
#include <Library/TimerLib.h>
#include <Library/pcom.h>

#include <Chipset/clock.h>

/* This function always returns 0, unless future versions check for
 * modem crash.
 * In that case it might return error.
 * This is called very early, dont debug here.
 */
static int pcom_wait_for(UINT32 addr, UINT32 value)
{
	while (1) {

		if (readl(addr) == value){
			return 0;
		}
		NanoSecondDelay(5);
	}
}

int msm_pcom_wait_for_modem_ready()
{
	// returns 0 to indicate success
	// we return ~0 to indicate modem ready
	return	~pcom_wait_for(MDM_STATUS,	PCOM_READY);
}

static inline void notify_modem(void)
{
	ArmDataSynchronizationBarrier();
	//wait for all the previous data to be written then fire
	writel(1, MSM_A2M_INT(6));
}

int msm_proc_comm(unsigned cmd, unsigned *data1, unsigned *data2)
{
	int ret = -1;
	unsigned status;

	msm_pcom_wait_for_modem_ready();
	
	writel(cmd, APP_COMMAND);
	writel(data1 ? *data1 : 0, APP_DATA1);
	writel(data2 ? *data2 : 0, APP_DATA2);
	
	notify_modem();
	
	pcom_wait_for(APP_COMMAND, PCOM_CMD_DONE);
	
	status = readl(APP_STATUS);

	if (status != PCOM_CMD_FAIL) {
		if (data1) *data1 = readl(APP_DATA1);
		if (data2) *data2 = readl(APP_DATA2);
		ret = 0;
	} else {
		DEBUG((EFI_D_INFO, "[PCOM]: FAIL [cmd:%x D1:%x D2:%x]\n", cmd, (unsigned)data1, (unsigned)data2));
		ret = -1;
	}

	writel(PCOM_CMD_IDLE, APP_COMMAND);

	return ret;
}

void msm_pcom_init(void)
{
	writel(PCOM_CMD_IDLE, APP_COMMAND);
	writel(PCOM_INVALID_STATUS, APP_STATUS);
}

int pcom_gpio_tlmm_config(unsigned config, unsigned disable)
{
	return msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &config, &disable);
}

int pcom_vreg_set_level(unsigned id, unsigned mv)
{
	return msm_proc_comm(PCOM_VREG_SET_LEVEL, &id, &mv);
}

int pcom_vreg_enable(unsigned id)
{
	unsigned int enable = 1;
	return msm_proc_comm(PCOM_VREG_SWITCH, &id, &enable);
}

int pcom_vreg_disable(unsigned id)
{
	unsigned int enable = 0;
	return msm_proc_comm(PCOM_VREG_SWITCH, &id, &enable);
}

int pcom_clock_enable(unsigned id)
{
	return msm_proc_comm(PCOM_CLKCTL_RPC_ENABLE, &id, 0);
}

int pcom_clock_disable(unsigned id)
{
	return msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, 0);
}

int pcom_clock_is_enabled(unsigned id)
{
	return msm_proc_comm(PCOM_CLKCTL_RPC_ENABLED, &id, 0);
}

int pcom_clock_set_rate(unsigned id, unsigned rate)
{
	return msm_proc_comm(PCOM_CLKCTL_RPC_SET_RATE, &id, &rate);
}

int pcom_clock_get_rate(unsigned id)
{
	if (msm_proc_comm(PCOM_CLKCTL_RPC_RATE, &id, 0)) {
		return -1;
	} else {
		return (int)id;
	}
}

int pcom_set_clock_flags(unsigned id, unsigned flags)
{
	return msm_proc_comm(PCOM_CLKCTL_RPC_SET_FLAGS, &id, &flags);
}
