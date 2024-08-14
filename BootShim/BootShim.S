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

#define DSB .byte 0x4f, 0xf0, 0x7f, 0xf5
#define ISB .byte 0x6f, 0xf0, 0x7f, 0xf5
#define WSPL_VADDR 0x80000000

.text
.globl _start
_start:
	b _Reset

//if EDK2 is loaded by wince spl add romhdr
.org 0x40
	.word 0x43454345
	.word (romhdr-_start)+WSPL_VADDR  // virtual address of romhdr
	.word romhdr-_start               // file address of romhdr

.org 0x00000900
romhdr:
	.word 0x2000000             // dllfirst
	.word 0x2000000             // dlllast
	.word WSPL_VADDR            // physfirst
	.word WSPL_VADDR+0x000F0000 // physlast (using hardoded FV size, fix later?)
	.word 0                     // nummods (no TOCentry after ROMHDR)
	.word WSPL_VADDR+0x000F0000 // ulRAMStart
	.word WSPL_VADDR+0x000F0000 // ulRAMFree
	.word WSPL_VADDR+0x001F0000 // ulRAMEnd
	.word 0                     // ulCopyEntries
	.word 0                     // ulCopyOffset
	.word 0                     // ulProfileLen
	.word 0                     // ulProfileOffset
	.word 0                     // numfiles
	.word 0                     // ulKernelFlags
	.word 0x80808080            // ulFSRamPercent
	.word 0                     // ulDrivglobStart
	.word 0                     // ulDrivglobLen
	.hword 0x1C2                // usCPUType
	.hword 0x2                  // usMiscFlags
	.word 0                     // pExtensions
	.word 0                     // ulTrackingStart
	.word 0                     // ulTrackingLen
.org 0x00001000

_Reset:
	/* do some cpu setup */
	mrc		p15, 0, r0, c1, c0, 0
	bic		r0, r0, #(1<<15| 1<<13 | 1<<12)
	bic		r0, r0, #(1<<2 | 1<<0)
	orr		r0, r0, #(1<<1)
	mcr		p15, 0, r0, c1, c0, 0

    LDR r0, =0xA3F90008      // Load the address where the start address is stored
    LDR r0, [r0]             // Load the actual start address from memory into r0
    ADD r1, r0, #0x300000    // Calculate the end address (start address + 0x300000)
    MOV r2, #0               // Set r2 to 0 (the value to store)

loop:
    STR r2, [r0], #4         // Store 0 at the address in r0 and increment r0 by 4
    CMP r0, r1               // Compare current address (r0) with end address (r1)
    BNE loop                 // If r0 != r1, branch back to loop

_UefiSetup:
	// From PrimeG2Pkg
	// Disable Strict alignment checking & Enable Instruction cache
  	bic     r0, r0, #0x00002300     /* clear bits 13, 9:8 (--V- --RS) */
  	bic     r0, r0, #0x00000005     /* clear bits 0, 2 (---- -C-M) */
  	bic     r0, r0, #0x00000002     /* Clear bit 1 (Alignment faults) */
  	orr     r0, r0, #0x00001000     /* set bit 12 (I) enable I-Cache */
  	mcr     p15, 0, r0, c1, c0, 0

    LDR r0, =0xA3F90008      // Load the address where the start address is stored
    LDR r0, [r0]             // Load the actual start address from memory into r0
    ADD r1, r0, #0x300000    // Calculate the end address (start address + 0x300000)
    MOV r2, #0xA               // Set r2 to 0 (the value to store)

loop1:
    STR r2, [r0], #4         // Store 0 at the address in r0 and increment r0 by 4
    CMP r0, r1               // Compare current address (r0) with end address (r1)
    BNE loop1                 // If r0 != r1, branch back to loop


_Relocate:
	/* see if we need to relocate */
	adr	r0, _Payload 	// load the payload adress into r0

	ldr	r1, UefiBase 	// load the expected payload base into r1
	cmp	r0, r1		 	// compare the adresses
	beq	_Entry			// jump to entry if the adress is the same
	ldr	r2, UefiSize

_CopyLoop:
	/* memcpy */
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	subs	r2, r2, #4	// substract the size that got copied from fd size
	bgt	_CopyLoop		// branch if greater than zero

	/* we're relocated, jump to the right address */

    LDR r0, =0xA3F90008      // Load the address where the start address is stored
    LDR r0, [r0]             // Load the actual start address from memory into r0
    ADD r1, r0, #0x300000    // Calculate the end address (start address + 0x300000)
    MOV r2, #0x0               // Set r2 to 0 (the value to store)

loop2:
    STR r2, [r0], #4         // Store 0 at the address in r0 and increment r0 by 4
    CMP r0, r1               // Compare current address (r0) with end address (r1)
    BNE loop2                 // If r0 != r1, branch back to loop

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