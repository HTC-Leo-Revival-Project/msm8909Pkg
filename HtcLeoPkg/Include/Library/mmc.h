/*
 * mmc.h 
 * SD/MMC definitions
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
 *
 */

#ifndef __QC_MMC_H
#define __QC_MMC_H


#define BLOCK_SIZE         512
#define SDCC_FIFO_SIZE     64

// HCLK_FREQ is only used to set timeout values.  This is not critical,
// as higher HCLK_FREQ will simply create a larger timeout count.
#define HCLK_FREQ         20e6
#define WR_DATA_TIMEOUT   (int)(HCLK_FREQ * 250e-3)
#define RD_DATA_TIMEOUT   (int)(HCLK_FREQ * 100e-3)

#define CMD0    0x00    // GO_IDLE_STATE
#define CMD1    0x01    // SEND_OP_COND
#define CMD2    0x02    // ALL_SEND_CID
#define CMD3    0x03    // SET_RELATIVE_ADDR
#define CMD4    0x04    // SET_DSR
#define CMD6    0x06    // SWITCH_FUNC
#define CMD7    0x07    // SELECT/DESELECT_CARD
#define CMD8    0x08    // SEND_IF_COND
#define CMD9    0x09    // SEND_CSD
#define CMD10   0x0A    // SEND_CID
#define CMD11   0x0B    // READ_DAT_UNTIL_STOP
#define CMD12   0x0C    // STOP_TRANSMISSION
#define CMD13   0x0D    // SEND_STATUS
#define CMD15   0x0F    // GO_INACTIVE_STATE
#define CMD16   0x10    // SET_BLOCKLEN
#define CMD17   0x11    // READ_SINGLE_BLOCK
#define CMD18   0x12    // READ_MULTIPLE_BLOCK
#define CMD20   0x14    // WRITE_DAT_UNTIL_STOP
#define CMD24   0x18    // WRITE_BLOCK
#define CMD25   0x19    // WRITE_MULTIPLE_BLOCK
#define CMD26   0x1A    // PROGRAM_CID
#define CMD27   0x1B    // PROGRAM_CSD
#define CMD28   0x1C    // SET_WRITE_PROT
#define CMD29   0x1D    // CLR_WRITE_PROT
#define CMD30   0x1E    // SEND_WRITE_PROT
#define CMD32   0x20    // TAG_SECTOR_START
#define CMD33   0x21    // TAG_SECTOR_END
#define CMD34   0x22    // UNTAG_SECTOR
#define CMD35   0x23    // TAG_ERASE_GROUP_START
#define CMD36   0x24    // TAG_ERASE_GROUP_END
#define CMD37   0x25    // UNTAG_ERASE_GROUP
#define CMD38   0x26    // ERASE
#define CMD42   0x2A    // LOCK_UNLOCK
#define CMD55   0x37    // APP_CMD
#define CMD56   0x38    // GEN_CMD
#define ACMD6   0x06    // SET_BUS_WIDTH
#define ACMD13  0x0D    // SD_STATUS
#define ACMD22  0x16    // SET_NUM_WR_BLOCKS
#define ACMD23  0x17    // SET_WR_BLK_ERASE_COUNT
#define ACMD41  0x29    // SD_APP_OP_COND
#define ACMD42  0x2A    // SET_CLR_CARD_DETECT
#define ACMD51  0x33    // SEND_SCR
#define CMD5    0x05    // IO_SEND_OP_COND
#define CMD52   0x34    // IO_RW_DIRECT
#define CMD53   0x35    // IO_RW_EXTENDED

// SD controller register offsets
#define MCI_POWER               (0x000)
#define MCI_CLK                 (0x004)
#define MCI_ARGUMENT            (0x008)
#define MCI_CMD                 (0x00C)
#define MCI_RESP_CMD            (0x010)
#define MCI_RESPn(n)            (0x014+4*n)
#define MCI_RESP0               (0x014)
#define MCI_RESP1               (0x018)
#define MCI_RESP2               (0x01C)
#define MCI_RESP3               (0x020)
#define MCI_DATA_TIMER          (0x024)
#define MCI_DATA_LENGTH         (0x028)
#define MCI_DATA_CTL            (0x02C)
#define MCI_DATA_COUNT          (0x030)
#define MCI_STATUS              (0x034)
#define MCI_CLEAR               (0x038)
#define MCI_INT_MASKn(n)        (0x03C+4*n)
#define MCI_INT_MASK0           (0x03C)
#define MCI_INT_MASK1           (0x040)
#define MCI_FIFO_COUNT          (0x044)
#define MCI_CCS_TIMER           (0x058)
#define MCI_FIFO                (0x080)
#define MCI_TESTBUS_CONFIG      (0x0CC)
#define MCI_TEST_CTL            (0x0D0)
#define MCI_TEST_INPUT          (0x0D4)
#define MCI_TEST_OUT            (0x0D8)
#define MCI_PERPH_ID0           (0x0E0)
#define MCI_PERPH_ID1           (0x0E4)
#define MCI_PERPH_ID2           (0x0E8)
#define MCI_PERPH_ID3           (0x0EC)
#define MCI_PCELL_ID0           (0x0F0)
#define MCI_PCELL_ID1           (0x0F4)
#define MCI_PCELL_ID2           (0x0F8)
#define MCI_PCELL_ID3           (0x0FC)



// SD MCLK definitions

// For MCLK 400KHz derived from TCXO (19.2MHz) (M=1, N=48, D=24, P=1)
#define MCLK_MD_400KHZ    0x000100CF
#define MCLK_NS_400KHZ    0x00D00B40   
// For MCLK 25MHz derived from PLL1 768MHz (M=14, N=215, D=107.5, P=2), dual edge mode
#define MCLK_MD_25MHZ     0x000E0028
#define MCLK_NS_25MHZ     0x00360B49
// For MCLK 48MHz derived from PLL1 768MHz (M=1, N=8, D=4, P=2)
#define MCLK_MD_48MHZ     0x000100F7
#define MCLK_NS_48MHZ     0x00F80B49
// For MCLK 50MHz derived from PLL1 768MHz (M=25, N=192, D=96, P=2)
#define MCLK_MD_50MHZ     0x0019003F
#define MCLK_NS_50MHZ     0x00580B49



// ADM Command Pointer List Entry definitions
#define ADM_CMD_PTR_LP          0x80000000    // Last pointer
#define ADM_CMD_PTR_CMD_LIST    (0 << 29)     // Command List

// ADM Command List definitions (First Word)
#define ADM_CMD_LIST_LC         0x80000000    // Last command
#define ADM_ADDR_MODE_BOX       (3 << 0)      // Box address mode

#define ROWS_PER_BLOCK   (BLOCK_SIZE / SDCC_FIFO_SIZE)


#define	Byte_swap32(value)  ( ((value >>24) & 0x000000ff) |  \
                              ((value >> 8) & 0x0000ff00) |  \
			                  ((value << 8) & 0x00ff0000) |  \
			                  ((value <<24) & 0xff000000) )

// Structure to hold parms for a given SD controller
typedef struct sd_parms {
   uint32_t instance;                  // which instance of the SD controller
   uint32_t base;                      // SD controller base address
   uint32_t ns_addr;                   // Clock controller NS reg address
   uint32_t md_addr;                   // Clock controller MD reg address
   uint32_t ns_initial;                // Clock controller NS reg initial value
   uint32_t md_initial;                // Clock controller MD reg initial value
   uint32_t row_reset_mask;            // Bit in the ROW reset register
   uint32_t glbl_clk_ena_mask;         // Bit in the global clock enable
   uint32_t glbl_clk_ena_initial;      // Initial value of the global clock enable bit                                
   uint32_t adm_crci_num;              // ADM CRCI number
   uint32_t adm_ch8_rslt_conf_initial; // Initial value of HI0_CH8_RSLT_CONF_SD3                                  
} sd_parms_t;

#endif              /* __QC_MMC_H */


