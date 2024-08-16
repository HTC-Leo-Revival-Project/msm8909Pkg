#include "commonvars.h"

int pm8058_masked_write(UINT16 addr, UINT8 val, UINT8 mask)
{
	int rc;
	UINT8 reg;

	rc = gSsbi->SsbiRead(addr, &reg, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: ssbi_read(0x%03X) failed: rc=%d\n", __func__, addr,rc));
		goto done;
	}

	reg &= ~mask;
	reg |= val & mask;

	rc = gSsbi->SsbiWrite(addr, &reg, 1);
	if (rc)
    DEBUG((EFI_D_ERROR, "%s: ssbi_write(0x%03X)=0x%02X failed: rc=%d\n",__func__, addr, reg, rc));
done:
	return rc;
}

/**
 * pm8058_smpl_control - enables/disables SMPL detection
 * @enable: 0 = shutdown PMIC on power loss, 1 = reset PMIC on power loss
 *
 * This function enables or disables the Sudden Momentary Power Loss detection
 * module.  If SMPL detection is enabled, then when a sufficiently long power
 * loss event occurs, the PMIC will automatically reset itself.  If SMPL
 * detection is disabled, then the PMIC will shutdown when power loss occurs.
 *
 * RETURNS: an appropriate -ERRNO error value on error, or zero for success.
 */
int pm8058_smpl_control(int enable)
{
	return pm8058_masked_write(SSBI_REG_ADDR_SLEEP_CNTL,
				   (enable ? PM8058_SLEEP_SMPL_EN_RESET
					   : PM8058_SLEEP_SMPL_EN_PWR_OFF),
				   PM8058_SLEEP_SMPL_EN_MASK);
}

/**
 * pm8058_smpl_set_delay - sets the SMPL detection time delay
 * @delay: enum value corresponding to delay time
 *
 * This function sets the time delay of the SMPL detection module.  If power
 * is reapplied within this interval, then the PMIC reset automatically.  The
 * SMPL detection module must be enabled for this delay time to take effect.
 *
 * RETURNS: an appropriate -ERRNO error value on error, or zero for success.
 */
int pm8058_smpl_set_delay(enum pm8058_smpl_delay delay)
{
	if (delay < PM8058_SLEEP_SMPL_SEL_MIN
	    || delay > PM8058_SLEEP_SMPL_SEL_MAX) {
    DEBUG((EFI_D_ERROR, "%s: invalid delay specified: %d\n", __func__, delay));
		return -1;
	}

	return pm8058_masked_write(SSBI_REG_ADDR_SLEEP_CNTL, delay,
				   PM8058_SLEEP_SMPL_SEL_MASK);
}

/**
 * pm8058_watchdog_reset_control - enables/disables watchdog reset detection
 * @enable: 0 = shutdown when PS_HOLD goes low, 1 = reset when PS_HOLD goes low
 *
 * This function enables or disables the PMIC watchdog reset detection feature.
 * If watchdog reset detection is enabled, then the PMIC will reset itself
 * when PS_HOLD goes low.  If it is not enabled, then the PMIC will shutdown
 * when PS_HOLD goes low.
 *
 * RETURNS: an appropriate -ERRNO error value on error, or zero for success.
 */
int pm8058_watchdog_reset_control(int enable)
{
	return pm8058_masked_write(SSBI_REG_ADDR_PON_CNTL_1,
				   (enable ? PM8058_PON_WD_EN_RESET
					   : PM8058_PON_WD_EN_PWR_OFF),
				   PM8058_PON_WD_EN_MASK);
}

/*
 * Set an SMPS regulator to be disabled in its CTRL register, but enabled
 * in the master enable register.  Also set it's pull down enable bit.
 * Take care to make sure that the output voltage doesn't change if switching
 * from advanced mode to legacy mode.
 */
int disable_smps_locally_set_pull_down(UINT16 ctrl_addr, UINT16 test2_addr,
		UINT16 master_enable_addr, UINT8 master_enable_bit)
{
	int rc = 0;
	UINT8 vref_sel, vlow_sel, band, vprog, bank, reg;

	bank = REGULATOR_BANK_SEL(7);
	rc = gSsbi->SsbiWrite(test2_addr, &bank, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%03X): rc=%d\n", __func__,test2_addr, rc));
		goto done;
	}

	rc = gSsbi->SsbiRead(test2_addr, &reg, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL pm8058_read(0x%03X): rc=%d\n",__func__, test2_addr, rc));
		goto done;
	}

	/* Check if in advanced mode. */
	if ((reg & SMPS_ADVANCED_MODE_MASK) == SMPS_ADVANCED_MODE) {
		/* Determine current output voltage. */
		rc = gSsbi->SsbiRead(ctrl_addr, &reg, 1);
		if (rc) {
      DEBUG((EFI_D_ERROR, "%s: FAIL pm8058_read(0x%03X): rc=%d\n",__func__, ctrl_addr, rc));
			goto done;
		}

		band = (reg & SMPS_ADVANCED_BAND_MASK)
			>> SMPS_ADVANCED_BAND_SHIFT;
		switch (band) {
		case 3:
			vref_sel = 0;
			vlow_sel = 0;
			break;
		case 2:
			vref_sel = SMPS_LEGACY_VREF_SEL;
			vlow_sel = 0;
			break;
		case 1:
			vref_sel = SMPS_LEGACY_VREF_SEL;
			vlow_sel = SMPS_LEGACY_VLOW_SEL;
			break;
		default:
      DEBUG((EFI_D_ERROR, "%s: regulator already disabled\n", __func__));
			return -1;
		}
		vprog = (reg & SMPS_ADVANCED_VPROG_MASK);
		/* Round up if fine step is in use. */
		vprog = (vprog + 1) >> 1;
		if (vprog > SMPS_LEGACY_VPROG_MASK)
			vprog = SMPS_LEGACY_VPROG_MASK;

		/* Set VLOW_SEL bit. */
		bank = REGULATOR_BANK_SEL(1);
		rc = gSsbi->SsbiWrite(test2_addr, &bank, 1);
		if (rc) {
      DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%03X): rc=%d\n",__func__, test2_addr, rc));
			goto done;
		}
		rc = pm8058_masked_write(test2_addr,
			REGULATOR_BANK_WRITE | REGULATOR_BANK_SEL(1)
				| vlow_sel,
			REGULATOR_BANK_WRITE | REGULATOR_BANK_MASK
				| SMPS_LEGACY_VLOW_SEL);
		if (rc)
			goto done;

		/* Switch to legacy mode */
		bank = REGULATOR_BANK_SEL(7);
		rc = gSsbi->SsbiWrite(test2_addr, &bank, 1);
		if (rc) {
      DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%03X): rc=%d\n", __func__,test2_addr, rc));
			goto done;
		}
		rc = pm8058_masked_write(test2_addr,
				REGULATOR_BANK_WRITE | REGULATOR_BANK_SEL(7)
					| SMPS_LEGACY_MODE,
				REGULATOR_BANK_WRITE | REGULATOR_BANK_MASK
					| SMPS_ADVANCED_MODE_MASK);
		if (rc)
			goto done;

		/* Enable locally, enable pull down, keep voltage the same. */
		rc = pm8058_masked_write(ctrl_addr,
			REGULATOR_ENABLE | REGULATOR_PULL_DOWN_EN
				| vref_sel | vprog,
			REGULATOR_ENABLE_MASK | REGULATOR_PULL_DOWN_MASK
			       | SMPS_LEGACY_VREF_SEL | SMPS_LEGACY_VPROG_MASK);
		if (rc)
			goto done;
	}

	/* Enable in master control register. */
	rc = pm8058_masked_write(master_enable_addr, master_enable_bit,
				 master_enable_bit);
	if (rc)
		goto done;

	/* Disable locally and enable pull down. */
	rc = pm8058_masked_write(ctrl_addr,
		REGULATOR_DISABLE | REGULATOR_PULL_DOWN_EN,
		REGULATOR_ENABLE_MASK | REGULATOR_PULL_DOWN_MASK);

done:
	return rc;
}

int disable_ldo_locally_set_pull_down(UINT16 ctrl_addr,UINT16 master_enable_addr, UINT8 master_enable_bit)
{
	int rc;

	/* Enable LDO in master control register. */
	rc = pm8058_masked_write(master_enable_addr, master_enable_bit,master_enable_bit);
	if (rc)
		goto done;

	/* Disable LDO in CTRL register and set pull down */
	rc = pm8058_masked_write(ctrl_addr,
		REGULATOR_DISABLE | REGULATOR_PULL_DOWN_EN,
		REGULATOR_ENABLE_MASK | REGULATOR_PULL_DOWN_MASK);

done:
	return rc;
}

int pm8058_reset_pwr_off(int reset)
{
	int rc;
	UINT8 pon, ctrl, smpl;

	/* When shutting down, enable active pulldowns on important rails. */
	if (!reset) {
		/* Disable SMPS's 0,1,3 locally and set pulldown enable bits. */
		disable_smps_locally_set_pull_down(SSBI_REG_ADDR_S0_CTRL,SSBI_REG_ADDR_S0_TEST2, SSBI_REG_ADDR_VREG_EN_MSM, BIT(7));
		disable_smps_locally_set_pull_down(SSBI_REG_ADDR_S1_CTRL,SSBI_REG_ADDR_S1_TEST2, SSBI_REG_ADDR_VREG_EN_MSM, BIT(6));
		disable_smps_locally_set_pull_down(SSBI_REG_ADDR_S3_CTRL,SSBI_REG_ADDR_S3_TEST2, SSBI_REG_ADDR_VREG_EN_GRP_5_4,BIT(7) | BIT(4));
		/* Disable LDO 21 locally and set pulldown enable bit. */
		disable_ldo_locally_set_pull_down(SSBI_REG_ADDR_L21_CTRL,SSBI_REG_ADDR_VREG_EN_GRP_5_4, BIT(1));
	}

	/* Set regulator L22 to 1.225V in high power mode. */
	rc = gSsbi->SsbiRead(SSBI_REG_ADDR_L22_CTRL, &ctrl, 1);
	if (rc) {
		DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_read(0x%x): rc=%d\n", __func__,SSBI_REG_ADDR_L22_CTRL, rc));
		goto get_out3;
	}
	/* Leave pull-down state intact. */
	ctrl &= 0x40;
	ctrl |= 0x93;
	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_L22_CTRL, &ctrl, 1);
	if (rc)
		DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n", __func__,SSBI_REG_ADDR_L22_CTRL, ctrl, rc));

get_out3:
	if (!reset) {
		/* Only modify the SLEEP_CNTL reg if shutdown is desired. */
		rc = gSsbi->SsbiRead(SSBI_REG_ADDR_SLEEP_CNTL,&smpl, 1);
		if (rc) {
			DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_read(0x%x): rc=%d\n",__func__, SSBI_REG_ADDR_SLEEP_CNTL, rc));
			goto get_out2;
		}

		smpl &= ~PM8058_SLEEP_SMPL_EN_MASK;
		smpl |= PM8058_SLEEP_SMPL_EN_PWR_OFF;

		rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_SLEEP_CNTL,&smpl, 1);
		if (rc)
			DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n",__func__, SSBI_REG_ADDR_SLEEP_CNTL, smpl, rc));
	}

get_out2:
	rc = gSsbi->SsbiRead(SSBI_REG_ADDR_PON_CNTL_1, &pon, 1);
	if (rc) {
		DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_read(0x%x): rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_1, rc));
		goto get_out;
	}

	pon &= ~PM8058_PON_WD_EN_MASK;
	pon |= reset ? PM8058_PON_WD_EN_RESET : PM8058_PON_WD_EN_PWR_OFF;

	/* Enable all pullups */
	pon |= PM8058_PON_PUP_MASK;

	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_PON_CNTL_1, &pon, 1);
	if (rc) {
		DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_1, pon, rc));
		goto get_out;
	}

get_out:
	return rc;
}


/**
 * pm8058_stay_on - enables stay_on feature
 *
 * PMIC stay-on feature allows PMIC to ignore MSM PS_HOLD=low
 * signal so that some special functions like debugging could be
 * performed.
 *
 * This feature should not be used in any product release.
 *
 * RETURNS: an appropriate -ERRNO error value on error, or zero for success.
 */
int pm8058_stay_on(void)
{
	UINT8	ctrl = 0x92;
	int	rc;

	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_GP_TEST_1, &ctrl, 1);
  DEBUG((EFI_D_ERROR, "%s: set stay-on: rc = %d\n", __func__, rc));
	return rc;
}

/*
   power on hard reset configuration
   config = DISABLE_HARD_RESET to disable hard reset
	  = SHUTDOWN_ON_HARD_RESET to turn off the system on hard reset
	  = RESTART_ON_HARD_RESET to restart the system on hard reset
 */
int pm8058_hard_reset_config(enum pon_config config)
{
	int rc, ret;
	UINT8 pon, pon_5;

	if (config >= MAX_PON_CONFIG)
		return -1;

	//mutex_lock(&pmic_chip->pm_lock);

	rc = gSsbi->SsbiRead(SSBI_REG_ADDR_PON_CNTL_5, &pon, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_read(0x%x): rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_5, rc));
		//mutex_unlock(&pmic_chip->pm_lock);
		return rc;
	}

	pon_5 = pon;
	(config != DISABLE_HARD_RESET) ? (pon |= PM8058_HARD_RESET_EN_MASK) :
					(pon &= ~PM8058_HARD_RESET_EN_MASK);

	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_PON_CNTL_5, &pon, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_5, pon, rc));
		//mutex_unlock(&pmic_chip->pm_lock);
		return rc;
	}

	if (config == DISABLE_HARD_RESET) {
		//mutex_unlock(&pmic_chip->pm_lock);
		return 0;
	}

	rc = gSsbi->SsbiRead(SSBI_REG_ADDR_PON_CNTL_4, &pon, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_read(0x%x): rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_4, rc));
		goto err_restore_pon_5;
	}

	(config == RESTART_ON_HARD_RESET) ? (pon |= PM8058_PON_RESET_EN_MASK) :
					(pon &= ~PM8058_PON_RESET_EN_MASK);

	rc = gSsbi->SsbiWrite(SSBI_REG_ADDR_PON_CNTL_4, &pon, 1);
	if (rc) {
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_4, pon, rc));
		goto err_restore_pon_5;
	}
	//mutex_unlock(&pmic_chip->pm_lock);
	return 0;

err_restore_pon_5:
	ret = gSsbi->SsbiWrite(SSBI_REG_ADDR_PON_CNTL_5, &pon_5, 1);
	if (ret)
    DEBUG((EFI_D_ERROR, "%s: FAIL ssbi_write(0x%x)=0x%x: rc=%d\n",__func__, SSBI_REG_ADDR_PON_CNTL_5, pon, ret));
	//mutex_unlock(&pmic_chip->pm_lock);
	return rc;
}


int pm8058_readb(UINT16 addr, UINT8 *val)
{

	return gSsbi->SsbiRead(addr, val, 1);
}

int pm8058_writeb(UINT16 addr, UINT8 val)
{

	return gSsbi->SsbiWrite(addr, &val, 1);
}

int pm8058_read_buf(UINT16 addr, UINT8 *buf,int cnt)
{

	return gSsbi->SsbiRead(addr, buf, cnt);
}

int pm8058_write_buf(UINT16 addr, UINT8 *buf,int cnt)
{

	return gSsbi->SsbiWrite(addr, buf, cnt);
}

int pm8058_read_irq_stat(int irq)
{

	return pm8xxx_get_irq_stat(irq);
}

enum pm8xxx_version pm8058_get_version(void)
{
	enum pm8xxx_version version = -1;

	if ((revision != 0) &&((revision & PM8058_VERSION_MASK) == PM8058_VERSION_VALUE))
		version = PM8XXX_VERSION_8058;

	return version;
}

int pm8058_get_revision(void)
{

	return revision & PM8058_REVISION_MASK;
}