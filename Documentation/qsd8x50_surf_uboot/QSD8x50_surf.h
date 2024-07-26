/*
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * (C) Copyright 2002-2005
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 *
 * Configuation settings for the QUALCOMM QSD8x50 SURF board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/QSD8x50_reg.h>
#include <asm-armv7Scorpion/armv7Scorpion.h>



#define IO_READ32(addr)        (*((volatile unsigned int *) (addr)))
#define IO_WRITE32(addr, val)  (*((volatile unsigned int *) (addr)) = ((unsigned int) (val)))
#define IO_READ16(addr)        (*((volatile unsigned short *) (addr)))
#define IO_WRITE16(addr, val)  (*((volatile unsigned short *) (addr)) = ((unsigned short) (val)))
#define IO_READ8(addr)         (*((volatile char *) (addr)))
#define IO_WRITE8(addr, val)   (*((volatile unsigned char *) (addr)) = ((unsigned char) (val)))

/*
 * High Level Configuration Options
 * (easy to change)
 */

#undef DEBUG         // ZZZZ remove

#undef CONFIG_DCACHE           // ZZZZ eventually
#undef CONFIG_START_PLL

#ifndef MACH_QSD8X50_SURF
	#define MACH_QSD8X50_SURF 1008000
#endif 

#define LINUX_MACH_TYPE	MACH_QSD8X50_SURF

#define CONFIG_SYS_HZ                  (32768)           /* GPT Timer frequency */

#define CONFIG_SHOW_BOOT_PROGRESS    /* ZZZZ temporary */

#define CONFIG_CMDLINE_TAG	   	  /* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	
#define CONFIG_INITRD_TAG 
//rlal: ATAG_CORE parameters  
#define ATAG_CORE_FLAGS 	0x00000001
#define ATAG_PAGE_SIZE	  	0x00001000
#define ATAG_CORE_RDEV		0x000000FF

#define CONFIG_MISC_INIT_R	1	/* call misc_init_r during start up */

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE  128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Ethernet driver (LAN91C111 controller)
 */
#define CONFIG_DRIVER_SMC91111
#undef  CONFIG_SMC_USE_32_BIT                 
#define CONFIG_SMC91111_BASE    (EBI2CS7_BASE  0x0300) 
#undef  CONFIG_SMC91111_EXT_PHY

#define HAPPY_LED_BASE_BANK1          (EBI2CS7_BASE  0x0282)
#define HAPPY_LED_BASE_BANK2          (EBI2CS7_BASE  0x0284)

/*
 * MMC/SD card
 */
#define CONFIG_MMC
#define CONFIG_DOS_PARTITION

/*
 * USB EHCI
 */
#define CONFIG_USB_EHCI_QC                 /* Qualcomm USB HS EHCI */
#define CONFIG_USB_EHCI
#define CONFIG_EHCI_IS_TDI                 /* ChipIdea/Transdimension EHCI IP */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET   
#undef  CONFIG_EHCI_DCACHE                 /* ZZZZ Eventually */
#undef  CONFIG_EHCI_MMIO_BIG_ENDIAN        
#undef  CONFIG_EHCI_DESC_BIG_ENDIAN        
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE

/*
 * Monitor functions
 * Defaults  some additional
 */
#include <config_cmd_default.h>      

#define	CONFIG_CMD_DHCP
#define	CONFIG_CMD_PING
#undef  CONFIG_CMD_FLASH     // no flash support
#undef  CONFIG_CMD_IMLS
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB
#define CONFIG_CMD_EXT2


/*
 * Serial port Configuration
 */
#undef  CONFIG_SILENT_CONSOLE
#define CFG_QC_SERIAL
#define CONFIG_CONS_INDEX	 0
#define CONFIG_BAUDRATE	     115200
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

#define CONFIG_BOOTDELAY	4
#define CONFIG_BOOTARGS "rdinit=/sbin/init ip=dhcp"
#define CONFIG_BOOTCOMMAND "dhcp 0x1f000000 10.228.196.66:default.scr; autoscr 0x1f000000;"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_LONGHELP	/* undef to save memory     */
#define CONFIG_SYS_PROMPT	"QSD8x50_SURF-QC> "	/* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZEsizeof(CONFIG_SYS_PROMPT)16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args   */
#define CONFIG_SYS_BARGSIZE	2048 /* Boot Argument Buffer Size    */

#define CONFIG_SYS_LOAD_ADDR	0x16007FC0 	/* default load address in EBI1 SDRAM */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map -
 * Linux runs in EBI1 SDRAM 0x16000000-0x1FFFFFFF.
 */
#define CONFIG_NR_DRAM_BANKS    1	  /* There is 1 bank of SDRAM */
#define PHYS_SDRAM_1            0x16000000	    /* EBI1 */
#define PHYS_SDRAM_1_SIZE       0x0A000000	

/*-----------------------------------------------------------------------
 * Physical Memory Map -
 * U-Boot code, data, stack, etc. reside in SMI SDRAM 0x00000000-0x000FFFFF.
 */
#define UBOOT_SDRAM_BASE         0x00000000      /* SMI */
#define UBOOT_SDRAM_SIZE         0x00100000

/* Memory Test */
#define CONFIG_SYS_MEMTEST_START       PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END         (PHYS_SDRAM_1  PHYS_SDRAM_1_SIZE)

/* Environment */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE         0x2000

/* Boot parameter address */
#define CFG_QC_BOOT_PARAM_ADDR    PHYS_SDRAM_1

/*-----------------------------------------------------------------------
 * The qc_serial driver uses the register names below. Set UART_BASE
 * for the desired UART.
 */
#define UART_BASE     UART3_BASE


/*-----------------------------------------------------------------------
 * Choose the SD controller to use. SDC1, 2, 3, or 4. 
 */
#define SDC_INSTANCE  1
#define USE_DM                    
#define USE_HIGH_SPEED_MODE       
#define USE_4_BIT_BUS_MODE        
#define CONFIG_SYS_MMC_BASE		0xF0000000    // ZZZZ remove??

#endif	