/*
 *  adm.c - Application Data Mover (ADM) functions
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 * 
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a 
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 * 
 * START
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * END
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// must come in order
#include <Library/LKEnvLib.h>
#include <Library/UbootEnvLib.h>
#include <Library/adm.h>
#include <Chipset/reg.h>


int adm_start_transfer(uint32_t adm_chn, uint32_t *cmd_ptr_list)
{
    uint32_t timeout = 1;
    uint32_t adm_status;
    uint32_t adm_results;
    uint32_t adm_addr_shift;
    uint32_t delay_count = 100;


    // Memory barrier to insure that all ADM command list structure writes have
    // completed before starting the ADM transfer.
    DSB;    

    // Start the ADM transfer
    adm_addr_shift = (uint32_t)cmd_ptr_list >> 3;
    IO_WRITE32(HI0_CHn_CMD_PTR_SD3(adm_chn), adm_addr_shift);

	do
   {
		adm_status = IO_READ32(HI0_CHn_STATUS_SD3(adm_chn));
       if ((adm_status & HI0_CHn_STATUS_SD3__RSLT_VLD___M) != 0)
		{
			timeout = 0;
			break;
		}
		/* 10ms wait */
		mdelay(10);
    }while(delay_count--);

    // Get the result from the RSLT FIFO
    if (timeout == 0)
    {
       adm_results = IO_READ32(HI0_CHn_RSLT_SD3(adm_chn));
       if ( ((adm_results & HI0_CHn_RSLT_SD3__ERR___M) != 0)  ||
            ((adm_results & HI0_CHn_RSLT_SD3__TPD___M) == 0)  ||
            ((adm_results & HI0_CHn_RSLT_SD3__V___M) == 0) )
          return(-1);
    }

    return(0);
}
