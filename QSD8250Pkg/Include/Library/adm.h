/*
 *  adm.h - Application Data Mover (ADM) definitions
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

#ifndef __QC_ADM_H
#define __QC_ADM_H

#include <Chipset/iomap.h>

// ADM base address for channel (n) and security_domain (s)
#define ADM_BASE_ADDR(n, s) (MSM_ADM_BASE + 4*(n) + ((MSM_ADM_SD_OFFSET)*(s)))

// ADM registers
#define ADM_REG_CMD_PTR(n, s)   (ADM_BASE_ADDR(n, s) + 0x000)
#define ADM_REG_RSLT(n, s)      (ADM_BASE_ADDR(n, s) + 0x040)
#define ADM_REG_STATUS(n, s)    (ADM_BASE_ADDR(n, s) + 0x200)
#define ADM_REG_IRQ(s)          (ADM_BASE_ADDR(0, s) + 0x380)

// Result reg bit masks
#define ADM_REG_RSLT__V___M         (1 << 31)
#define ADM_REG_RSLT__ERR___M       (1 << 3)
#define ADM_REG_RSLT__TPD___M       (1 << 1)
#define ADM_REG_CH8_RSLT_CONF		(0xA9700F20)

// Status reg bit masks
#define ADM_REG_STATUS__RSLT_VLD___M    (1 << 1)

#define ADM_SD          	 1

// QSD8x50 specific ADM channels
#define ADM_AARM_NAND_CHN    7
#define ADM_AARM_SD_CHN      8

// QSD8x50 specific ADM CRCIs
#define ADM_CRCI_NAND_DATA     4
#define ADM_CRCI_NAND_CMD      5 
#define ADM_CRCI_SDC1          6
#define ADM_CRCI_SDC2          7
#define ADM_CRCI_SDC3         12
#define ADM_CRCI_SDC4         13

// ADM Command Pointer List Entry definitions
#define ADM_CMD_PTR_LP          0x80000000    // Last pointer
#define ADM_CMD_PTR_CMD_LIST    (0 << 29)     // Command List

// ADM Command List definitions (First Word)
#define ADM_CMD_LIST_LC         0x80000000    // Last command
#define ADM_CMD_LIST_OCU        0x00200000    // Other channel unblock
#define ADM_CMD_LIST_OCB        0x00100000    // Other channel block
#define ADM_CMD_LIST_TCB        0x00080000    // This channel block
#define ADM_ADDR_MODE_BOX       (3 << 0)      // Box address mode
#define ADM_ADDR_MODE_SI        (0 << 0)      // Single item address mode

// ADM Single item command list entry
typedef struct si_cmd_list {
    UINT32 first;
    UINT32 src_addr;
    UINT32 dst_addr;
    UINT32 len;
} si_cmd_list_t;

// ADM Box command list entry
typedef struct box_cmd_list {
    UINT32 first;
    UINT32 src_row_addr;
    UINT32 dst_row_addr;
    UINT16 src_row_len;
    UINT16 dst_row_len;
    UINT16 src_row_num;
    UINT16 dst_row_num;
    UINT16 src_row_off;
    UINT16 dst_row_off;
} box_cmd_list_t;

int adm_start_transfer(UINT32 adm_chn, UINT32 *cmd_ptr_list);

#endif /* __QC_ADM_H */