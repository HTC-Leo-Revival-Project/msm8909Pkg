//
//  Copyright (c) 2011 - 2020, Arm Limited. All rights reserved.<BR>
//
//  SPDX-License-Identifier: BSD-2-Clause-Patent
//
//

#include <AsmMacroIoLib.h>

#include <Chipset/ArmV7.h>

#include "PrePi.h"

ASM_FUNC(_ModuleEntryPoint)
  // Do early platform specific actions
  bl    ASM_PFX(ArmPlatformPeiBootAction)

  // Get ID of this CPU in multi-core system
  bl    ASM_PFX(ArmReadMpidr)
  // Keep a copy of the MpId register value
  mov   r8, r0

_SetSVCMode:
  // Enter SVC mode, Disable FIQ and IRQ
  mov     r1, #(CPSR_MODE_SVC | CPSR_IRQ | CPSR_FIQ)
  msr     CPSR_c, r1

// Check if we can install the stack at the top of the System Memory or if we need
// to install the stacks at the bottom of the Firmware Device (case the FD is located
// at the top of the DRAM)
_SystemMemoryEndInit:
  ADRL  (r1, mSystemMemoryEnd)
  ldrd  r2, r3, [r1]
  teq   r3, #0
  moveq r1, r2
  mvnne r1, #0

_SetupStackPosition:
  // r1 = SystemMemoryTop

  // Calculate Top of the Firmware Device
  MOV32 (r2, FixedPcdGet32(PcdFdBaseAddress))
  MOV32 (r3, FixedPcdGet32(PcdFdSize) - 1)
  add   r3, r3, r2      // r3 = FdTop = PcdFdBaseAddress + PcdFdSize

  // UEFI Memory Size (stacks are allocated in this region)
  MOV32 (r4, FixedPcdGet32(PcdSystemMemoryUefiRegionSize))

  //
  // Reserve the memory for the UEFI region (contain stacks on its top)
  //

  // Calculate how much space there is between the top of the Firmware and the Top of the System Memory
  subs  r0, r1, r3      // r0 = SystemMemoryTop - FdTop
  bmi   _SetupStack     // Jump if negative (FdTop > SystemMemoryTop). Case when the PrePi is in XIP memory outside of the DRAM
  cmp   r0, r4
  bge   _SetupStack

  // Case the top of stacks is the FdBaseAddress
  mov   r1, r2

_SetupStack:
  // r1 contains the top of the stack (and the UEFI Memory)

  // Because the 'push' instruction is equivalent to 'stmdb' (decrement before), we need to increment
  // one to the top of the stack. We check if incrementing one does not overflow (case of DRAM at the
  // top of the memory space)
  adds  r9, r1, #1
  bcs   _SetupOverflowStack

_SetupAlignedStack:
  mov   r1, r9
  b     _GetBaseUefiMemory

_SetupOverflowStack:
  // Case memory at the top of the address space. Ensure the top of the stack is EFI_PAGE_SIZE
  // aligned (4KB)
  MOV32 (r9, ~EFI_PAGE_MASK & 0xFFFFFFFF)
  and   r1, r1, r9

_GetBaseUefiMemory:
  // Calculate the Base of the UEFI Memory
  sub   r9, r1, r4

_GetStackBase:
  // r1 = The top of the Mpcore Stacks
  // Stack for the primary core = PrimaryCoreStack
  MOV32 (r2, FixedPcdGet32(PcdCPUCorePrimaryStackSize))
  sub   r10, r1, r2

  // Stack for the secondary core = Number of Cores - 1
  MOV32 (r1, (FixedPcdGet32(PcdCoreCount) - 1) * FixedPcdGet32(PcdCPUCoreSecondaryStackSize))
  sub   r10, r10, r1

  // r10 = The base of the MpCore Stacks (primary stack & secondary stacks)
  mov   r0, r10
  mov   r1, r8
  //ArmPlatformStackSet(StackBase, MpId, PrimaryStackSize, SecondaryStackSize)
  MOV32 (r2, FixedPcdGet32(PcdCPUCorePrimaryStackSize))
  MOV32 (r3, FixedPcdGet32(PcdCPUCoreSecondaryStackSize))
  bl    ASM_PFX(ArmPlatformStackSet)

  // Is it the Primary Core ?
  mov   r0, r8
  bl    ASM_PFX(ArmPlatformIsPrimaryCore)
  cmp   r0, #1
  bne   _PrepareArguments

_PrepareArguments:
  mov   r0, r8
  mov   r1, r9
  mov   r2, r10
  mov   r3, sp

  // Move sec startup address into a data register
  // Ensure we're jumping to FV version of the code (not boot remapped alias)
  ldr   r4, =ASM_PFX(CEntryPoint)

  // Jump to PrePiCore C code
  //    r0 = MpId
  //    r1 = UefiMemoryBase
  //    r2 = StacksBase
  blx   r4

_NeverReturn:
  b _NeverReturn

/* void ArchDisableCache(uint flags) */
ASM_FUNC(ArchDisableCache)
	stmfd	sp!, {r4-r11, lr}

	mov		r7, r0						// save flags

	mrs		r12, cpsr					// save the old interrupt state
	cpsid	iaf							// interrupts disabled

.Ldcache_disable:
	tst		r7, #DCACHE
	beq		.Licache_disable
	mrc     p15, 0, r0, c1, c0, 0		// cr1
	tst		r0, #(1<<2)					// is the dcache already disabled?
	beq		.Ldcache_already_disabled

	bic		r0, #(1<<2)
	mcr		p15, 0, r0, c1, c0, 0		// disable dcache

	// flush and invalidate the dcache
	// NOTE: trashes a bunch of registers, can't be spilling stuff to the stack
	bl		flush_invalidate_cache_v7

	b		.Ldcache_disable_L2

.Ldcache_already_disabled:
	// make sure all of the caches are invalidated
	// NOTE: trashes a bunch of registers, can't be spilling stuff to the stack
	bl		invalidate_cache_v7

.Ldcache_disable_L2:

	// disable the L2, if present
	mrc     p15, 0, r0, c1, c0, 1		// aux cr1
	bic		r0, #(1<<1)
	mcr		p15, 0, r0, c1, c0, 1		// disable L2 dcache

.Licache_disable:
	tst		r7, #ICACHE
	beq		.Ldone_disable

	mrc     p15, 0, r0, c1, c0, 0		// cr1
	bic		r0, #(1<<12)
	mcr		p15, 0, r0, c1, c0, 0		// disable icache

.Ldone_disable:
	// make sure the icache is always invalidated
	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0		// invalidate icache to PoU

	msr		cpsr, r12
	ldmfd	sp!, {r4-r11, pc}

/* void ArchEnableCache(uint flags) */
ASM_FUNC(ArchEnableCache)
	stmfd	sp!, {r4-r11, lr}

	mov		r7, r0						// save flags

	mrs		r12, cpsr					// save the old interrupt state
	cpsid	iaf							// interrupts disabled
	
.Ldcache_enable:
	tst		r7, #DCACHE
	beq		.Licache_enable
	mrc     p15, 0, r0, c1, c0, 0		// cr1
	tst		r0, #(1<<2)					// is the dcache already enabled?
	bne		.Licache_enable

	// invalidate L1 and L2
	// NOTE: trashes a bunch of registers, can't be spilling stuff to the stack
	bl		invalidate_cache_v7

	// enable the L2, if present
	mrc     p15, 0, r0, c1, c0, 1		// aux cr1
	orr		r0, #(1<<1)
	mcr		p15, 0, r0, c1, c0, 1		// enable L2 dcache

	mrc     p15, 0, r0, c1, c0, 0		// cr1
	orr		r0, #(1<<2)
	mcr		p15, 0, r0, c1, c0, 0		// enable dcache

.Licache_enable:
	tst		r7, #ICACHE
	beq		.Ldone_enable

	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0		// invalidate icache to PoU

	mrc     p15, 0, r0, c1, c0, 0		// cr1
	orr		r0, #(1<<12)
	mcr		p15, 0, r0, c1, c0, 0		// enable icache

.Ldone_enable:
	msr		cpsr, r12
	ldmfd	sp!, {r4-r11, pc}

// flush & invalidate cache routine, trashes r0-r6, r9-r11
flush_invalidate_cache_v7:
	/* from ARMv7 manual, B2-17 */
	MRC 	p15, 1, R0, c0, c0, 1 		// Read CLIDR 
	ANDS 	R3, R0, #0x7000000 
	MOV 	R3, R3, LSR #23 			// Cache level value (naturally aligned) 
	BEQ 	.Lfinished 
	MOV 	R10, #0 
.Loop1:
	ADD 	R2, R10, R10, LSR #1 		// Work out 3xcachelevel 
	MOV 	R1, R0, LSR R2 				// bottom 3 bits are the Cache type for this level 
	AND 	R1, R1, #7 					// get those 3 bits alone 
	CMP 	R1, #2 
	BLT 	.Lskip 						// no cache or only instruction cache at this level 
	MCR 	p15, 2, R10, c0, c0, 0 		// write the Cache Size selection register 
	isb						 			// ISB to sync the change to the CacheSizeID reg 
	MRC 	p15, 1, R1, c0, c0, 0 		// reads current Cache Size ID register 
	AND 	R2, R1, #0x7 				// extract the line length field 
	ADD 	R2, R2, #4 					// add 4 for the line length offset (log2 16 bytes) 
	LDR 	R4, =0x3FF 
	ANDS 	R4, R4, R1, LSR #3 			// R4 is the max number on the way size (right aligned) 
	CLZ 	R5, R4 						// R5 is the bit position of the way size increment 
	LDR 	R6, =0x00007FFF 
	ANDS 	R6, R6, R1, LSR #13 		// R6 is the max number of the index size (right aligned) 
.Loop2:
	MOV 	R9, R4 						// R9 working copy of the max way size (right aligned) 
.Loop3:
	ORR 	R11, R10, R9, LSL R5 		// factor in the way number and cache number into R11 
	ORR 	R11, R11, R6, LSL R2 		// factor in the index number 
	MCR 	p15, 0, R11, c7, c14, 2 	// clean & invalidate by set/way 
	SUBS 	R9, R9, #1 					// decrement the way number 
	BGE 	.Loop3 
	SUBS 	R6, R6, #1 					// decrement the index 
	BGE 	.Loop2 
.Lskip:
 	ADD 	R10, R10, #2 					// increment the cache number 
	CMP 	R3, R10 
	BGT 	.Loop1 

.Lfinished:
	mov		r10, #0
	mcr		p15, 2, r10, c0, c0, 0		// select cache level 0
	isb

	bx		lr

// invalidate cache routine, trashes r0-r6, r9-r11
invalidate_cache_v7:
	/* from ARMv7 manual, B2-17 */
	MRC 	p15, 1, R0, c0, c0, 1 		// Read CLIDR 
	ANDS 	R3, R0, #0x7000000 
	MOV 	R3, R3, LSR #23 			// Cache level value (naturally aligned) 
	BEQ 	.Lfinished_invalidate
	MOV 	R10, #0 
.Loop1_invalidate:
	ADD 	R2, R10, R10, LSR #1 		// Work out 3xcachelevel 
	MOV 	R1, R0, LSR R2 				// bottom 3 bits are the Cache type for this level 
	AND 	R1, R1, #7 					// get those 3 bits alone 
	CMP 	R1, #2 
	BLT 	.Lskip_invalidate 			// no cache or only instruction cache at this level 
	MCR 	p15, 2, R10, c0, c0, 0 		// write the Cache Size selection register 
	isb						 			// ISB to sync the change to the CacheSizeID reg 
	MRC 	p15, 1, R1, c0, c0, 0 		// reads current Cache Size ID register 
	AND 	R2, R1, #0x7 				// extract the line length field 
	ADD 	R2, R2, #4 					// add 4 for the line length offset (log2 16 bytes) 
	LDR 	R4, =0x3FF 
	ANDS 	R4, R4, R1, LSR #3 			// R4 is the max number on the way size (right aligned) 
	CLZ 	R5, R4 						// R5 is the bit position of the way size increment 
	LDR 	R6, =0x00007FFF 
	ANDS 	R6, R6, R1, LSR #13 		// R6 is the max number of the index size (right aligned) 
.Loop2_invalidate:
	MOV 	R9, R4 						// R9 working copy of the max way size (right aligned) 
.Loop3_invalidate:
	ORR 	R11, R10, R9, LSL R5 		// factor in the way number and cache number into R11 
	ORR 	R11, R11, R6, LSL R2 		// factor in the index number 
	MCR 	p15, 0, R11, c7, c6, 2 		// invalidate by set/way 
	SUBS 	R9, R9, #1 					// decrement the way number 
	BGE 	.Loop3_invalidate 
	SUBS 	R6, R6, #1 					// decrement the index 
	BGE 	.Loop2_invalidate 
.Lskip_invalidate:
 	ADD 	R10, R10, #2 				// increment the cache number 
	CMP 	R3, R10 
	BGT 	.Loop1_invalidate 

.Lfinished_invalidate:
	mov		r10, #0
	mcr		p15, 2, r10, c0, c0, 0		// select cache level 0
	isb

	bx		lr