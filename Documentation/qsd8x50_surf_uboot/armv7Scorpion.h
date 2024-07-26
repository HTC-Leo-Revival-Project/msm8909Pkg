/*
 * armv7Scorpion.h 
 * Macros for accessing Scorpion CP registers
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


#ifndef __ARMV7SCORPION_H__
#define __ARMV7SCORPION_H__

/* Include macros for creating MRC and MCR instructions */
#include "arm_mrcmcr.h"


/*
 *  Macros for CP15 register writes and commands
 */

#define WCP15_SCTLR(reg)	    MCR15(reg,0,	c1	,c0	,0)
#define WCP15_ACTLR(reg)	    MCR15(reg,0,	c1	,c0	,1)

#define WCP15_TTBR0(reg)        MCR15(reg,0,    c2  ,c0 ,0)

#define WCP15_DACR(reg)         MCR15(reg,0,    c3  ,c0, 0)

#define WCP15_ICIALLU(reg)      MCR15(reg,0,	c7	,c5	,0)
#define WCP15_PFF(reg)          MCR15(reg,0,	c7	,c5	,4)
#define WCP15_BPIALL(reg)       MCR15(reg,0,	c7	,c5	,6)
#define WCP15_DSB(reg)          MCR15(reg,0,	c7	,c10,4)
#define WCP15_DMB(reg)          MCR15(reg,0,	c7	,c10,5)
#define WCP15_UTLBIALL(reg)     MCR15(reg,0,	c8	,c7	,0)
#define WCP15_DCIALL(reg)       MCR15(reg,0,	c9	,c0	,6)

#define WCP15_ICLKCR(reg)       MCR15(reg,1,    c9  ,c0 ,3)
#define WCP15_DCLKCR(reg)       MCR15(reg,1,    c9  ,c0 ,7)

#define WCP15_ICVIC0(reg)    	MCR15(reg,0,	c9	,c1	,0)
#define WCP15_ICVIC1(reg)    	MCR15(reg,0,	c9	,c1	,1)
#define WCP15_ICVIC2(reg)    	MCR15(reg,0,	c9	,c1	,2)
#define WCP15_ICVIC3(reg)    	MCR15(reg,0,	c9	,c1	,3)
#define WCP15_ICVIC4(reg)    	MCR15(reg,0,	c9	,c1	,4)
#define WCP15_ICVIC5(reg)    	MCR15(reg,0,	c9	,c1	,5)
#define WCP15_ICVIC6(reg)    	MCR15(reg,0,	c9	,c1	,6)
#define WCP15_ICVIC7(reg)    	MCR15(reg,0,	c9	,c1	,7)
#define WCP15_ICFLOOR0(reg) 	MCR15(reg,1,	c9	,c1	,0)
#define WCP15_ICFLOOR1(reg) 	MCR15(reg,1,	c9	,c1	,1)
#define WCP15_ICFLOOR2(reg) 	MCR15(reg,1,	c9	,c1	,2)
#define WCP15_ICFLOOR3(reg) 	MCR15(reg,1,	c9	,c1	,3)
#define WCP15_ICFLOOR4(reg) 	MCR15(reg,1,	c9	,c1	,4)
#define WCP15_ICFLOOR5(reg) 	MCR15(reg,1,	c9	,c1	,5)
#define WCP15_ICFLOOR6(reg) 	MCR15(reg,1,	c9	,c1	,6)
#define WCP15_ICFLOOR7(reg) 	MCR15(reg,1,	c9	,c1	,7)
#define WCP15_DCVIC0(reg)    	MCR15(reg,2,	c9	,c1	,0)
#define WCP15_DCVIC1(reg)   	MCR15(reg,2,	c9	,c1	,1)
#define WCP15_DCVIC2(reg)    	MCR15(reg,2,	c9	,c1	,2)
#define WCP15_DCVIC3(reg)    	MCR15(reg,2,	c9	,c1	,3)
#define WCP15_DCVIC4(reg)    	MCR15(reg,2,	c9	,c1	,4)
#define WCP15_DCVIC5(reg)    	MCR15(reg,2,	c9	,c1	,5)
#define WCP15_DCVIC6(reg)    	MCR15(reg,2,	c9	,c1	,6)
#define WCP15_DCVIC7(reg)    	MCR15(reg,2,	c9	,c1	,7)
#define WCP15_DCFLOOR0(reg) 	MCR15(reg,3,	c9	,c1	,0)
#define WCP15_DCFLOOR1(reg) 	MCR15(reg,3,	c9	,c1	,1)
#define WCP15_DCFLOOR2(reg) 	MCR15(reg,3,	c9	,c1	,2)
#define WCP15_DCFLOOR3(reg) 	MCR15(reg,3,	c9	,c1	,3)
#define WCP15_DCFLOOR4(reg) 	MCR15(reg,3,	c9	,c1	,4)
#define WCP15_DCFLOOR5(reg) 	MCR15(reg,3,	c9	,c1	,5)
#define WCP15_DCFLOOR6(reg) 	MCR15(reg,3,	c9	,c1	,6)
#define WCP15_DCFLOOR7(reg) 	MCR15(reg,3,	c9	,c1	,7)
#define WCP15_SPCR(reg)         MCR15(reg,0,    c9  ,c7 ,0)

#define WCP15_TLBLKCR(reg)      MCR15(reg,0,    c10 ,c1 ,3)
#define WCP15_MMUDMTR(reg)      MCR15(reg,0,    c10 ,c4 ,0)
#define WCP15_MMUDCPR(reg)      MCR15(reg,0,    c10 ,c4 ,1)

#define WCP15_DMASELR(reg)      MCR15(reg,0,    c11 ,c0 ,0)
#define WCP15_DMACHCR(reg)      MCR15(reg,0,    c11 ,c0 ,2)

#define WCP15_VBAR(reg)         MCR15(reg,0,    c12 ,c0 ,0)

#define WCP15_CONTEXTIDR(reg)   MCR15(reg,0,    c13 ,c0 ,1)

#define WCP15_BPCR(reg)         MCR15(reg,7,    c15 ,c0 ,2)  
#define WCP15_L2CR0(reg)        MCR15(reg,3,    c15 ,c0 ,1)
#define WCP15_L2CPUCR(reg)      MCR15(reg,3,    c15 ,c0 ,2)
#define WCP15_L2CR1(reg)        MCR15(reg,3,    c15 ,c0 ,3)

/*
 *  Macros for CP15 register reads
 */
 
#define RCP15_MIDR(reg)		    MRC15(reg,0,	c0  ,c0 ,0)
#define RCP15_SCTLR(reg)	    MRC15(reg,0,	c1	,c0	,0)
#define RCP15_ACTLR(reg)	    MRC15(reg,0,	c1	,c0	,1)


/*
 *  New ARM v7 barrier instructions
 *  DSB, DMB and ISB
 */

#ifdef __ASSEMBLY__
#define DSB	.word	0xf57ff04f
#else
#define DSB	asm("	.word	0xf57ff04f\n" )
#endif


#ifdef __ASSEMBLY__
#define DMB	.word	0xf57ff05f
#else
#define DMB	asm("	.word	0xf57ff05f\n" )
#endif


#ifdef __ASSEMBLY__
#define ISB	.word	0xf57ff06f
#else
#define ISB	asm("	.word	0xf57ff06f\n" )
#endif

/*
 *  Data Cache operations
 */
#define DCIMVAC(reg)          MCR15(reg,0,    c7 ,c6  ,1)
#define DCCMVAC(reg)          MCR15(reg,0,    c7 ,c10 ,1)
#define DCCIMVAC(reg)         MCR15(reg,0,    c7 ,c14 ,1)



#endif /*__ARMV7SCORPION_H__*/