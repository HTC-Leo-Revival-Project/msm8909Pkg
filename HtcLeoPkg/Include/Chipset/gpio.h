/*
 * Copyright (c) 2008, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __PLATFORM_QSD8K_GPIO_HW_H
#define __PLATFORM_QSD8K_GPIO_HW_H

#define GPIOF_IRQF_MASK         0x0000ffff
#define GPIOF_IRQF_TRIGGER_NONE 0x00010000
#define GPIOF_INPUT             0x00020000
#define GPIOF_DRIVE_OUTPUT      0x00040000
#define GPIOF_OUTPUT_LOW        0x00080000
#define GPIOF_OUTPUT_HIGH       0x00100000

#define GPIO1_REG(off)    (MSM_GPIO1_BASE + 0x800 + (off))
#define GPIO2_REG(off)    (MSM_GPIO2_BASE + 0xC00 + (off))

#define GPIO1_REG_R(off)  (MSM_GPIOCFG1_BASE + 0x000 + (off))
#define GPIO2_REG_R(off)  (MSM_GPIOCFG2_BASE + 0x400 + (off))

/* pin ownership */
#define GPIO_OWNER_0       GPIO1_REG_R(0x100)
#define GPIO_OWNER_1       GPIO2_REG_R(0x104)
#define GPIO_OWNER_2       GPIO1_REG_R(0x108)
#define GPIO_OWNER_3       GPIO1_REG_R(0x10c)
#define GPIO_OWNER_4       GPIO1_REG_R(0x110)
#define GPIO_OWNER_5       GPIO1_REG_R(0x114)
#define GPIO_OWNER_6       GPIO1_REG_R(0x118)
#define GPIO_OWNER_7       GPIO1_REG_R(0x11c)

/* output value */
#define GPIO_OUT_0         GPIO1_REG(0x00)  /* gpio  15-0   */
#define GPIO_OUT_1         GPIO2_REG(0x00)  /* gpio  42-16  */
#define GPIO_OUT_2         GPIO1_REG(0x04)  /* gpio  67-43  */
#define GPIO_OUT_3         GPIO1_REG(0x08)  /* gpio  94-68  */
#define GPIO_OUT_4         GPIO1_REG(0x0C)  /* gpio 103-95  */
#define GPIO_OUT_5         GPIO1_REG(0x10)  /* gpio 121-104 */
#define GPIO_OUT_6         GPIO1_REG(0x14)  /* gpio 152-122 */
#define GPIO_OUT_7         GPIO1_REG(0x18)  /* gpio 164-153 */

/* same pin map as above, output enable */
#define GPIO_OE_0          GPIO1_REG(0x20)
#define GPIO_OE_1          GPIO2_REG(0x08)
#define GPIO_OE_2          GPIO1_REG(0x24)
#define GPIO_OE_3          GPIO1_REG(0x28)
#define GPIO_OE_4          GPIO1_REG(0x2C)
#define GPIO_OE_5          GPIO1_REG(0x30)
#define GPIO_OE_6          GPIO1_REG(0x34)
#define GPIO_OE_7          GPIO1_REG(0x38)

/* same pin map as above, input read */
#define GPIO_IN_0          GPIO1_REG(0x50)
#define GPIO_IN_1          GPIO2_REG(0x20)
#define GPIO_IN_2          GPIO1_REG(0x54)
#define GPIO_IN_3          GPIO1_REG(0x58)
#define GPIO_IN_4          GPIO1_REG(0x5C)
#define GPIO_IN_5          GPIO1_REG(0x60)
#define GPIO_IN_6          GPIO1_REG(0x64)
#define GPIO_IN_7          GPIO1_REG(0x68)

/* same pin map as above, 1=edge 0=level interrup */
#define GPIO_INT_EDGE_0    GPIO1_REG(0x70)
#define GPIO_INT_EDGE_1    GPIO2_REG(0x50)
#define GPIO_INT_EDGE_2    GPIO1_REG(0x74)
#define GPIO_INT_EDGE_3    GPIO1_REG(0x78)
#define GPIO_INT_EDGE_4    GPIO1_REG(0x7C)
#define GPIO_INT_EDGE_5    GPIO1_REG(0x80)
#define GPIO_INT_EDGE_6    GPIO1_REG(0x84)
#define GPIO_INT_EDGE_7    GPIO1_REG(0x88)

/* same pin map as above, 1=positive 0=negative */
#define GPIO_INT_POS_0     GPIO1_REG(0x90)
#define GPIO_INT_POS_1     GPIO2_REG(0x58)
#define GPIO_INT_POS_2     GPIO1_REG(0x94)
#define GPIO_INT_POS_3     GPIO1_REG(0x98)
#define GPIO_INT_POS_4     GPIO1_REG(0x9C)
#define GPIO_INT_POS_5     GPIO1_REG(0xA0)
#define GPIO_INT_POS_6     GPIO1_REG(0xA4)
#define GPIO_INT_POS_7     GPIO1_REG(0xA8)

/* same pin map as above, interrupt enable */
#define GPIO_INT_EN_0      GPIO1_REG(0xB0)
#define GPIO_INT_EN_1      GPIO2_REG(0x60)
#define GPIO_INT_EN_2      GPIO1_REG(0xB4)
#define GPIO_INT_EN_3      GPIO1_REG(0xB8)
#define GPIO_INT_EN_4      GPIO1_REG(0xBC)
#define GPIO_INT_EN_5      GPIO1_REG(0xC0)
#define GPIO_INT_EN_6      GPIO1_REG(0xC4)
#define GPIO_INT_EN_7      GPIO1_REG(0xC8)

/* same pin map as above, write 1 to clear interrupt */
#define GPIO_INT_CLEAR_0   GPIO1_REG(0xD0)
#define GPIO_INT_CLEAR_1   GPIO2_REG(0x68)
#define GPIO_INT_CLEAR_2   GPIO1_REG(0xD4)
#define GPIO_INT_CLEAR_3   GPIO1_REG(0xD8)
#define GPIO_INT_CLEAR_4   GPIO1_REG(0xDC)
#define GPIO_INT_CLEAR_5   GPIO1_REG(0xE0)
#define GPIO_INT_CLEAR_6   GPIO1_REG(0xE4)
#define GPIO_INT_CLEAR_7   GPIO1_REG(0xE8)

/* same pin map as above, 1=interrupt pending */
#define GPIO_INT_STATUS_0  GPIO1_REG(0xF0)
#define GPIO_INT_STATUS_1  GPIO2_REG(0x70)
#define GPIO_INT_STATUS_2  GPIO1_REG(0xF4)
#define GPIO_INT_STATUS_3  GPIO1_REG(0xF8)
#define GPIO_INT_STATUS_4  GPIO1_REG(0xFC)
#define GPIO_INT_STATUS_5  GPIO1_REG(0x100)
#define GPIO_INT_STATUS_6  GPIO1_REG(0x103)
#define GPIO_INT_STATUS_7  GPIO1_REG(0x108)

enum {
	MSM_GPIO_CFG_ENABLE,
	MSM_GPIO_CFG_DISABLE,
};

enum {
	MSM_GPIO_CFG_INPUT,
	MSM_GPIO_CFG_OUTPUT,
};

/* GPIO TLMM: Pullup/Pulldown */
enum {
	MSM_GPIO_CFG_NO_PULL,
	MSM_GPIO_CFG_PULL_DOWN,
	MSM_GPIO_CFG_KEEPER,
	MSM_GPIO_CFG_PULL_UP,
};

/* GPIO TLMM: Drive Strength */
enum {
	MSM_GPIO_CFG_2MA,
	MSM_GPIO_CFG_4MA,
	MSM_GPIO_CFG_6MA,
	MSM_GPIO_CFG_8MA,
	MSM_GPIO_CFG_10MA,
	MSM_GPIO_CFG_12MA,
	MSM_GPIO_CFG_14MA,
	MSM_GPIO_CFG_16MA,
};

enum {
	MSM_GPIO_OWNER_BASEBAND,
	MSM_GPIO_OWNER_ARM11,
};

enum gpio_irq_trigger {
	GPIO_IRQF_NONE,
	GPIO_IRQF_RISING = 1,
	GPIO_IRQF_FALLING = 2,
	GPIO_IRQF_BOTH = GPIO_IRQF_RISING | GPIO_IRQF_FALLING,
};

#define MSM_GPIO_CFG(gpio, func, dir, pull, drvstr)	(						\
													(((gpio) & 0x3FF) << 4)|\
													((func) & 0xf)         |\
													(((dir) & 0x1) << 14)  |\
													(((pull) & 0x3) << 15) |\
													(((drvstr) & 0xF) << 17)\
													)

#define MSM_GPIO_PIN(gpio_cfg)    	(((gpio_cfg) >>  4) & 0x3ff)
#define MSM_GPIO_FUNC(gpio_cfg)   	(((gpio_cfg) >>  0) & 0xf)
#define MSM_GPIO_DIR(gpio_cfg)    	(((gpio_cfg) >> 14) & 0x1)
#define MSM_GPIO_PULL(gpio_cfg)   	(((gpio_cfg) >> 15) & 0x3)
#define MSM_GPIO_DRVSTR(gpio_cfg) 	(((gpio_cfg) >> 17) & 0xf)

#define NR_MSM_GPIOS 164

void msm_gpio_set_owner(unsigned gpio, unsigned owner);
void msm_gpio_init(void);
void msm_gpio_deinit(void);

/* Specific gpios definitions */
#define HTCLEO_GPIO_SD_STATUS 153

/* Definitions moved from the driver */
struct gpioregs_t {
	unsigned out;
	unsigned in;
	unsigned int_status;
	unsigned int_clear;
	unsigned int_en;
	unsigned int_edge;
	unsigned int_pos;
	unsigned oe;
	unsigned owner;
	unsigned start;
	unsigned end;
};

typedef struct gpioregs_t gpioregs;

/*
 * IRQ -> GPIO mapping table

static signed char irq2gpio[32] = {
	-1, -1, -1, -1, -1, -1,  0,  1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1,  2,  3,  4,  5,  6,
	 7,  8,  9, 10, 11, 12, -1, -1,
};*/

#endif	//__PLATFORM_QSD8K_GPIO_HW_H