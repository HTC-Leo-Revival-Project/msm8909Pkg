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

.text
.globl _start
_start:
	b	reset
    
reset:
	/* do some cpu setup */
	mrc		p15, 0, r0, c1, c0, 0
	bic		r0, r0, #(1<<15| 1<<13 | 1<<12)
	bic		r0, r0, #(1<<2 | 1<<0)
	orr		r0, r0, #(1<<1)
	mcr		p15, 0, r0, c1, c0, 0

	// Martijn Stolk's code so kernel will not crash. aux control register
	MRC p15, 0, r0, c1, c0, 1
	BIC r0, r0, #0x40
	BIC r0, r0, #0x200000
	MCR p15, 0, r0, c1, c0, 1

	// Disable VFP
	MOV R0, #0
	//FMXR FPEXC, r0

        // disable mmu
	MRC p15, 0, r0, c1, c0, 0
	BIC r0, r0, #(1<<0)
	MCR p15, 0, r0, c1, c0, 0
	ISB
	
	// Invalidate the UTLB
	MOV r0, #0
	MCR p15, 0, r0, c8, c7, 0

	// Clean and invalidate cache - Ensure pipeline flush
	MOV R0, #0
	DSB
	ISB

	// From PrimeG2Pkg
	// Disable Strict alignment checking & Enable Instruction cache
  	bic     r0, r0, #0x00002300     /* clear bits 13, 9:8 (--V- --RS) */
  	bic     r0, r0, #0x00000005     /* clear bits 0, 2 (---- -C-M) */
  	bic     r0, r0, #0x00000002     /* Clear bit 1 (Alignment faults) */
  	orr     r0, r0, #0x00001000     /* set bit 12 (I) enable I-Cache */
	mcr		p15, 0, r0, c1, c0, 0
.text
.align 4
_Payload: