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

//if LK is loaded by wince spl add romhdr
.org 0x40
	.word 0x43454345
	.word (romhdr-_start)+0x80000000		// virtual address of romhdr
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

	/* we're relocated, jump to the right address */

_Entry:
	ldr		r5, UefiBase
	bx		r5

UefiBase:
	.word UEFI_BASE

UefiSize:
	.word UEFI_SIZE

.ltorg

.text
.align 4
_Payload: