/*
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __QSD8K_PLATFORM_ACPUCLOCK_H_
#define __QSD8K_PLATFORM_ACPUCLOCK_H_


enum setrate_reason {
	SETRATE_CPUFREQ = 0,
	SETRATE_SWFI,
};

void clock_config(UINT32 ns, UINT32 md, UINT32 ns_addr, UINT32 md_addr);
int acpuclk_set_rate(unsigned long rate, enum setrate_reason reason);
unsigned long acpuclk_get_rate(void);
void msm_acpu_clock_init(int freq_num);
#endif //__QSD8K_PLATFORM_ACPUCLOCK_H_