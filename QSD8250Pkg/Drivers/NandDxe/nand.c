 /*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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
#include <Library/LKEnvLib.h>
#include "ptable.h"
#include "flash.h"
#include "nand.h"
#include "dmov.h"

#define VERBOSE 1
#define VERIFY_WRITE 0

#define NUM_PROTECTED_BLOCKS 0x212

static void *flash_spare;
static void *flash_data;

typedef struct dmov_ch dmov_ch;
struct dmov_ch
{
	volatile unsigned cmd;
	volatile unsigned result;
	volatile unsigned status;
	volatile unsigned config;
};

static void dmov_prep_ch(dmov_ch *ch, unsigned id)
{
	ch->cmd = DMOV_CMD_PTR(id);
	ch->result = DMOV_RSLT(id);
	ch->status = DMOV_STATUS(id);
	ch->config = DMOV_CONFIG(id);
}

#define SRC_CRCI_NAND_CMD  CMD_SRC_CRCI(DMOV_NAND_CRCI_CMD)
#define DST_CRCI_NAND_CMD  CMD_DST_CRCI(DMOV_NAND_CRCI_CMD)
#define SRC_CRCI_NAND_DATA CMD_SRC_CRCI(DMOV_NAND_CRCI_DATA)
#define DST_CRCI_NAND_DATA CMD_DST_CRCI(DMOV_NAND_CRCI_DATA)

#define NAND_CFG0_RAW 0xA80420C0
#define NAND_CFG1_RAW 0x5045D

static unsigned CFG0, CFG1;

#define CFG1_WIDE_FLASH (1U << 1)

#define paddr(n) ((unsigned) (n))

static int dmov_exec_cmdptr(unsigned id, unsigned *ptr)
{
	dmov_ch ch;
	unsigned n;

	dmov_prep_ch(&ch, id);

	writel(DMOV_CMD_PTR_LIST | DMOV_CMD_ADDR(paddr(ptr)), ch.cmd);

	while(!(readl(ch.status) & DMOV_STATUS_RSLT_VALID)) ;

	n = readl(ch.status);
	while(DMOV_STATUS_RSLT_COUNT(n)) {
		n = readl(ch.result);
		if(n != 0x80000002) {
			dprintf(CRITICAL, "ERROR: result: %x\n", n);
			dprintf(CRITICAL, "ERROR:  flush: %x %x %x %x\n",
				readl(DMOV_FLUSH0(DMOV_NAND_CHAN)),
				readl(DMOV_FLUSH1(DMOV_NAND_CHAN)),
				readl(DMOV_FLUSH2(DMOV_NAND_CHAN)),
				readl(DMOV_FLUSH3(DMOV_NAND_CHAN)));
			return -1;
		}
		n = readl(ch.status);
	}
	return 0;
}

static struct flash_info flash_info;
static unsigned flash_pagesize = 0;

struct flash_identification {
	unsigned flash_id;
	unsigned mask;
	unsigned density;
	unsigned widebus;
	unsigned pagesize;
	unsigned blksize;
	unsigned oobsize;
};

static struct flash_identification supported_flash[] =
{
	/* Flash ID	ID Mask Density(MB)  Wid Pgsz	Blksz	oobsz 	Manuf */
	{0x00000000, 0xFFFFFFFF,		 0, 0,	0,		 0,  0}, /*ONFI*/
	{0x1500aaec, 0xFF00FFFF, (256<<20), 0, 2048, (2048<<6), 64}, /*Sams*/
	{0x5500baec, 0xFF00FFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Sams*/
	{0x1500aa98, 0xFFFFFFFF, (256<<20), 0, 2048, (2048<<6), 64}, /*Tosh*/
	{0x5500ba98, 0xFFFFFFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Tosh*/
	{0xd580b12c, 0xFFFFFFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Micr*/
	{0x5590bc2c, 0xFFFFFFFF, (512<<20), 1, 2048, (2048<<6), 64}, /*Micr*/
	{0x1580aa2c, 0xFFFFFFFF, (256<<20), 0, 2048, (2048<<6), 64}, /*Micr*/
	{0x1590aa2c, 0xFFFFFFFF, (256<<20), 0, 2048, (2048<<6), 64}, /*Micr*/
	{0x1590ac2c, 0xFFFFFFFF, (512<<20), 0, 2048, (2048<<6), 64}, /*Micr*/
	{0x5580baad, 0xFFFFFFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Hynx*/
	{0x5510baad, 0xFFFFFFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Hynx*/
	{0x6600bcec, 0xFF00FFFF, (512<<20), 1, 4096, (4096<<6), 128}, /*Sams*/

	/*added from kernel nand */
	{0x0000aaec, 0x0000FFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Samsung 2Gbit*/
	{0x0000acec, 0x0000FFFF, (512<<20), 1, 2048, (2048<<6), 64}, /*Samsung 4Gbit*/
	{0x0000bcec, 0x0000FFFF, (512<<20), 1, 2048, (2048<<6), 64}, /*Samsung 4Gbit*/
	{0x6601b3ec, 0xFFFFFFFF, (1024<<20),1, 4096, (4096<<6), 128}, /*Samsung 8Gbit 4Kpage*/
	{0x0000b3ec, 0x0000FFFF, (1024<<20),1, 2048, (2048<<6), 64}, /*Samsung 8Gbit*/
	{0x0000ba2c, 0x0000FFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Micron 2Gbit*/
	{0x0000bc2c, 0x0000FFFF, (512<<20), 1, 2048, (2048<<6), 64}, /*Micron 4Gbit*/
	{0x0000b32c, 0x0000FFFF, (1024<<20),1, 2048, (2048<<6), 64}, /*Micron 8Gbit*/
	{0x0000baad, 0x0000FFFF, (256<<20), 1, 2048, (2048<<6), 64}, /*Hynix 2Gbit*/
	{0x0000bcad, 0x0000FFFF, (512<<20), 1, 2048, (2048<<6), 64}, /*Hynix 4Gbit*/
	{0x0000b3ad, 0x0000FFFF, (1024<<20),1, 2048, (2048<<6), 64}, /*Hynix 8Gbit*/

	/* Note: Width flag is 0 for 8 bit Flash and 1 for 16 bit flash	  */
	/* Note: The First row will be filled at runtime during ONFI probe	*/

};

static void set_nand_configuration(char type)
{
	ASSERT(type==TYPE_APPS_PARTITION);
}

static void flash_nand_read_id(dmov_s *cmdlist, unsigned *ptrlist)
{
	dmov_s *cmd = cmdlist;
	unsigned *ptr = ptrlist;
	unsigned *data = ptrlist + 4;

	data[0] = 0 | 4;
	data[1] = NAND_CMD_FETCH_ID;
	data[2] = 1;
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	data[6] = 0;
	data[7] = 0xAAD40000;  /* Default value for CFG0 for reading device id */

	/* Read NAND device id */
	cmd[0].cmd = 0 | CMD_OCB;
	cmd[0].src = paddr(&data[7]);
	cmd[0].dst = NAND_DEV0_CFG0;
	cmd[0].len = 4;

	cmd[1].cmd = 0;
	cmd[1].src = NAND_SFLASHC_BURST_CFG;
	cmd[1].dst = paddr(&data[5]);
	cmd[1].len = 4;

	cmd[2].cmd = 0;
	cmd[2].src = paddr(&data[6]);
	cmd[2].dst = NAND_SFLASHC_BURST_CFG;
	cmd[2].len = 4;

	cmd[3].cmd = 0;
	cmd[3].src = paddr(&data[0]);
	cmd[3].dst = NAND_FLASH_CHIP_SELECT;
	cmd[3].len = 4;

	cmd[4].cmd = DST_CRCI_NAND_CMD;
	cmd[4].src = paddr(&data[1]);
	cmd[4].dst = NAND_FLASH_CMD;
	cmd[4].len = 4;

	cmd[5].cmd = 0;
	cmd[5].src = paddr(&data[2]);
	cmd[5].dst = NAND_EXEC_CMD;
	cmd[5].len = 4;

	cmd[6].cmd = SRC_CRCI_NAND_DATA;
	cmd[6].src = NAND_FLASH_STATUS;
	cmd[6].dst = paddr(&data[3]);
	cmd[6].len = 4;

	cmd[7].cmd = 0;
	cmd[7].src = NAND_READ_ID;
	cmd[7].dst = paddr(&data[4]);
	cmd[7].len = 4;

	cmd[8].cmd = CMD_OCU | CMD_LC;
	cmd[8].src = paddr(&data[5]);
	cmd[8].dst = NAND_SFLASHC_BURST_CFG;
	cmd[8].len = 4;

	ptr[0] = (paddr(cmd) >> 3) | CMD_PTR_LP;

	dmov_exec_cmdptr(DMOV_NAND_CHAN, ptr);

#if VERBOSE
	dprintf(INFO, "status: %x\n", data[3]);
#endif

	flash_info.id = data[4];
	flash_info.vendor = data[4] & 0xff;
	flash_info.device = (data[4] >> 8) & 0xff;
	return;
}

static int flash_nand_block_isbad(dmov_s *cmdlist, unsigned *ptrlist,
								  unsigned page)
{
	dmov_s *cmd = cmdlist;
	unsigned *ptr = ptrlist;
	unsigned *data = ptrlist + 4;
	char buf[4];
	unsigned cwperpage;

	if (page < (NUM_PROTECTED_BLOCKS<<6))
	{
		//protected blocks are always good
		return 0;
	}

	cwperpage = (flash_pagesize >> 9);

	/* Check first page of this block */
	if(page & 63)
		page = page - (page & 63);

	/* Check bad block marker */
	data[0] = NAND_CMD_PAGE_READ;	/* command */

	/* addr0 */
	if (CFG1 & CFG1_WIDE_FLASH)
		data[1] = (page << 16) | ((528*(cwperpage-1)) >> 1);
	else
		data[1] = (page << 16) | (528*(cwperpage-1));

	data[2] = (page >> 16) & 0xff;						/* addr1	*/
	data[3] = 0 | 4;									/* chipsel	*/
	data[4] = NAND_CFG0_RAW & ~(7U << 6);				/* cfg0		*/
	data[5] = NAND_CFG1_RAW | (CFG1 & CFG1_WIDE_FLASH);	/* cfg1		*/
	data[6] = 1;
	data[7] = CLEAN_DATA_32;	/* flash status */
	data[8] = CLEAN_DATA_32;	/* buf status	*/

	cmd[0].cmd = DST_CRCI_NAND_CMD | CMD_OCB;
	cmd[0].src = paddr(&data[0]);
	cmd[0].dst = NAND_FLASH_CMD;
	cmd[0].len = 16;

	cmd[1].cmd = 0;
	cmd[1].src = paddr(&data[4]);
	cmd[1].dst = NAND_DEV0_CFG0;
	cmd[1].len = 8;

	cmd[2].cmd = 0;
	cmd[2].src = paddr(&data[6]);
	cmd[2].dst = NAND_EXEC_CMD;
	cmd[2].len = 4;

	cmd[3].cmd = SRC_CRCI_NAND_DATA;
	cmd[3].src = NAND_FLASH_STATUS;
	cmd[3].dst = paddr(&data[7]);
	cmd[3].len = 8;

	cmd[4].cmd = CMD_OCU | CMD_LC;
	cmd[4].src = NAND_FLASH_BUFFER + (flash_pagesize - (528*(cwperpage-1)));
	cmd[4].dst = paddr(&buf);
	cmd[4].len = 4;

	ptr[0] = (paddr(cmd) >> 3) | CMD_PTR_LP;

	dmov_exec_cmdptr(DMOV_NAND_CHAN, ptr);

#if VERBOSE
	dprintf(INFO, "status: %x\n", data[7]);
#endif

	/* we fail if there was an operation error, a mpu error, or the
	** erase success bit was not set.
	*/
	if(data[7] & 0x110) return -1;

	/* Check for bad block marker byte */
	if (CFG1 & CFG1_WIDE_FLASH) {
		if (buf[0] != 0xFF || buf[1] != 0xFF)
			return 1;
	} else {
		if (buf[0] != 0xFF)
			return 1;
	}

	return 0;
}

static int flash_nand_erase_block(dmov_s *cmdlist, unsigned *ptrlist,
								  unsigned page)
{
	dmov_s *cmd = cmdlist;
	unsigned *ptr = ptrlist;
	unsigned *data = ptrlist + 4;
	int isbad = 0;

	if (page < (NUM_PROTECTED_BLOCKS<<6))
	{
		dprintf(INFO, "protected block not errased: %x\n", (page>>6));
		return -1;
	}
	/* only allow erasing on block boundaries */
	if(page & 63) return -1;

	/* Check for bad block and erase only if block is not marked bad */
	isbad = flash_nand_block_isbad(cmdlist, ptrlist, page);

	if (isbad) {
		dprintf(INFO, "skipping @ %d (bad block)\n", page >> 6);
		return -1;
	}

	/* Erase block */
	data[0] = NAND_CMD_BLOCK_ERASE;
	data[1] = page;
	data[2] = 0;
	data[3] = 0 | 4;
	data[4] = 1;
	data[5] = 0xeeeeeeee;
	data[6] = CFG0 & (~(7 << 6));  /* CW_PER_PAGE = 0 */
	data[7] = CFG1;
	data[8] = 0x00000020;
	data[9] = 0x000000C0;

	cmd[0].cmd = DST_CRCI_NAND_CMD | CMD_OCB;
	cmd[0].src = paddr(&data[0]);
	cmd[0].dst = NAND_FLASH_CMD;
	cmd[0].len = 16;

	cmd[1].cmd = 0;
	cmd[1].src = paddr(&data[6]);
	cmd[1].dst = NAND_DEV0_CFG0;
	cmd[1].len = 8;

	cmd[2].cmd = 0;
	cmd[2].src = paddr(&data[4]);
	cmd[2].dst = NAND_EXEC_CMD;
	cmd[2].len = 4;

	cmd[3].cmd = SRC_CRCI_NAND_DATA;
	cmd[3].src = NAND_FLASH_STATUS;
	cmd[3].dst = paddr(&data[5]);
	cmd[3].len = 4;

	cmd[4].cmd = 0;
	cmd[4].src = paddr(&data[8]);
	cmd[4].dst = NAND_FLASH_STATUS;
	cmd[4].len = 4;

	cmd[5].cmd = CMD_OCU | CMD_LC;
	cmd[5].src = paddr(&data[9]);
	cmd[5].dst = NAND_READ_STATUS;
	cmd[5].len = 4;

	ptr[0] = (paddr(cmd) >> 3) | CMD_PTR_LP;

	dmov_exec_cmdptr(DMOV_NAND_CHAN, ptr);

#if VERBOSE
	dprintf(INFO, "status: %x\n", data[5]);
#endif

	/* we fail if there was an operation error, a mpu error, or the
	 ** erase success bit was not set.
	 */
	if(data[5] & 0x110) return -1;
	if(!(data[5] & 0x80)) return -1;

	return 0;
}

struct data_flash_io {
	unsigned cmd;
	unsigned addr0;
	unsigned addr1;
	unsigned chipsel;
	unsigned cfg0;
	unsigned cfg1;
	unsigned exec;
	unsigned ecc_cfg;
	unsigned ecc_cfg_save;
	unsigned clrfstatus;
	unsigned clrrstatus;
	struct {
		unsigned flash_status;
		unsigned buffer_status;
	} result[8];
};

static int _flash_nand_read_page(dmov_s *cmdlist, unsigned *ptrlist,
								 unsigned page, void *_addr, void *_spareaddr)
{
	dmov_s *cmd = cmdlist;
	unsigned *ptr = ptrlist;
	struct data_flash_io *data = (void*) (ptrlist + 4);
	unsigned addr = (unsigned) _addr;
	unsigned spareaddr = (unsigned) _spareaddr;
	unsigned n;
	int isbad = 0;
	unsigned cwperpage;
	unsigned cwdatasize;
	unsigned cwoobsize;
	cwperpage = (flash_pagesize >> 9);
	cwdatasize = flash_pagesize/cwperpage;
	cwoobsize = /*oobavail*/ 16 / cwperpage; //spare size - ecc size (64 - 4*10)

	/* Check for bad block and read only from a good block */
	isbad = flash_nand_block_isbad(cmdlist, ptrlist, page);
	if (isbad)
	{
		dprintf(INFO, "bad block %x:\n",page);
		return -2;
	}

	data->cmd = NAND_CMD_PAGE_READ_ALL;
	data->addr0 = page << 16;
	data->addr1 = (page >> 16) & 0xff;
	data->chipsel = 0 | 4; /* flash0 + undoc bit */

	/* GO bit for the EXEC register */
	data->exec = 1;

	data->cfg0 = (CFG0 & ~(7U << 6)) | ((cwperpage-1) << 6);
	data->cfg1 = CFG1;

	data->ecc_cfg = 0x1FF;

	/* save existing ecc config */
	cmd->cmd = CMD_OCB;
	cmd->src = NAND_EBI2_ECC_BUF_CFG;
	cmd->dst = paddr(&data->ecc_cfg_save);
	cmd->len = 4;
	cmd++;

	for(n = 0; n < cwperpage; n++) {
		/* write CMD / ADDR0 / ADDR1 / CHIPSEL regs in a burst */
		cmd->cmd = DST_CRCI_NAND_CMD;
		cmd->src = paddr(&data->cmd);
		cmd->dst = NAND_FLASH_CMD;
		cmd->len = ((n == 0) ? 16 : 4);
		cmd++;

		if (n == 0) {
			/* block on cmd ready, set configuration */
			cmd->cmd = 0;
			cmd->src = paddr(&data->cfg0);
			cmd->dst = NAND_DEV0_CFG0;
			cmd->len = 8;
			cmd++;

			/* set our ecc config */
			cmd->cmd = 0;
			cmd->src = paddr(&data->ecc_cfg);
			cmd->dst = NAND_EBI2_ECC_BUF_CFG;
			cmd->len = 4;
			cmd++;
		}
		/* kick the execute register */
		cmd->cmd = 0;
		cmd->src = paddr(&data->exec);
		cmd->dst = NAND_EXEC_CMD;
		cmd->len = 4;
		cmd++;

		/* block on data ready, then read the status register */
		cmd->cmd = SRC_CRCI_NAND_DATA;
		cmd->src = NAND_FLASH_STATUS;
		cmd->dst = paddr(&data->result[n]);
		cmd->len = 8;
		cmd++;

		/* read data block */
		cmd->cmd = 0;
		cmd->src = NAND_FLASH_BUFFER;
		cmd->dst = addr + n * cwdatasize;
		cmd->len = cwdatasize;
		cmd++;

		/* read extra data */
		cmd->cmd = 0;
		cmd->src = NAND_FLASH_BUFFER + cwdatasize + 10; // adter data and 10 bytes of ECC
		cmd->dst = spareaddr + n*cwoobsize;
		cmd->len = cwoobsize;
		cmd++;
	}

	/* restore saved ecc config */
	cmd->cmd = CMD_OCU | CMD_LC;
	cmd->src = paddr(&data->ecc_cfg_save);
	cmd->dst = NAND_EBI2_ECC_BUF_CFG;
	cmd->len = 4;

	ptr[0] = (paddr(cmdlist) >> 3) | CMD_PTR_LP;

	int result = dmov_exec_cmdptr(DMOV_NAND_CHAN, ptr);
	if(result!=0)
	{
		dprintf(INFO, "read page failed %x (block %x)\n",page, page>>6);
		return -1;
	}

#if VERBOSE
	dprintf(INFO, "read page %d: status: %x %x %x %x\n",
		page, data[5], data[6], data[7], data[8]);
	for(n = 0; n < 4; n++) {
		ptr = (unsigned*)(addr + 512 * n);
		dprintf(INFO, "data%d:	%x %x %x %x\n", n, ptr[0], ptr[1], ptr[2], ptr[3]);
		ptr = (unsigned*)(spareaddr + 16 * n);
		dprintf(INFO, "spare data%d	%x %x %x %x\n", n, ptr[0], ptr[1], ptr[2], ptr[3]);
	}
#endif

	/* if any of the writes failed (0x10), or there was a
	 ** protection violation (0x100), we lose
	 */
	for(n = 0; n < cwperpage; n++) {
			if (data->result[n].flash_status & 0x110) {
				return -1;
			}
		}

	return 0;
}

static int _flash_nand_write_page(dmov_s *cmdlist, unsigned *ptrlist, unsigned page,
								  const void *_addr, const void *_spareaddr, unsigned raw_mode)
{
	if (page < (NUM_PROTECTED_BLOCKS<<6))
	{
			dprintf(INFO, "write disabled block %x is protected\n", (page>>6));
			return -1;
	}

	dmov_s *cmd = cmdlist;
	unsigned *ptr = ptrlist;
	struct data_flash_io *data = (void*) (ptrlist + 4);
	unsigned addr = (unsigned) _addr;
	unsigned spareaddr = (unsigned) _spareaddr;
	unsigned n;
	unsigned cwperpage;
	unsigned cwdatasize;
	unsigned cwoobsize;
	cwperpage = (flash_pagesize >> 9);
	cwdatasize = flash_pagesize/cwperpage;
	cwoobsize = /*oobavail*/ 16 / cwperpage; //spare size - ecc size (64 - 4*10)

	data->cmd = NAND_CMD_PRG_PAGE_ALL;
	data->addr0 = page << 16;
	data->addr1 = (page >> 16) & 0xff;
	data->chipsel = 0 | 4; /* flash0 + undoc bit */
	data->clrfstatus = 0x00000020;
	data->clrrstatus = 0x000000C0;

	if (!raw_mode){
		data->cfg0 = CFG0;
		data->cfg1 = CFG1;
	}else{
		data->cfg0 = (NAND_CFG0_RAW & ~(7 << 6)) |((cwperpage-1) << 6);
		data->cfg1 = NAND_CFG1_RAW | (CFG1 & CFG1_WIDE_FLASH);
	}

	/* GO bit for the EXEC register */
	data->exec = 1;

	data->ecc_cfg = 0x1FF;

	/* save existing ecc config */
	cmd->cmd = CMD_OCB;
	cmd->src = NAND_EBI2_ECC_BUF_CFG;
	cmd->dst = paddr(&data->ecc_cfg_save);
	cmd->len = 4;
	cmd++;

	for(n = 0; n < cwperpage; n++) {
			/* write CMD / ADDR0 / ADDR1 / CHIPSEL regs in a burst */
		cmd->cmd = DST_CRCI_NAND_CMD;
		cmd->src = paddr(&data->cmd);
		cmd->dst = NAND_FLASH_CMD;
		cmd->len = ((n == 0) ? 16 : 4);
		cmd++;

		if (n == 0) {
			/*  set configuration */
			cmd->cmd = 0;
			cmd->src = paddr(&data->cfg0);
			cmd->dst = NAND_DEV0_CFG0;
			cmd->len = 8;
			cmd++;

			/* set our ecc config */
			cmd->cmd = 0;
			cmd->src = paddr(&data->ecc_cfg);
			cmd->dst = NAND_EBI2_ECC_BUF_CFG;
			cmd->len = 4;
			cmd++;
		}

		/* write data block */
		cmd->cmd = 0;
		cmd->dst = NAND_FLASH_BUFFER;
		if (!raw_mode){
			cmd->src = addr + n * cwdatasize;
			cmd->len = cwdatasize;
		}else{
			cmd->src = addr;
			cmd->len =  528;
		}
		cmd++;

		if ((!raw_mode)) {
			/* write extra data */
			cmd->cmd = 0;
			cmd->src = spareaddr+n*cwoobsize;
			cmd->dst = NAND_FLASH_BUFFER + cwdatasize;
			cmd->len = cwoobsize;
			cmd++;
		}

		/* kick the execute register */
		cmd->cmd = 0;
		cmd->src = paddr(&data->exec);
		cmd->dst = NAND_EXEC_CMD;
		cmd->len = 4;
		cmd++;

		/* block on data ready, then read the status register */
		cmd->cmd = SRC_CRCI_NAND_DATA;
		cmd->src = NAND_FLASH_STATUS;
		cmd->dst = paddr(&data->result[n]);
		cmd->len = 8;
		cmd++;

		cmd->cmd = 0;
		cmd->src = paddr(&data->clrfstatus);
		cmd->dst = NAND_FLASH_STATUS;
		cmd->len = 4;
		cmd++;

		cmd->cmd = 0;
		cmd->src = paddr(&data->clrrstatus);
		cmd->dst = NAND_READ_STATUS;
		cmd->len = 4;
		cmd++;
	}

	/* restore saved ecc config */
	cmd->cmd = CMD_OCU | CMD_LC;
	cmd->src = paddr(&data->ecc_cfg_save);
	cmd->dst = NAND_EBI2_ECC_BUF_CFG;
	cmd->len = 4;

	ptr[0] = (paddr(cmdlist) >> 3) | CMD_PTR_LP;

	dmov_exec_cmdptr(DMOV_NAND_CHAN, ptr);

#if VERBOSE
	dprintf(INFO, "write page %d: status: %x %x %x %x\n",
		page, data[5], data[6], data[7], data[8]);
#endif

	/* if any of the writes failed (0x10), or there was a
	 ** protection violation (0x100), or the program success
	 ** bit (0x80) is unset, we lose
	 */
	for(n = 0; n < cwperpage; n++) {
		if(data->result[n].flash_status & 0x110) return -1;
		if(!(data->result[n].flash_status & 0x80)) return -1;
	}

#if VERIFY_WRITE
	n = _flash_read_page(cmdlist, ptrlist, page, flash_data,
				 flash_data + 2048);
	if (n != 0)
		return -1;
	if (memcmp(flash_data, _addr, 2048) ||
		memcmp(flash_data + 2048, _spareaddr, 16)) {
		dprintf(CRITICAL, "verify error @ page %d\n", page);
		return -1;
	}
#endif
	return 0;
}


char empty_buf[528];
static int flash_nand_mark_badblock(dmov_s *cmdlist, unsigned *ptrlist, unsigned page)
{
	if (page < (NUM_PROTECTED_BLOCKS<<6))
	{
		dprintf(INFO, "protected block %x cannot be marked bad: \n", (page>>6));
			return -1;
	}
  memset(empty_buf,0,528);
  /* Going to first page of the block */
  if(page & 63)
	page = page - (page & 63);
  return _flash_nand_write_page(cmdlist, ptrlist, page, empty_buf, 0, 1);
}

unsigned nand_cfg0;
unsigned nand_cfg1;

static int flash_nand_read_config(dmov_s *cmdlist, unsigned *ptrlist)
{
	static unsigned CFG0_TMP, CFG1_TMP;
	cmdlist[0].cmd = CMD_OCB;
	cmdlist[0].src = NAND_DEV0_CFG0;
	cmdlist[0].dst = paddr(&CFG0_TMP);
	cmdlist[0].len = 4;

	cmdlist[1].cmd = CMD_OCU | CMD_LC;
	cmdlist[1].src = NAND_DEV0_CFG1;
	cmdlist[1].dst = paddr(&CFG1_TMP);
	cmdlist[1].len = 4;

	*ptrlist = (paddr(cmdlist) >> 3) | CMD_PTR_LP;

	dmov_exec_cmdptr(DMOV_NAND_CHAN, ptrlist);

	if((CFG0_TMP == 0) || (CFG1_TMP == 0)) {
		return -1;
	}

	CFG0 = CFG0_TMP;
	CFG1 = CFG1_TMP;
	if (flash_info.type == FLASH_16BIT_NAND_DEVICE) {
		nand_cfg1 |= CFG1_WIDE_FLASH;
	}
	dprintf(INFO, "nandcfg: %x %x (initial)\n", CFG0_TMP, CFG1_TMP);

	CFG0 = (((flash_pagesize >> 9) - 1) <<  6)  /* 4/8 cw/pg for 2/4k */
		|	(512 <<  9)  /* 516 user data bytes */
		|	(10 << 19)  /* 10 parity bytes */
		|	(4 << 23)  /* spare size */
		|	(5 << 27)  /* 5 address cycles */
		|	(1 << 30)  /* Do not read status before data */
		|	(1 << 31);  /* Send read cmd */

	CFG1 = CFG1
#if 0
		|	(7 <<  2)  /* 8 recovery cycles */
		|	(0 <<  5)  /* Allow CS deassertion */
		|	(2 << 17)  /* 6 cycle tWB/tRB */
#endif
	  	|	((flash_pagesize - (528 * ((flash_pagesize >> 9) - 1)) + 1) <<  6)	/* Bad block marker location */
		|	(nand_cfg1 & CFG1_WIDE_FLASH); /* preserve wide flash flag */
	CFG1 = CFG1
		&   ~(1 <<  0)  /* Enable ecc */
		&   ~(1 << 16); /* Bad block in user data area */
	dprintf(INFO, "nandcfg: %x %x (used)\n", CFG0, CFG1);

	return 0;
}

static int flash_mark_badblock(dmov_s *cmdlist, unsigned *ptrlist, unsigned page)
{
	return flash_nand_mark_badblock(cmdlist, ptrlist, page);
}

/* Wrapper functions */
static void flash_read_id(dmov_s *cmdlist, unsigned *ptrlist)
{
	int dev_found = 0;
	unsigned index;

	// Try to read id
	flash_nand_read_id(cmdlist, ptrlist);
	// Check if we support the device
	for (index=1;
		 index < (sizeof(supported_flash)/sizeof(struct flash_identification));
		 index++)
	{
		if ((flash_info.id & supported_flash[index].mask) ==
			(supported_flash[index].flash_id &
			(supported_flash[index].mask))) {
			dev_found = 1;
			break;
		}
	}

	if(dev_found) {
		if (supported_flash[index].widebus)
			flash_info.type = FLASH_16BIT_NAND_DEVICE;
		else
			flash_info.type = FLASH_8BIT_NAND_DEVICE;

		flash_info.page_size = supported_flash[index].pagesize;
		flash_pagesize = flash_info.page_size;
		flash_info.block_size = supported_flash[index].blksize;
		flash_info.spare_size = supported_flash[index].oobsize;
		if (flash_info.block_size && flash_info.page_size)
		{
			flash_info.num_blocks = supported_flash[index].density;
			flash_info.num_blocks /= (flash_info.block_size);
		}
		else
		{
			flash_info.num_blocks = 0;
		}
        ASSERT(flash_info.num_blocks);
		//return;
	}

	// Assume 8 bit nand device for backward compatability
	if (dev_found == 0) {
		dprintf(INFO, "Device not supported.  Assuming 8 bit NAND device\n");
		flash_info.type = FLASH_8BIT_NAND_DEVICE;
	}
	dprintf(INFO, "nandid: 0x%x maker=0x%02x device=0x%02x page_size=%d\n",
		flash_info.id, flash_info.vendor, flash_info.device,
		flash_info.page_size);
	dprintf(INFO, "		spare_size=%d block_size=%d num_blocks=%d\n",
		flash_info.spare_size, flash_info.block_size,
		flash_info.num_blocks);
}

static int flash_erase_block(dmov_s *cmdlist, unsigned *ptrlist, unsigned page)
{
	return flash_nand_erase_block(cmdlist, ptrlist, page);
}

static int _flash_read_page(dmov_s *cmdlist, unsigned *ptrlist,
							unsigned page, void *_addr, void *_spareaddr)
{
	return _flash_nand_read_page(cmdlist, ptrlist, page, _addr, _spareaddr);
}

static int _flash_block_isbad(dmov_s *cmdlist, unsigned *ptrlist, unsigned page)
{
	return flash_nand_block_isbad(cmdlist, ptrlist, page);
}

static int _flash_write_page(dmov_s *cmdlist, unsigned *ptrlist,
							 unsigned page, const void *_addr,
							 const void *_spareaddr)
{
	return _flash_nand_write_page(cmdlist, ptrlist, page, _addr, _spareaddr, 0);
}

static unsigned *flash_ptrlist;
static dmov_s *flash_cmdlist;

static struct ptable *flash_ptable = NULL;

void flash_init(void)
{
	ASSERT(flash_ptable == NULL);

	flash_ptrlist = memalign(32, 1024);
	flash_cmdlist = memalign(32, 1024);
	//flash_data = memalign(32, 4096 + 128);
    flash_data = memalign(32, 4096);
	flash_spare = memalign(32, 128);

	flash_read_id(flash_cmdlist, flash_ptrlist);
	if((FLASH_8BIT_NAND_DEVICE == flash_info.type)
		||(FLASH_16BIT_NAND_DEVICE == flash_info.type)) {
		if(flash_nand_read_config(flash_cmdlist, flash_ptrlist)) {
			dprintf(CRITICAL, "ERROR: could not read CFG0/CFG1 state\n");
			ASSERT(0);
		}
	}
}

struct ptable *flash_get_ptable(void)
{
	return flash_ptable;
}

void flash_set_ptable(struct ptable *new_ptable)
{
	ASSERT(flash_ptable == NULL && new_ptable != NULL);
	flash_ptable = new_ptable;
}

struct flash_info *flash_get_info(void)
{
	return &flash_info;
}

int flash_erase(struct ptentry *ptn)
{
	unsigned block = ptn->start;
	unsigned count = ptn->length;

	set_nand_configuration(ptn->type);
	while(count-- > 0) {
		if(flash_erase_block(flash_cmdlist, flash_ptrlist, block * 64)) {
			dprintf(INFO, "cannot erase @ %d (bad block?)\n", block);
		}
		block++;
	}
	return 0;
}

int flash_read_ext(struct ptentry *ptn, unsigned extra_per_page,
			unsigned offset, void *data, unsigned bytes)
{
	unsigned page = (ptn->start * 64) + (offset / flash_pagesize);
	unsigned lastpage = (ptn->start + ptn->length) * 64;
	unsigned count = (bytes + flash_pagesize - 1 + extra_per_page) / (flash_pagesize + extra_per_page);
	unsigned *spare = (unsigned*) flash_spare;
	unsigned errors = 0;
	unsigned char *image = data;
	unsigned current_block = (page - (page & 63)) >> 6;
	unsigned start_block = ptn->start;
	int result = 0;
	int isbad = 0;
	int start_block_count = 0;

	dprintf(INFO, "flash read: %s %x %x\n", ptn->name, offset, bytes);
	ASSERT(ptn->type == TYPE_APPS_PARTITION);
	set_nand_configuration(TYPE_APPS_PARTITION);

	if(offset & (flash_pagesize - 1))
		return -1;

// Adjust page offset based on number of bad blocks from start to current page
	if (start_block < current_block)
	{
		start_block_count = (current_block - start_block);
		while (start_block_count && (start_block < (ptn->start + ptn->length))) {
			isbad = _flash_block_isbad(flash_cmdlist, flash_ptrlist, start_block*64);
			if (isbad)
				page += 64;
			else
				start_block_count--;
			start_block++;
		}
	}

	while((page < lastpage) && !start_block_count) {
		if(count == 0) {
			dprintf(INFO, "flash_read_image: success (%d errors)\n", errors);
			return 0;
		}

		result = _flash_read_page(flash_cmdlist, flash_ptrlist, page, image, spare);

		if (result == -1) {
			// bad page, go to next page
			page++;
			errors++;
			continue;
		}
		else if (result == -2) {
			// bad block, go to next block same offset
			page += 64;
			errors++;
			continue;
		}

		page++;
		image += flash_pagesize;
		memcpy(image, spare, extra_per_page);
		image += extra_per_page;
		count -= 1;
	}

	/* could not find enough valid pages before we hit the end */
	dprintf(INFO, "flash_read_image: failed (%d errors)\n", errors);
	return 0xffffffff;
}

int flash_write(struct ptentry *ptn, unsigned extra_per_page, const void *data,
		unsigned bytes)
{
	unsigned page = ptn->start * 64;
	unsigned lastpage = (ptn->start + ptn->length) * 64;
	unsigned *spare = (unsigned*) flash_spare;
	const unsigned char *image = data;
	unsigned wsize = flash_pagesize + extra_per_page;
	unsigned n;
	int r;

	if (ptn->type == TYPE_MODEM_PARTITION)
	{
		dprintf(CRITICAL, "flash_write_image: model partition not supported\n");
		return -1;
	}

	set_nand_configuration(ptn->type);
	for(n = 0; n < 16; n++) spare[n] = 0xffffffff;

	while(bytes > 0) {
		if(bytes < wsize) {
			dprintf(CRITICAL, "flash_write_image: image undersized (%d < %d)\n", bytes, wsize);
			return -1;
		}
		if(page >= lastpage) {
			dprintf(CRITICAL, "flash_write_image: out of space\n");
			return -1;
		}

		if((page & 63) == 0) {
			if(flash_erase_block(flash_cmdlist, flash_ptrlist, page)) {
				dprintf(INFO, "flash_write_image: bad block @ %d\n", page >> 6);
				page += 64;
				continue;
			}
		}

		if(extra_per_page) {
			r = _flash_write_page(flash_cmdlist, flash_ptrlist, page, image, image + flash_pagesize);
		} else {
			r = _flash_write_page(flash_cmdlist, flash_ptrlist, page, image, spare);
		}
		if(r) {
			dprintf(INFO, "flash_write_image: write failure @ page %d (src %d)\n", page, image - (const unsigned char *)data);
			image -= (page & 63) * wsize;
			bytes += (page & 63) * wsize;
			page &= ~63;
			if(flash_erase_block(flash_cmdlist, flash_ptrlist, page)) {
				dprintf(INFO, "flash_write_image: erase failure @ page %d\n", page);
			}
			if (ptn->type != TYPE_MODEM_PARTITION) {
				flash_mark_badblock(flash_cmdlist, flash_ptrlist, page);
			}
			dprintf(INFO, "flash_write_image: restart write @ page %d (src %d)\n", page, image - (const unsigned char *)data);
			page += 64;
			continue;
		}
		page++;
		image += wsize;
		bytes -= wsize;
	}

	/* erase any remaining pages in the partition */
	page = (page + 63) & (~63);
	while(page < lastpage){
		if(flash_erase_block(flash_cmdlist, flash_ptrlist, page)) {
			dprintf(INFO, "flash_write_image: bad block @ %d\n", page >> 6);
		}
		page += 64;
	}

	dprintf(INFO, "flash_write_image: success\n");
	return 0;
}

#if 0
static int flash_read_page(unsigned page, void *data, void *extra)
{
	return _flash_read_page(flash_cmdlist, flash_ptrlist,
				page, data, extra);
}
#endif

unsigned flash_page_size(void)
{
	return flash_pagesize;
}