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
#include <Library/FrameBufferConfigLib.h>
#include <Chipset/timer.h>
#include "dmov.h"
#include "nand.h"
#include "ptable.h"
#include "flash.h"

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
	
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
    for(;;){};
  }

	SetMem(ptable,sizeof(struct ptable), 0);
}

#if 0
int
strcmp1(char const *cs, char const *ct)
{
	signed char __res;

	while(1) {
		if((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}
#endif

EFI_STATUS
EFIAPI
NandDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;

	struct flash_info* flash_info;
	unsigned start_block;
	unsigned blocks_per_plen = 1; //blocks per partition length
	unsigned nand_num_blocks;

	// Clear screen for debug
	PaintScreen(0);

	DEBUG((EFI_D_ERROR, "ptable1 init\n"));
	flash_init();
	flash_info = flash_get_info();
	ASSERT(flash_info);
	ASSERT(flash_info->num_blocks);
	nand_num_blocks = flash_info->num_blocks;
#if 0
	if (flash_info == NULL){
		DEBUG((EFI_D_ERROR, "flash_get_info failed\n"));
		for(;;){};
	}
#endif
    for(;;){};
#if 0
	ptable_init(&flash_ptable);

		blocks_per_plen = (1024*1024)/(2048<<6); //block_size needs to be dumped and checked

	start_block = HTCLEO_FLASH_OFFSET;
	for (unsigned i = 1; i < num_parts; i++) {
		struct ptentry *ptn = &board_part_list[i];
		if( IS_PART_EMPTY(ptn) )
			break;
		int len = ((ptn->length) * blocks_per_plen);

		if( ptn->start == 0 )
			ptn->start = start_block;
		else if( ptn->start < start_block)
			DEBUG((EFI_D_ERROR, "NandDxe: Platform Panic\n"));
			DEBUG((EFI_D_ERROR, "Partition %s start %x < %x\n", *ptn->name, ptn->start, start_block));
			for(;;){};

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
	//flash_set_ptable(&flash_ptable);
#endif
	return Status;
}