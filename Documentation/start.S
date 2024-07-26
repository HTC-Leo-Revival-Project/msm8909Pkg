/*
 *  armboot - Startup Code for Qualcomm Scorpion ARMv7 CPU
 *
 *  Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <config.h>
#include <version.h>
#include <asm-armv7Scorpion/armv7Scorpion.h>


/*
 *************************************************************************
 *
 * Vector table
 *
 *************************************************************************
 */


.globl _start
_start:
	b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:
	.word undefined_instruction
_software_interrupt:
	.word software_interrupt
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_not_used:
	.word not_used
_irq:
	.word irq
_fiq:
	.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif


/*
 * the actual reset code
 */

reset:
   
       /* Zero out R0 for use below */
        MOV     R0,  #0x0

       /* DCIALL to invalidate L2 cache banks (needs to be run 4 times, once per bank)
          Doing this as early as possible to hide the latency in the L2 */
        MOV    r1, #0x2
        WCP15_DCIALL(r1)                         /* bank D ([15:14] == 2'b00) */
        ORR    r1, r1, #0x00004000
        WCP15_DCIALL(r1)                         /* bank C ([15:14] == 2'b01) */
        ADD    r1, r1, #0x00004000
        WCP15_DCIALL(r1)                         /* bank B ([15:14] == 2'b10) */
        ADD    r1, r1, #0x00004000
        WCP15_DCIALL(r1)                         /* bank A ([15:14] == 2'b11) */

        /* Initialize the BPCR - setup Global History Mask (GHRM) to all 1's */
        LDR       r1, = 0x01FF01FF
        WCP15_BPCR(r1)

        /* Initialize I-cache victim and floor registers. */
         
	WCP15_ICVIC0(r0)
	WCP15_ICVIC1(r0)
	WCP15_ICVIC2(r0)
	WCP15_ICVIC3(r0)
	WCP15_ICVIC4(r0)
	WCP15_ICVIC5(r0)
	WCP15_ICVIC6(r0)
	WCP15_ICVIC7(r0)

	WCP15_ICFLOOR0(r0)
	WCP15_ICFLOOR1(r0)
	WCP15_ICFLOOR2(r0)
	WCP15_ICFLOOR3(r0)
	WCP15_ICFLOOR4(r0)
	WCP15_ICFLOOR5(r0)
	WCP15_ICFLOOR6(r0)
	WCP15_ICFLOOR7(r0)
         
        /* Initialize D-cache victim and floor registers. */
         
	WCP15_DCVIC0(r0)
	WCP15_DCVIC1(r0)
	WCP15_DCVIC2(r0)
	WCP15_DCVIC3(r0)
	WCP15_DCVIC4(r0)
	WCP15_DCVIC5(r0)
	WCP15_DCVIC6(r0)
	WCP15_DCVIC7(r0)
         
	WCP15_DCFLOOR0(r0)
	WCP15_DCFLOOR1(r0)
	WCP15_DCFLOOR2(r0)
	WCP15_DCFLOOR3(r0)
	WCP15_DCFLOOR4(r0)
	WCP15_DCFLOOR5(r0)
	WCP15_DCFLOOR6(r0)
	WCP15_DCFLOOR7(r0)
 
        /* Initialize ASID to zero */
        WCP15_CONTEXTIDR (r0)

	/* Invalidate both L1 I-cache and the branch predictor array. */
	WCP15_ICIALLU(r0)

        /* Invalidate the D-cache */                 
        WCP15_DCIALL(r0)
        
        WCP15_VBAR(r0)
         
        DSB
        ISB

        /* Setup DACR (Domain Access Control Register) to zero*/
        WCP15_DACR(r0)

        /* Setup DCLKCR and ICLKCR to allow normal D-Cache line fills */ 
        WCP15_DCLKCR(r0)
        WCP15_ICLKCR(r0)


        /* Setup the TLBLKCR 
           Victim = 6'b000000; Floor = 6'b000000;
           IASIDCFG = 2'b00 (State-Machine); IALLCFG = 2'b01 (Flash); BNA = 1'b0 */
        MOV    r1, #0x02
        WCP15_TLBLKCR(r1)

        /* Make sure TLBLKCR is complete before continuing */
        ISB

        /* Invalidate the UTLB */
        WCP15_UTLBIALL(r0)
        DSB
        ISB

        /* Setup L2CR1 to some default Instruction and data prefetching values
           Users may want specific settings for various performance enhancements */
        WCP15_L2CR1(r0)

        /* Enable Z bit to enable branch prediction (default is off) */
        RCP15_SCTLR(r2)
        ORR    r2, r2, #0x00000800
        WCP15_SCTLR(r2)

        /* Make sure Link stack is initialized with branch and links to sequential addresses
           This aids in creating a predictable startup environment */
        BL      SEQ1
SEQ1:   BL      SEQ2
SEQ2:   BL      SEQ3
SEQ3:   BL      SEQ4
SEQ4:   BL      SEQ5
SEQ5:   BL      SEQ6
SEQ6:   BL      SEQ7
SEQ7:   BL      SEQ8
SEQ8:  

        /* REMOVE FOLLOWING THREE INSTRUCTIONS WHEN POWER COLLAPSE IS ENA
         * Make sure the DBGOSLSR[LOCK] bit is cleared to allow access to the debug registers
         * Writing anything but the "secret code" to the DBGOSLAR clears the DBGOSLSR[LOCK] bit */
        MCR    p14, 0, r0, c1, c0, 4       /* WCP14_DBGOSLAR r0 */
        
        /* Read the DBGPRSR to clear the DBGPRSR[STICKYPD]
         * Any read to DBGPRSR clear the STICKYPD bit
         * ISB guarantees the read completes before attempting to 
         * execute a CP14 instruction. */
        MRC    p14, 0, r3, c1, c5, 4       /* RCP14_DBGPRSR r3 */
        ISB

        /*----------------------------------------------------------------------
          The saved Program Status Registers (SPSRs) should be setup 
          prior to any automatic mode switches. The following
          code sets these registers up to a known state. Users will need to 
          customize these settings to meet their needs.
         ----------------------------------------------------------------------*/
        MOV    r2,  #0x1f
        MOV    r1,  #0x17                 /* ABT mode         */
        msr    cpsr_c, r1                 /* ABT mode         */
        msr    spsr_cxfs, r2              /* clear the spsr   */
        MOV    r1,  #0x1b                 /* UND mode         */
        msr    cpsr_c, r1                 /* UND mode         */
        msr    spsr_cxfs, r2              /* clear the spsr   */
        MOV    r1,  #0x11                 /* FIQ mode         */
        msr    cpsr_c, r1                 /* FIQ mode         */
        msr    spsr_cxfs, r2              /* clear the spsr   */
        MOV    r1,  #0x12                 /* IRQ mode         */
        msr    cpsr_c, r1                 /* IRQ mode         */
        msr    spsr_cxfs, r2              /* clear the spsr   */
        MOV    r1,  #0x13                 /* SVC mode         */
        msr    cpsr_c, r1                 /* SVC mode         */
        msr    spsr_cxfs, r2              /* clear the spsr   */
        MOV    r1,  #0x16                 /* Monitor mode     */
        msr    cpsr_c, r1                 /* Monitor mode     */
        msr    spsr_cxfs, r2              /* clear the spsr   */

        /*----------------------------------------------------------------------
          Enabling Error reporting is something users may want to do at
          some other point in time. We have chosen some default settings  
          that should be reviewed. Most of these registers come up in an
          unpredictable state after reset.   
         ----------------------------------------------------------------------*/

        /* Setup L2CR0 with various L2/TCM control settings
           enable out of order bus attributes and error reporting
           this register comes up unpredictable after reset */
        LDR    r1, = 0xC0050F0F
        WCP15_L2CR0(r1) 

        /* Setup L2CPUCR */
/*      MOV    r2, #0xFF  */
        MOV    r2, #0
        WCP15_L2CPUCR(r2)

        /* Setup SPCR
           enable all error reporting (reset value is unpredicatble for most bits) */
        MOV    r3, #0x0F
        WCP15_SPCR(r3)

        /* Set ACTLR (reset unpredictable)
           Set AVIVT control, error reporting, etc. */
/*      MOV   r3, #0x07  */
        MOV   r3, #0x04
        WCP15_ACTLR(r3)

        /*----------------------------------------------------------------------
          Unlock ETM and read StickyPD to halt the ETM clocks from running.
          This is required for power saving whether the ETM is used or not.
         ----------------------------------------------------------------------*/
        
        /* Clear ETMOSLSR[LOCK] bit */
        MOV   r1, #0x00000000
        MCR   p14, 1, r1, c1, c0, 4        /* WCP14_ETMOSLAR       r1 */

        /* Clear ETMPDSR[STICKYPD] bit */
        MRC   p14, 1, r2, c1, c5, 4        /* RCP14_ETMPDSR       r2  */

        /* END of Scorpion Initialization */


	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	bl	platformsetup	/* go setup pll,mux,memory */

relocate:		      /* relocate U-Boot to RAM	    */
	adr	r0, _start	      /* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE	      /* test if we run from flash or RAM */
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     stack_setup

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2	      /* r2 <- size of armboot            */
	add	r2, r0, r2	      /* r2 <- source end address         */

copy_loop:
	ldmia	r0!, {r3-r10}	      /* copy from source address [r0]    */
	stmia	r1!, {r3-r10}	      /* copy to   target address [r1]    */
	cmp	r0, r2	      /* until source end addreee [r2]    */
	ble	copy_loop

	/* Set up the stack	      		    */
stack_setup:
	ldr	r0, _TEXT_BASE	           /* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN     /* malloc area  */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE     /* bdinfo */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQCONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12	           /* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start	           /* find start of bss segment  */
	ldr	r1, _bss_end	           /* stop here                  */
	mov 	r2, #0x00000000	           /* clear                      */

clbss_l:str	r2, [r0]	           /* clear loop...              */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	ldr	pc, _start_armboot

_start_armboot:
	.word start_armboot


/*
 *******************************************************************    **
 *
 * CPU_init_critical registers
 *
 * setup important registers
 *
 *******************************************************************    **
 */

cpu_init_crit:


/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	@ carve out a frame on current user stack
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}	@ Save user registers (now in svc mode) r0-r12

	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZECONFIG_SYS_MALLOC_LEN)
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE8)  @ set base 2 words into abort stack
	@ get values for "aborted" pc and cpsr (into parm regs)
	ldmia	r2, {r2 - r3}
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack
	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}	@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp		@ save current stack into r0 (param register)
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^		@ Calling SP, LR
	str	lr, [r8, #0]		@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]		@ Save CPSR
	str	r0, [r8, #8]		@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4		@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZECONFIG_SYS_MALLOC_LEN)
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE8) @ reserved a couple spots in abort stack

	str	lr, [r13]	@ save caller lr in position 0 of saved stack
	mrs	lr, spsr	@ get the spsr
	str	lr, [r13, #4]	@ save spsr in position 1 of saved stack
	mov	r13, #MODE_SVC	@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13	@ switch modes, make sure moves will execute
	mov	lr, pc		@ capture return pc
	movs	pc, lr		@ jump to next instruction & switch modes.
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl 	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl 	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq

#endif

	.align	5
.globl reset_cpu
reset_cpu:
_loop_forever:
	b	_loop_forever

