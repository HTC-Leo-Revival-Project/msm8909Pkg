/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "asm.h"

#define DSB .byte 0x4f, 0xf0, 0x7f, 0xf5
#define ISB .byte 0x6f, 0xf0, 0x7f, 0xf5

.text
.globl _start
_start:
	b	reset
	b 	relocate
	b	arm_undefined
	b	arm_syscall
	b	arm_prefetch_abort
	b	arm_data_abort
	b	arm_reserved
	b	arm_irq
	b	arm_fiq

//if LK is loaded by wince spl add romhdr
.org 0x40
	.word 0x43454345
	.word (romhdr-_start)+0x80000000		// virtual address of romhdr
	//.word romhdr+0x96C00000		// virtual address of romhdr
	.word romhdr-_start				// file address of romhdr

.org 0x00000900
romhdr:
	.word 0x2000000				// dllfirst
	.word 0x2000000				// dlllast
	.word 0x80000000				// physfirst
	.word 0x80000000+0x00000d640 //(_end-_start)	 physlast
	.word 0						// nummods (no TOCentry after ROMHDR)
	.word 0x80000000+0x00000d640 // ulRAMStart
	.word 0x80000000+0x00000d640	// ulRAMFree
	.word 0x80000000+0x00100000	// ulRAMEnd
	.word 0						// ulCopyEntries
	.word 0						// ulCopyOffset
	.word 0						// ulProfileLen
	.word 0						// ulProfileOffset
	.word 0						// numfiles
	.word 0						// ulKernelFlags
	.word 0x80808080			// ulFSRamPercent
	.word 0						// ulDrivglobStart
	.word 0						// ulDrivglobLen
	.hword 0x1C2				// usCPUType
	.hword 0x2					// usMiscFlags
	.word 0						// pExtensions
	.word 0						// ulTrackingStart
	.word 0						// ulTrackingLen
.org 0x00001000


reset:
	/* do some cpu setup */
	mrc		p15, 0, r0, c1, c0, 0
		/* XXX this is currently for arm926, revist with armv6 cores */
		/* new thumb behavior, low exception vectors, i/d cache disable, mmu disabled */
	bic		r0, r0, #(1<<15| 1<<13 | 1<<12)
	bic		r0, r0, #(1<<2 | 1<<0)
		/* enable alignment faults */
	orr		r0, r0, #(1<<1)
	mcr		p15, 0, r0, c1, c0, 0

	
	b		.Lstack_setup
.ltorg

.global load_address
load_address:
	.word 0

.Lstack_setup:
	/* set up the stack for irq, fiq, abort, undefined, system/user, and lastly supervisor mode */
	mrs     r0, cpsr
	bic     r0, r0, #0x1f

	/* copy the initialized data segment out of rom if necessary */
	ldr		r0, =__data_start_rom
	ldr		r1, =__data_start
	ldr		r2, =__data_end

	cmp		r0, r1
	beq		.L__do_bss

.L__copy_loop:
	cmp		r1, r2
	ldrlt	r3, [r0], #4
	strlt	r3, [r1], #4
	blt		.L__copy_loop

.L__do_bss:
	/* clear out the bss */
	ldr		r0, =__bss_start
	ldr		r1, =_end
	mov		r2, #0
.L__bss_loop:
	cmp		r0, r1
	strlt	r2, [r0], #4
	blt		.L__bss_loop

	DSB
	ISB
.ltorg

.code 32
.text
_relocate:
	/* see if we need to relocate */
	adr			r0, _Payload 	//Load the payload adress into r0

	ldr			r1, UefiBase 	//Load the expected payload base into r1
	cmp			r0, r1			//Compare the adresses
	beq			_Entry			//Jump to entry if the adress is the same
	ldr 		r2, UefiSize

_CopyLoop:
	/* memcpy */
	ldr		r3, [r0], #4
	str		r3, [r1], #4
	subs	r2, r2, #4		//substract the size that got copied from fd size
	bgt		_CopyLoop		//branch if greater than zero

	//flashlight to see we get here
	ldr     r4, =0xa9000864        @ bank6_in (phys)
	ldr     r5, =0xa9000814        @ bank6_out (phys)
	orr     r6, r4, #0x200000      @ 22nd bit for flash
	str     r6, [r5, #0]           @ store in out (enables bright for 500ms, limited by hardware)

	/* we're relocated, jump to the right address */

_Entry:
	ldr		r5, UefiBase
	bx		r5

UefiBase:
	.word UEFI_BASE

UefiSize:
	.word UEFI_SIZE

.ltorg

FUNCTION(arm_undefined)
	stmfd 	sp!, { r0-r12, r14 }
	sub		sp, sp, #12
	mov		r0, sp
	mrs		r1, spsr
	stmia	r0, { r1, r13-r14 }^
	b		arm_undefined_handler
	b		.

FUNCTION(arm_syscall)
	stmfd 	sp!, { r0-r12, r14 }
	sub		sp, sp, #12
	mov		r0, sp
	mrs		r1, spsr
	stmia	r0, { r1, r13-r14 }^
	b		arm_syscall_handler
	b		.
	
FUNCTION(arm_prefetch_abort)
	stmfd 	sp!, { r0-r12, r14 }
	sub		sp, sp, #12
	mov		r0, sp
	mrs		r1, spsr
	stmia	r0, { r1, r13-r14 }^
	b		arm_prefetch_abort_handler
	b		.

FUNCTION(arm_data_abort)
	stmfd 	sp!, { r0-r12, r14 }
	sub		sp, sp, #12
	mov		r0, sp
	mrs		r1, spsr
	stmia	r0, { r1, r13-r14 }^
	b		arm_data_abort_handler
	b		.
	
FUNCTION(arm_reserved)
	b	.
	
FUNCTION(arm_irq)
	/* XXX only deals with interrupting supervisor mode */

	/* save r4-r6 and use as a temporary place to save while we switch into supervisor mode */
	stmia	r13, { r4-r6 }
	mov		r4, r13
	sub		r5, lr, #4
	mrs		r6, spsr

	/* move into supervisor mode. irq/fiq disabled */
	msr	cpsr_c, #(3<<6 | 0x13)

	/* save the return address */
	stmfd	sp!, { r5 }

	/* save C trashed regs, supervisor lr */
	stmfd	sp!, { r0-r3, r12, lr }

	/* save spsr */
	stmfd	sp!, { r6 }

	/* restore r4-r6 */
	ldmia	r4, { r4-r6 }

	/* increment the global critical section count */
	ldr     r1, =critical_section_count
	ldr     r0, [r1]
	add     r0, r0, #1
	str     r0, [r1]
	
	/* call into higher level code */
	mov	r0, sp /* iframe */
	bl	platform_irq

	/* reschedule if the handler returns nonzero */
	cmp     r0, #0
	blne    thread_preempt

	/* decrement the global critical section count */
	ldr     r1, =critical_section_count
	ldr     r0, [r1]
	sub     r0, r0, #1
	str     r0, [r1]

	/* restore spsr */
	ldmfd	sp!, { r0 }
	msr     spsr_cxsf, r0

	/* restore back to where we came from */
	ldmfd	sp!, { r0-r3, r12, lr, pc }^

.bss
.align 2
	.global irq_save_spot
irq_save_spot:
	.word	0	/* r4 */
	.word	0	/* r5 */
	.word	0	/* r6 */
	
.text
FUNCTION(arm_fiq)
	sub	lr, lr, #4
	stmfd	sp!, { r0-r3, r12, lr }

	bl	platform_fiq
	
	ldmfd	sp!, { r0-r3, r12, pc }^

.ltorg

.text
.align 4
_Payload: