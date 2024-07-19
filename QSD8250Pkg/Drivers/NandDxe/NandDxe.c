/*
 * Copyright (c) 2012, Shantanu Gupta <shans95g@gmail.com>
 * Copyright (c) 2024, Aljoshua Hell <aljoshua.hellg@gmail.com>
 * Based on the open source driver from HTC, Interrupts are not supported yet
 */
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/LKEnvLib.h>
#include <Chipset/timer.h>
#include "dmov.h"
#include "nand.h"
#include "ptable.h"
#include "flash.h"

// Cached copy of the i2c protocol
//HTCLEO_I2C_PROTOCOL *gI2C = NULL;
#define HTCLEO_FLASH_OFFSET	0x219
#define IS_PART_EMPTY(p) (p->name[0]==0)
#define NAND_CFG0_RAW 0xA80420C0
#define NAND_CFG1_RAW 0x5045D
#define NUM_PROTECTED_BLOCKS 0x212
#define CFG1_WIDE_FLASH (1U << 1)
#define paddr(n) ((unsigned) (n))

#define SRC_CRCI_NAND_CMD  CMD_SRC_CRCI(DMOV_NAND_CRCI_CMD)
#define DST_CRCI_NAND_CMD  CMD_DST_CRCI(DMOV_NAND_CRCI_CMD)
#define SRC_CRCI_NAND_DATA CMD_SRC_CRCI(DMOV_NAND_CRCI_DATA)
#define DST_CRCI_NAND_DATA CMD_DST_CRCI(DMOV_NAND_CRCI_DATA)

static void *flash_spare;
static void *flash_data;
static unsigned CFG0, CFG1;
static unsigned CFG0_M, CFG1_M;
static unsigned CFG0_A, CFG1_A;
struct ptable flash_ptable;
static struct flash_info flash_info;
static unsigned *flash_ptrlist;
static dmov_s *flash_cmdlist;
static unsigned flash_pagesize = 0;
unsigned nand_cfg0;
unsigned nand_cfg1;



typedef struct dmov_ch dmov_ch;
struct dmov_ch
{
	volatile unsigned cmd;
	volatile unsigned result;
	volatile unsigned status;
	volatile unsigned config;
};

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


// align data on a 512 boundary so will not be interrupted in nbh
static struct ptentry board_part_list[MAX_PTABLE_PARTS] __attribute__ ((aligned (512))) = {
		{
				.name = "PTABLE-MB", // PTABLE-BLK or PTABLE-MB for length in MB or BLOCKS
		},
		{
				.name = "misc",
				.length = 1 /* In MB */,
		},
		{
				.name = "recovery",
				.length = 5 /* In MB */,
		},
		{
				.name = "sboot",
				.length = 5 /* In MB */,
		},
		{
				.name = "boot",
				.length = 5 /* In MB */,
		},
		{
				.name = "system",
				.length = 20 /* In MB */,
		},
		{
				.length = 20 /* In MB */,
				.name = "cache",
		},
		{
				.name = "userdata",
		},
};
static unsigned num_parts = sizeof(board_part_list)/sizeof(struct ptentry);

struct flash_info *flash_get_info(void)
{
	return &flash_info;
}

static void dmov_prep_ch(dmov_ch *ch, unsigned id)
{
	ch->cmd = DMOV_CMD_PTR(id);
	ch->result = DMOV_RSLT(id);
	ch->status = DMOV_STATUS(id);
	ch->config = DMOV_CONFIG(id);
}

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
		}
		n = readl(ch.status);
	}

	return 0;
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

void flash_init(void)
{
	//ASSERT(flash_ptable == NULL);

	flash_ptrlist = AllocateAlignedPages(32,1024);//memalign(32, 1024);
	flash_cmdlist = AllocateAlignedPages(32,1024);//memalign(32, 1024);
	flash_data = AllocateAlignedPages(32,4096+128);//memalign(32, 4096 + 128);
	flash_spare = AllocateAlignedPages(32,128);//memalign(32, 128);

	flash_read_id(flash_cmdlist, flash_ptrlist);
	if((FLASH_8BIT_NAND_DEVICE == flash_info.type)
		||(FLASH_16BIT_NAND_DEVICE == flash_info.type)) {
		if(flash_nand_read_config(flash_cmdlist, flash_ptrlist)) {
			dprintf(CRITICAL, "ERROR: could not read CFG0/CFG1 state\n");
			ASSERT(0);
		}
	}
}

void htcleo_ptable_dump(struct ptable *ptable)
{
	struct ptentry *ptn;
	int i;

	for (i = 0; i < ptable->count; ++i) {
		ptn = &ptable->parts[i];
		dprintf(INFO, "ptn %d name='%s' start=%08x len=%08x end=%08x \n",i, ptn->name, ptn->start, ptn->length,  ptn->start+ptn->length);
	}
}

void flash_set_ptable(struct ptable *new_ptable)
{
	//ToDo: fix me
	// if(!flash_ptable && new_ptable){
	// 	dprintf(INFO, "flash_set_ptable INIT FAILED");
	// 	for(;;);
	// }
	
	//flash_ptable = new_ptable;
}

void ptable_add(struct ptable *ptable, char *name, unsigned start,
		unsigned length, unsigned flags, char type, char perm)
{
	//ToDo: fix me
	// struct ptentry *ptn;

	// ASSERT(ptable && ptable->count < MAX_PTABLE_PARTS);

	// ptn = &ptable->parts[ptable->count++];
	// strncpy(ptn->name, name, MAX_PTENTRY_NAME);
	// ptn->start = start;
	// ptn->length = length;
	// ptn->flags = flags;
	// ptn->type = type;
	// ptn->perm = perm;
}

void ptable_init(struct ptable *ptable)
{
	ASSERT(ptable);
if (ptable == NULL) {
	dprintf(INFO, "PTABLE INIT FAILED");
    for(;;);
  }

	SetMem(ptable,sizeof(struct ptable), 0);
}

EFI_STATUS
EFIAPI
NandDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;
	unsigned nand_num_blocks;
	unsigned start_block;
	struct flash_info* flash_info1;
	unsigned blocks_per_plen = 1; //blocks per partition length
	// EFI_HANDLE Handle = NULL;

	// Find the i2c protocol.  ASSERT if not found.
  	// Status = gBS->LocateProtocol (&gHtcLeoI2CProtocolGuid, NULL, (VOID **)&gI2C);
  	// ASSERT_EFI_ERROR (Status);


		// Status = gBS->InstallMultipleProtocolInterfaces(
		// &Handle, &gHtcLeoMicropProtocolGuid, &gHtcLeoMicropProtocol, NULL);
		// ASSERT_EFI_ERROR(Status);
	
	
	
	DEBUG((EFI_D_INFO, "flash init\n"));
	flash_init();
	flash_info1 = flash_get_info();
	//ASSERT(flash_info);
	//ASSERT(flash_info->num_blocks);
	nand_num_blocks = flash_info1->num_blocks;

	ptable_init(&flash_ptable);

	if( strcmp(board_part_list[0].name,"PTABLE-BLK")==0 )
		blocks_per_plen =1 ;
	else if( strcmp(board_part_list[0].name,"PTABLE-MB")==0 )
		blocks_per_plen = (1024*1024)/flash_info1->block_size;
	else
		//panic("Invalid partition table\n");

	start_block = HTCLEO_FLASH_OFFSET;
	for (unsigned i = 1; i < num_parts; i++) {
		struct ptentry *ptn = &board_part_list[i];
		if( IS_PART_EMPTY(ptn) )
			break;
		int len = ((ptn->length) * blocks_per_plen);

		if( ptn->start == 0 )
			ptn->start = start_block;
		else if( ptn->start < start_block)
			//panic("Partition %s start %x < %x\n", ptn->name, ptn->start, start_block);

		if(ptn->length == 0) {
			unsigned length_for_prt = 0;
			if( i<num_parts && !IS_PART_EMPTY((&board_part_list[i+1]))
					&& board_part_list[i+1].start!=0)
			{
				length_for_prt =  board_part_list[i+1].start - ptn->start;
			}
			else
			{
				for (unsigned j = i+1; j < num_parts; j++) {
						struct ptentry *temp_ptn = &board_part_list[j];
						if( IS_PART_EMPTY(temp_ptn) ) break;
						if( temp_ptn->length==0 ) //panic("partition %s and %s have variable length\n", ptn->name, temp_ptn->name);
						length_for_prt += ((temp_ptn->length) * blocks_per_plen);
				}
			}
			len = (nand_num_blocks - 1 - 186 - 4) - (ptn->start + length_for_prt);
			ASSERT(len >= 0);
		}

		start_block = ptn->start + len;
		ptable_add(&flash_ptable, ptn->name, ptn->start, len, ptn->flags,
				TYPE_APPS_PARTITION, PERM_WRITEABLE);
	}

	htcleo_ptable_dump(&flash_ptable);
	flash_set_ptable(&flash_ptable);

	return Status;
}

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
	
}