/*
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * Copyright (c) 2009 Google, Inc.
 * Copyright (c) 2012 Shantanu Gupta <shans95g@gmail.com>
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

#include <Library/pcom.h>
#include <Library/LKEnvLib.h>
#if SOC == 1
#include <Chipset/iomap_qsd8250.h>
#elif SOC == 2
#include <Chipset/iomap_msm7230.h>
#endif
#include <Chipset/clock.h>
#include <Library/acpuclock.h>
#include <Library/TimerLib.h>

unsigned acpuclk_init_done = 0;

#define SHOT_SWITCH  4
#define HOP_SWITCH   5
#define SIMPLE_SLEW  6
#define COMPLEX_SLEW 7

#define SPSS_CLK_CNTL_ADDR		(MSM_CSR_BASE + 0x100)
#define SPSS_CLK_SEL_ADDR		(MSM_CSR_BASE + 0x104)

#define SCPLL_CTL_ADDR			(MSM_SCPLL_BASE + 0x4)
#define SCPLL_STATUS_ADDR		(MSM_SCPLL_BASE + 0x18)
#define SCPLL_FSM_CTL_EXT_ADDR	(MSM_SCPLL_BASE + 0x10)

#define CLK_TCXO		0 /* 19.2 MHz */
#define CLK_GLOBAL_PLL	1 /* 768 MHz */
#define CLK_MODEM_PLL	4 /* 245 MHz (UMTS) or 235.93 MHz (CDMA) */

#define CCTL(src, div) (((src) << 4) | (div - 1))

#define SRC_RAW		0 /* clock from SPSS_CLK_CNTL */
#define SRC_SCPLL	1 /* output of scpll 128-998 MHZ */

/* Set rate and enable the clock */
void clock_config(UINT32 ns, UINT32 md, UINT32 ns_addr, UINT32 md_addr)
{
	unsigned int val = 0;

	/* Activate the reset for the M/N Counter */
	val = 1 << 7;
	writel(val, ns_addr);

	/* Write the MD value into the MD register */
	if (md_addr != 0x0)
		writel(md, md_addr);

	/* Write the ns value, and active reset for M/N Counter, again */
	val = 1 << 7;
	val |= ns;
	writel(val, ns_addr);

	/* De-activate the reset for M/N Counter */
	val = 1 << 7;
	val = ~val;
	val = val & readl(ns_addr);
	writel(val, ns_addr);

	/* Enable the Clock Root */
	val = 1 << 11;
	val = val | readl(ns_addr);
	writel(val, ns_addr);

	/* Enable the Clock Branch */
	val = 1 << 9;
	val = val | readl(ns_addr);
	writel(val, ns_addr);

	/* Enable the M/N Counter */
	val = 1 << 8;
	val = val | readl(ns_addr);
	writel(val, ns_addr);
}

struct clkctl_acpu_speed {
	unsigned acpu_khz;
	unsigned clk_cfg;
	unsigned clk_sel;
	unsigned sc_l_value;
	int      vdd;
	unsigned axiclk_khz;
};

struct clkctl_acpu_speed acpu_freq_tbl[] = {
	{ 245000, CCTL(CLK_MODEM_PLL, 1),	SRC_RAW,   0x00, 1050, 29000 },
	{ 384000, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0A, 1050, 58000 },
	{ 422400, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0B, 1050, 117000 },
	{ 460800, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0C, 1050, 117000 },
	{ 499200, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0D, 1075, 117000 },
	{ 537600, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0E, 1100, 117000 },
	{ 576000, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x0F, 1100, 117000 },
	{ 614400, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x10, 1125, 117000 },
	{ 652800, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x11, 1150, 117000 },
	{ 691200, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x12, 1175, 117000 },
	{ 729600, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x13, 1200, 117000 },
	{ 768000, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x14, 1200, 128000 },
	{ 806400, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x15, 1225, 128000 },
	{ 844800, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x16, 1250, 128000 },
	{ 883200, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x17, 1275, 128000 },
	{ 921600, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x18, 1300, 128000 },
	{ 960000, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x19, 1300, 128000 },
	{ 998400, CCTL(CLK_TCXO, 1),		SRC_SCPLL, 0x1A, 1300, 128000 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct clkctl_acpu_speed *acpu_stby = &acpu_freq_tbl[0];
struct clkctl_acpu_speed *acpu_mpll = &acpu_freq_tbl[0];
struct clkctl_acpu_speed *current_speed = NULL;

#define IS_ACPU_STANDBY(x)	(((x)->clk_cfg == acpu_stby->clk_cfg) && ((x)->clk_sel == acpu_stby->clk_sel))

#define PLLMODE_POWERDOWN	0
#define PLLMODE_BYPASS		1
#define PLLMODE_STANDBY		2
#define PLLMODE_FULL_CAL	4
#define PLLMODE_HALF_CAL	5
#define PLLMODE_STEP_CAL	6
#define PLLMODE_NORMAL		7
#define PLLMODE_MASK		7

static void scpll_power_down(void)
{
	UINT32 val;

	/* Wait for any frequency switches to finish. */
	while (readl(SCPLL_STATUS_ADDR) & 0x1);

	/* put the pll in standby mode */
	val = readl(SCPLL_CTL_ADDR);
	val = (val & (~PLLMODE_MASK)) | PLLMODE_STANDBY;
	writel(val, SCPLL_CTL_ADDR);
	dmb();

	/* wait to stabilize in standby mode */
	NanoSecondDelay(10);

	val = (val & (~PLLMODE_MASK)) | PLLMODE_POWERDOWN;
	writel(val, SCPLL_CTL_ADDR);
	dmb();
}

static void scpll_set_freq(UINT32 lval)
{
	UINT32 val, ctl;

	if (lval > 33)
		lval = 33;
	if (lval < 10)
		lval = 10;

	/* wait for any calibrations or frequency switches to finish */
	while (readl(SCPLL_STATUS_ADDR) & 0x3);

	ctl = readl(SCPLL_CTL_ADDR);

	if ((ctl & PLLMODE_MASK) != PLLMODE_NORMAL) {
		/* put the pll in standby mode */
		writel((ctl & (~PLLMODE_MASK)) | PLLMODE_STANDBY, SCPLL_CTL_ADDR);
		dmb();

		/* wait to stabilize in standby mode */
		NanoSecondDelay(10);

		/* switch to 384 MHz */
		val = readl(SCPLL_FSM_CTL_EXT_ADDR);
		val = (val & (~0x1FF)) | (0x0A << 3) | SHOT_SWITCH;
		writel(val, SCPLL_FSM_CTL_EXT_ADDR);
		dmb();

		ctl = readl(SCPLL_CTL_ADDR);
		writel(ctl | PLLMODE_NORMAL, SCPLL_CTL_ADDR);
		dmb();

		/* wait for frequency switch to finish */
		while (readl(SCPLL_STATUS_ADDR) & 0x1) ;

		/* completion bit is not reliable for SHOT switch */
		NanoSecondDelay(25);
	}

	/* write the new L val and switch mode */
	val = readl(SCPLL_FSM_CTL_EXT_ADDR);
	val = (val & (~0x1FF)) | (lval << 3) | HOP_SWITCH;
	writel(val, SCPLL_FSM_CTL_EXT_ADDR);
	dmb();

	ctl = readl(SCPLL_CTL_ADDR);
	writel(ctl | PLLMODE_NORMAL, SCPLL_CTL_ADDR);
	dmb();

	/* wait for frequency switch to finish */
	while (readl(SCPLL_STATUS_ADDR) & 0x1) ;
}

/* this is still a bit weird... */
static void select_clock(unsigned src, unsigned config)
{
	UINT32 val;

	if (src == SRC_RAW) {
		UINT32 sel = readl(SPSS_CLK_SEL_ADDR);
		unsigned shift = (sel & 1) ? 8 : 0;

		/* set other clock source to the new configuration */
		val = readl(SPSS_CLK_CNTL_ADDR);
		val = (val & (~(0x7F << shift))) | (config << shift);
		writel(val, SPSS_CLK_CNTL_ADDR);

		/* switch to other clock source */
		writel(sel ^ 1, SPSS_CLK_SEL_ADDR);

		dmb(); /* necessary? */
	}

	/* switch to new source */
	val = readl(SPSS_CLK_SEL_ADDR) & (~6);
	writel(val | ((src & 3) << 1), SPSS_CLK_SEL_ADDR);
}

int acpuclk_set_rate(unsigned long rate, enum setrate_reason reason)
{
	struct clkctl_acpu_speed *cur, *next;

	cur = current_speed;

	/* convert to KHz */
	rate /= 1000;

	dprintf(INFO, "[ACPU] switching to %d MHz\n", ((int) (rate/1000)));

	if (rate == cur->acpu_khz || rate == 0)
		return 0;

	next = acpu_freq_tbl;
	for (;;) {
		if (next->acpu_khz == rate)
			break;
		if (next->acpu_khz == 0)
			return -1;
		next++;
	}

	if (next->clk_sel == SRC_SCPLL) {
		/* curr -> standby(MPLL speed) -> target */
		if (!IS_ACPU_STANDBY(cur))
			select_clock(acpu_stby->clk_sel, acpu_stby->clk_cfg);
		scpll_set_freq(next->sc_l_value);
		select_clock(SRC_SCPLL, 0);
	} else {
		if (cur->clk_sel == SRC_SCPLL) {
			select_clock(acpu_stby->clk_sel, acpu_stby->clk_cfg);
			select_clock(next->clk_sel, next->clk_cfg);
			scpll_power_down();
		} else {
			select_clock(next->clk_sel, next->clk_cfg);
		}
	}

	current_speed = next;

/*	This will fail anyway, so remove it for now
	if (reason == SETRATE_CPUFREQ) {
		if (cur->axiclk_khz != next->axiclk_khz) 
			clk_set_rate(EBI1_CLK, next->axiclk_khz * 1000);
	}
*/

	return 0;
}

static unsigned acpuclk_find_speed(void)
{
	UINT32 sel, val;

	sel = readl(SPSS_CLK_SEL_ADDR);
	switch ((sel & 6) >> 1) {
	case 1:
		val = readl(SCPLL_FSM_CTL_EXT_ADDR);
		val = (val >> 3) & 0x3f;
		return val * 38400;
	case 2:
		return 128000;
	default:
		dprintf(CRITICAL, "acpu_find_speed: failed spinning forever!\n");
		for(;;);
		return 0;
	}
}

#define PCOM_MODEM_PLL	0

static void acpuclk_init(int freq_num)
{
	if (acpuclk_init_done)
		return;

	unsigned init_khz __UNUSED;
	init_khz = acpuclk_find_speed();
	/*
	 * request the modem pll, and then drop it. We don't want to keep a
	 * ref to it, but we do want to make sure that it is initialized at
	 * this point. The ARM9 will ensure that the MPLL is always on
	 * once it is fully booted, but it may not be up by the time we get
	 * to here. So, our pll_request for it will block until the mpll is
	 * actually up. We want it up because we will want to use it as a
	 * temporary step during frequency scaling.
	 */
	msm_pll_request(PCOM_MODEM_PLL, 1);
	msm_pll_request(PCOM_MODEM_PLL, 0);

	if (!(readl(MSM_CLK_CTL_BASE + 0x300) & 1)) {
		dprintf(CRITICAL, "%s: MPLL IS NOT ON!!! RUN AWAY!!\n", __func__);
		acpuclk_init_done = 1;
		for(;;); /* DO NOT RETURN */
	}

	/* 
	 * Move to 768MHz for boot, which is a safe frequency
	 * for all versions of Scorpion at the moment.
	 */
	current_speed = (struct clkctl_acpu_speed *) &acpu_freq_tbl[freq_num];

	/*
	 * Bootloader needs to have SCPLL operating, but we're
	 * going to step over to the standby clock and make sure
	 * we select the right frequency on SCPLL and then
	 * step back to it, to make sure we're sane here.
	 */
	select_clock(acpu_stby->clk_sel, acpu_stby->clk_cfg);
	scpll_power_down();
	scpll_set_freq(current_speed->sc_l_value);
	select_clock(SRC_SCPLL, 0);
	acpuclk_init_done = 1;
	
	return;
}

unsigned long acpuclk_get_rate(void)
{
	return (acpuclk_init_done == 1 ? current_speed->acpu_khz : 998001);
}

void msm_acpu_clock_init(int freq_num)
{
/*
	drv_state.acpu_switch_time_us = 20;
	drv_state.max_speed_delta_khz = 256000;
	drv_state.vdd_switch_time_us = 62;
	drv_state.power_collapse_khz = 245000;
	drv_state.wait_for_irq_khz = 245000;
*/
	acpuclk_init(freq_num);
	//clk_set_rate(EBI1_CLK, drv_state.current_speed->axiclk_khz * 1000);
}
