#ifndef __PLATFORM_QSD8K_GPIO_HW_H
#define __PLATFORM_QSD8K_GPIO_HW_H

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


void msm_gpio_set_owner(unsigned gpio, unsigned owner);
void msm_gpio_init(void);
void msm_gpio_deinit(void);

#endif