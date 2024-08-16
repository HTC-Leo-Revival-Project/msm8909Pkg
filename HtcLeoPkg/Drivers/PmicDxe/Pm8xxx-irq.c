#include "commonvars.h"


static int pm8xxx_read_root_irq(UINT8 *rp)
{
	return pm8058_readb(SSBI_REG_ADDR_IRQ_ROOT(REG_IRQ_BASE), rp);
}

static int pm8xxx_read_master_irq(UINT8 m, UINT8 *bp)
{
	return pm8058_readb(SSBI_REG_ADDR_IRQ_M_STATUS1(REG_IRQ_BASE) + m, bp);
}

static int pm8xxx_read_block_irq(UINT8 bp, UINT8 *ip)
{
    int rc;
    EFI_TPL old_tpl;  // Variable to store the old TPL

    // Raise TPL to HIGH level
    old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    rc = pm8058_writeb(SSBI_REG_ADDR_IRQ_BLK_SEL(REG_IRQ_BASE), bp);
    if (rc) {
        DEBUG((EFI_D_ERROR, "Failed Selecting Block %d rc=%d\n", bp, rc));
        goto bail;
    }

    rc = pm8058_readb(SSBI_REG_ADDR_IRQ_IT_STATUS(REG_IRQ_BASE), ip);
    if (rc)
        DEBUG((EFI_D_ERROR, "Failed Reading Status rc=%d\n", rc));

bail:
    // Restore TPL to the previous level
    gBS->RestoreTPL(old_tpl);
    return rc;
}

static int pm8xxx_read_config_irq(UINT8 bp, UINT8 cp, UINT8 *r)
{
    int rc;
    EFI_TPL old_tpl;  // Variable to store the old TPL

    // Raise TPL to HIGH level
    old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    rc = pm8058_writeb(SSBI_REG_ADDR_IRQ_BLK_SEL(REG_IRQ_BASE), bp);
    if (rc) {
        DEBUG((EFI_D_ERROR, "Failed Selecting Block %d rc=%d\n", bp, rc));
        goto bail;
    }

    rc = pm8058_writeb(SSBI_REG_ADDR_IRQ_CONFIG(REG_IRQ_BASE), cp);
    if (rc)
        DEBUG((EFI_D_ERROR, "Failed Configuring IRQ rc=%d\n", rc));

    rc = pm8058_readb(SSBI_REG_ADDR_IRQ_CONFIG(REG_IRQ_BASE), r);
    if (rc)
        DEBUG((EFI_D_ERROR, "Failed reading IRQ rc=%d\n", rc));

bail:
    // Restore TPL to the previous level
    gBS->RestoreTPL(old_tpl);
    return rc;
}

static int pm8xxx_write_config_irq(UINT8 bp, UINT8 cp)
{
	int	rc;
    EFI_TPL old_tpl;  // Variable to store the old TPL

	old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
	rc = pm8058_writeb(			SSBI_REG_ADDR_IRQ_BLK_SEL(REG_IRQ_BASE), bp);
	if (rc) {
		DEBUG((EFI_D_ERROR, "Failed Selecting Block %d rc=%d\n", bp, rc));
		goto bail;
	}

	cp |= PM_IRQF_WRITE;
	rc = pm8058_writeb(			SSBI_REG_ADDR_IRQ_CONFIG(REG_IRQ_BASE), cp);
	if (rc)
		DEBUG((EFI_D_ERROR, "Failed Configuring IRQ rc=%d\n", rc));
bail:
	gBS->RestoreTPL(old_tpl);
	return rc;
}

static int pm8xxx_irq_block_handler(int block)
{
	int pmirq, irq, i, ret = 0;
	UINT8 bits;

	ret = pm8xxx_read_block_irq(block, &bits);
	if (ret) {
		DEBUG((EFI_D_ERROR, "Failed reading %d block ret=%d", block, ret));
		return ret;
	}
	if (!bits) {
		DEBUG((EFI_D_ERROR, "block bit set in master but no irqs: %d", block));
		return 0;
	}

	/* Check IRQ bits */
	for (i = 0; i < 8; i++) {
		if (bits & (1 << i)) {
			pmirq = block * 8 + i;
			irq = pmirq + PMIC8058_IRQ_BASE;
			//generic_handle_irq(irq);
		}
	}
	return 0;
}

static int pm8xxx_irq_master_handler(int master)
{
	UINT8 blockbits;
	int block_number, i, ret = 0;

	ret = pm8xxx_read_master_irq(master, &blockbits);
	if (ret) {
		DEBUG((EFI_D_ERROR, "Failed to read master %d ret=%d\n", master, ret));
		return ret;
	}
	if (!blockbits) {
		DEBUG((EFI_D_ERROR, "master bit set in root but no blocks: %d", master));
		return 0;
	}

	for (i = 0; i < 8; i++)
		if (blockbits & (1 << i)) {
			block_number = master * 8 + i;	/* block # */
			ret |= pm8xxx_irq_block_handler(block_number);
		}
	return ret;
}





int pm8xxx_get_irq_base(void)
{
	return PMIC8058_IRQ_BASE; //PMIC8058_IRQ_BASE
}

int pm8xxx_get_irq_stat(int irq)
{
    int pmirq, rc;
    UINT8 block, bits, bit;
    EFI_TPL old_tpl;  // Variable to store the old TPL

    if (irq < PMIC8058_IRQ_BASE ||irq >= PMIC8058_IRQ_BASE + PM8058_NR_IRQS)
        return -1;

    pmirq = irq - PMIC8058_IRQ_BASE; // irq - PMIC8058_IRQ_BASE;

    block = pmirq / 8;
    bit = pmirq % 8;

    // Raise TPL to HIGH level
    old_tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);

    rc = pm8058_writeb(SSBI_REG_ADDR_IRQ_BLK_SEL(REG_IRQ_BASE), block); //cant be sure that base is pmic base need to check, see commonvars.h for more info
    if (rc) {
        DEBUG((EFI_D_ERROR, "Failed Selecting block irq=%d pmirq=%d blk=%d rc=%d\n", irq, pmirq, block, rc));
        goto bail_out;
    }

    rc = pm8058_readb(SSBI_REG_ADDR_IRQ_RT_STATUS(REG_IRQ_BASE), &bits);
    if (rc) {
        DEBUG((EFI_D_ERROR, "Failed Configuring irq=%d pmirq=%d blk=%d rc=%d\n", irq, pmirq, block, rc));
        goto bail_out;
    }

    rc = (bits & (1 << bit)) ? 1 : 0;

bail_out:
    // Restore TPL to the previous level
    gBS->RestoreTPL(old_tpl);

    return rc;
}
