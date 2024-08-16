#include "commonvars.h"

UINT8			*bank1;

int pm_gpio_get(unsigned gpio)
{
	int	mode;

	if (gpio >= PM8058_GPIOS)
		return -1;

	/* Get gpio value from config bank 1 if output gpio.
	   Get gpio value from IRQ RT status register for all other gpio modes.
	 */
	mode = (bank1[gpio] & PM_GPIO_MODE_MASK) >>PM_GPIO_MODE_SHIFT;
	if (mode == PM_GPIO_MODE_OUTPUT)
		return bank1[gpio] & PM_GPIO_OUT_INVERT;
	else
		return pm8058_read_irq_stat(PMIC8058_IRQ_BASE + gpio);
}

int pm_gpio_set(unsigned gpio, int value)
{
	int rc;
	UINT8 bank2; //actually bank1 but conversion to uefi requires a global bank1 variable so its bank2 here
	EFI_TPL old_tpl;  // Variable to store the old TPL

	if (gpio >= PM8058_GPIOS)
		return -1;

	// Raise TPL to HIGH level
	old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

	bank2 = PM_GPIO_WRITE | (bank1[gpio] & ~PM_GPIO_OUT_INVERT);

	if (value)
		bank2 |= PM_GPIO_OUT_INVERT;

	bank1[gpio] = bank2;
	rc = pm8058_writeb(SSBI_REG_ADDR_GPIO(gpio), bank2);

	// Restore TPL to the previous level
	gBS->RestoreTPL(old_tpl);

	if (rc)
		DEBUG((EFI_D_ERROR, "FAIL pm8058_writeb(): rc=%d. (gpio=%d, value=%d)\n", rc, gpio, value));

	return rc;
}

int pm_gpio_set_direction(unsigned gpio, int direction)
{
	int rc;
	UINT8 bank;
	EFI_TPL old_tpl;  // Variable to store the old TPL

	if (!direction)
		return -1;

	// Raise TPL to HIGH level
	old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

	bank = PM_GPIO_WRITE | (bank1[gpio] & ~PM_GPIO_MODE_MASK);
	bank |= ((dir_map[direction] << PM_GPIO_MODE_SHIFT) & PM_GPIO_MODE_MASK);

	bank1[gpio] = bank;
	rc = pm8058_writeb(SSBI_REG_ADDR_GPIO(gpio), bank);

	// Restore TPL to the previous level
	gBS->RestoreTPL(old_tpl);

	if (rc)
		DEBUG((EFI_D_ERROR, "Failed on pm8058_writeb(): rc=%d (GPIO config)\n", rc));

	return rc;
}

int pm_gpio_init_bank1(void)
{
	int i, rc;
	UINT8 bank;

	for (i = 0; i < PM8058_GPIOS; i++) {
		bank = 1 << PM_GPIO_BANK_SHIFT;
		rc = pm8058_writeb(SSBI_REG_ADDR_GPIO(i), bank);
		if (rc) {
			DEBUG((EFI_D_ERROR, "error setting bank rc=%d\n", rc));
			return rc;
		}

		rc = pm8058_readb(SSBI_REG_ADDR_GPIO(i), &bank1[i]);
		if (rc) {
			DEBUG((EFI_D_ERROR, "error reading bank 1 rc=%d\n", rc));
			return rc;
		}
	}
	return 0;
}

int pm_gpio_to_irq(unsigned offset)
{

	return PMIC8058_IRQ_BASE + offset;
}

int pm_gpio_read(unsigned offset)
{

	return pm_gpio_get(offset);
}

void pm_gpio_write(unsigned offset, int val)
{

	pm_gpio_set(offset, val);
}

int pm_gpio_direction_input(unsigned offset)
{

	return pm_gpio_set_direction(offset, PM_GPIO_DIR_IN);
}

int pm_gpio_direction_output(unsigned offset,int val)
{
	int ret;

	ret = pm_gpio_set_direction(offset, PM_GPIO_DIR_OUT);
	if (!ret)
		ret = pm_gpio_set(offset, val);

	return ret;
}

void pm_gpio_dbg_show(void)
{
	const char * const cmode[] = { "in", "in/out", "out", "off" };
	UINT8 mode, state, bank;
	int i, j;

	for (i = 0; i < PM8058_GPIOS; i++) {
		mode = (bank1[i] & PM_GPIO_MODE_MASK) >>PM_GPIO_MODE_SHIFT;
		state = pm_gpio_get(i);
        DEBUG((EFI_D_ERROR, "gpio-%-3d (%-12.12s) %-10.10s"" %s",NR_GPIO_IRQS + i,cmode[mode],state ? "hi" : "lo"));
		for (j = 0; j < PM_GPIO_BANKS; j++) {
			bank = j << PM_GPIO_BANK_SHIFT;
			pm8058_writeb(SSBI_REG_ADDR_GPIO(i),bank);
			pm8058_readb(SSBI_REG_ADDR_GPIO(i),&bank);
            DEBUG((EFI_D_ERROR, " 0x%02x", bank));
		}
        DEBUG((EFI_D_ERROR, "\n"));
	}
}


void pm8058_gpio_init(void){




}


