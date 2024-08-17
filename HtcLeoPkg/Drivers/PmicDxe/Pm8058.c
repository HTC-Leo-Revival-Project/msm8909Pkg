#include <Chipset/pmic_msm7230.h>
#include <Chipset/ssbi_msm7230.h>
#include <Platform/board_vision.h>
#include <Library/LKEnvLib.h>
#include "ssbi.h"

struct pm8058_gpio gpio_pm[] = {
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM8058_GPIO_VIN_L5,
        .out_strength   = PM_GPIO_STRENGTH_HIGH,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_TP_RSTz,  // Corresponds to VISION_TP_RSTz
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_VOL_UP,  // Corresponds to VISION_VOL_UP
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_VOL_DN,  // Corresponds to VISION_VOL_DN
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_SLIDING_INTz,  // Corresponds to VISION_SLIDING_INTz
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_OJ_ACTION,  // Corresponds to VISION_OJ_ACTION
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_CAM_STEP1,  // Corresponds to VISION_CAM_STEP1
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_UP_31P5,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_CAM_STEP2,  // Corresponds to VISION_CAM_STEP2
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM8058_GPIO_VIN_S3,
        .out_strength   = 0,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_AUD_HP_DETz,  // Corresponds to VISION_AUD_HP_DETz
    },
    {
        .direction      = PM8058_GPIO_MODE_INPUT,
        .pull           = PM_GPIO_PULL_NO,
        .vin_sel        = PM8058_GPIO_VIN_L5,
        .out_strength   = PM_GPIO_STRENGTH_HIGH,
        .function       = PM_GPIO_FUNC_NORMAL,
        .inv_int_pol    = 0,
        .gpio           = VISION_GPIO_PROXIMITY_EN,  // Corresponds to VISION_GPIO_PROXIMITY_EN
    },
};

/* PM8058 APIs */
int pm8058_write(uint16_t addr, uint8_t * data, uint16_t length)
{
	return gSsbi->SsbiWrite(addr,data, length);
}

int pm8058_read(uint16_t addr, uint8_t * data, uint16_t length)
{
	return gSsbi->SsbiRead(addr, data, length);
}

int pm8058_get_irq_status(pm_irq_id_type irq, bool * rt_status)
{
    unsigned char block_index = PM_IRQ_ID_TO_BLOCK_INDEX(irq);
    unsigned char reg_data;
    unsigned reg_mask;
    int errFlag;

    /* select the irq block */
    errFlag = gSsbi->SsbiWrite(IRQ_BLOCK_SEL_USR_ADDR, &block_index, 1);
    if (errFlag) {dprintf(ALWAYS, "Device Timeout");
        return 1;
    }

    /* read real time status */
    errFlag = gSsbi->SsbiRead(IRQ_STATUS_RT_USR_ADDR,&reg_data, 1);
    if (errFlag) {dprintf(ALWAYS, "Device Timeout");
        return 1;
    }

    reg_mask = PM_IRQ_ID_TO_BIT_MASK(irq);

    if ((reg_data & reg_mask) == reg_mask) {
        /* The RT Status is high. */
        *rt_status = TRUE;
    } else {
        /* The RT Status is low. */
        *rt_status = FALSE;
    }
    return 0;
}


int pm8058_reset_pwr_off(int reset)
{
	int rc;
	uint8_t pon, ctrl, smpl;

	/* Set regulator L22 to 1.225V in high power mode. */
	rc = pm8058_read(SSBI_REG_ADDR_L22_CTRL, &ctrl, 1);
	if (rc) {
		goto get_out3;
	}
	/* Leave pull-down state intact. */
	ctrl &= 0x40;
	ctrl |= 0x93;

	rc = pm8058_write(SSBI_REG_ADDR_L22_CTRL, &ctrl, 1);
	if (rc) {
	}

 get_out3:
	if (!reset) {
		/* Only modify the SLEEP_CNTL reg if shutdown is desired. */
		rc = pm8058_read(SSBI_REG_ADDR_SLEEP_CNTL, &smpl, 1);
		if (rc) {
			goto get_out2;
		}

		smpl &= ~PM8058_SLEEP_SMPL_EN_MASK;
		smpl |= PM8058_SLEEP_SMPL_EN_PWR_OFF;

		rc = pm8058_write(SSBI_REG_ADDR_SLEEP_CNTL, &smpl, 1);
		if (rc) {
		}
	}

 get_out2:
	rc = pm8058_read(SSBI_REG_ADDR_PON_CNTL_1, &pon, 1);
	if (rc) {
		goto get_out;
	}

	pon &= ~PM8058_PON_WD_EN_MASK;
	pon |= reset ? PM8058_PON_WD_EN_RESET : PM8058_PON_WD_EN_PWR_OFF;

	/* Enable all pullups */
	pon |= PM8058_PON_PUP_MASK;

	rc = pm8058_write(SSBI_REG_ADDR_PON_CNTL_1, &pon, 1);
	if (rc) {
		goto get_out;
	}

 get_out:
	return rc;
}

int pm8058_rtc0_alarm_irq_disable(void)
{
	int rc;
	uint8_t reg;

	rc = pm8058_read(PM8058_RTC_CTRL, &reg, 1);
	if (rc) {
		return rc;
	}
	reg = (reg & ~PM8058_RTC_ALARM_ENABLE);

	rc = pm8058_write(PM8058_RTC_CTRL, &reg, 1);
	if (rc) {
		return rc;
	}

	return rc;
}

bool pm8058_gpio_get(unsigned int gpio)
{
	pm_irq_id_type gpio_irq;
	bool status;
	int ret;

	gpio_irq = gpio + PM_GPIO01_CHGED_ST_IRQ_ID;
	ret = pm8058_get_irq_status(gpio_irq, &status);

	if (ret)
		dprintf(CRITICAL, "pm8058_gpio_get failed\n");

	return status;
}

int pm8058_mwrite(uint16_t addr, uint8_t val, uint8_t mask, uint8_t * reg_save)
{
	int rc = 0;
	uint8_t reg;

	reg = (*reg_save & ~mask) | (val & mask);
	if (reg != *reg_save)
		rc = pm8058_write(addr, &reg, 1);
	if (rc)
		dprintf(CRITICAL, "pm8058_write failed; addr=%03X, rc=%d\n",
			addr, rc);
	else
		*reg_save = reg;
	return rc;
}

int pm8058_ldo_set_voltage()
{
	int ret = 0;
	unsigned vprog = 0x00000110;
	ret =
	    pm8058_mwrite(PM8058_HDMI_L16_CTRL, vprog, LDO_CTRL_VPROG_MASK, 0);
	if (ret) {
		dprintf(SPEW, "Failed to set voltage for l16 regulator\n");
	}
	return ret;
}

int pm8058_vreg_enable()
{
	int ret = 0;
	ret =
	    pm8058_mwrite(PM8058_HDMI_L16_CTRL, REGULATOR_EN_MASK,
			  REGULATOR_EN_MASK, 0);
	if (ret) {
		dprintf(SPEW, "Vreg enable failed for PM 8058\n");
	}
	return ret;
}

int pm8058_gpio_config(int gpio, struct pm8058_gpio *param)
{
	int	rc;
	unsigned char bank[8];
	static int dir_map[] = {
		PM8058_GPIO_MODE_OFF,
		PM8058_GPIO_MODE_OUTPUT,
		PM8058_GPIO_MODE_INPUT,
		PM8058_GPIO_MODE_BOTH,
	};

	if (param == 0) {
	  dprintf (INFO, "pm8058_gpio struct not defined\n");
          return -1;
	}

	/* Select banks and configure the gpio */
	bank[0] = PM8058_GPIO_WRITE |
		((param->vin_sel << PM8058_GPIO_VIN_SHIFT) &
			PM8058_GPIO_VIN_MASK) |
		PM8058_GPIO_MODE_ENABLE;
	bank[1] = PM8058_GPIO_WRITE |
		((1 << PM8058_GPIO_BANK_SHIFT) & PM8058_GPIO_BANK_MASK) |
		((dir_map[param->direction] << PM8058_GPIO_MODE_SHIFT) &
			PM8058_GPIO_MODE_MASK) |
		((param->direction & PM_GPIO_DIR_OUT) ?
			PM8058_GPIO_OUT_BUFFER : 0);
	bank[2] = PM8058_GPIO_WRITE |
		((2 << PM8058_GPIO_BANK_SHIFT) & PM8058_GPIO_BANK_MASK) |
		((param->pull << PM8058_GPIO_PULL_SHIFT) &
			PM8058_GPIO_PULL_MASK);
	bank[3] = PM8058_GPIO_WRITE |
		((3 << PM8058_GPIO_BANK_SHIFT) & PM8058_GPIO_BANK_MASK) |
		((param->out_strength << PM8058_GPIO_OUT_STRENGTH_SHIFT) &
			PM8058_GPIO_OUT_STRENGTH_MASK);
	bank[4] = PM8058_GPIO_WRITE |
		((4 << PM8058_GPIO_BANK_SHIFT) & PM8058_GPIO_BANK_MASK) |
		((param->function << PM8058_GPIO_FUNC_SHIFT) &
			PM8058_GPIO_FUNC_MASK);

	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_GPIO(gpio),bank, 5);
	if (rc) {
        dprintf(ALWAYS, "Failed on 1st ssbi_write(): rc=%d.\n", rc);
		return 1;
	}
	return 0;
}

int pm8058_gpio_config_kypd_drv(int gpio_start, int num_gpios, unsigned mach_id)
{
	int	rc;
	struct pm8058_gpio kypd_drv = {
		.direction	= PM_GPIO_DIR_OUT,
		.pull		= PM_GPIO_PULL_NO,
		.vin_sel	= 2,
		.out_strength	= PM_GPIO_STRENGTH_LOW,
		.function	= PM_GPIO_FUNC_1,
		.inv_int_pol	= 1,
	};


	while (num_gpios--) {
		rc = pm8058_gpio_config(gpio_start++, &kypd_drv);
		if (rc) {
	dprintf(ALWAYS, "FAIL pm8058_gpio_config(): rc=%d.\n", rc);
			return rc;
		}
	}

	return 0;
}

int pm8058_gpio_config_kypd_sns(int gpio_start, int num_gpios)
{
	int	rc;
	struct pm8058_gpio kypd_sns = {
		.direction	= PM_GPIO_DIR_IN,
		.pull		= PM_GPIO_PULL_UP1,
		.vin_sel	= 2,
		.out_strength	= PM_GPIO_STRENGTH_NO,
		.function	= PM_GPIO_FUNC_NORMAL,
		.inv_int_pol	= 1,
	};

	while (num_gpios--) {
		rc = pm8058_gpio_config(gpio_start++, &kypd_sns);
		if (rc) {
		        dprintf(ALWAYS, "FAIL pm8058_gpio_config(): rc=%d.\n", rc);
			return rc;
		}
	}

	return 0;
}
extern UINTN EFIAPI MicroSecondDelay (IN      UINTN                     MicroSeconds);

void configkeypadgpios(void){

    int rc;
    for (int i =0; i < 8; i++) {
		rc = pm8058_gpio_config(gpio_pm[i].gpio, &gpio_pm[i]);
		if (rc) {
	        dprintf(ALWAYS, "FAIL pm8058_gpio_config(): rc=%d.\n", rc);
            MicroSecondDelay(500000);
		}
	}
}