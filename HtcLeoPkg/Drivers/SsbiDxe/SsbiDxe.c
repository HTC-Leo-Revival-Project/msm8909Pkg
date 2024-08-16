#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/Ssbi.h>

#include <Chipset/ssbi_msm7230.h>



 UINT32 ssbi_readl(unsigned offset)
{
	return MmioRead32(SSBI_REG(offset));
}

void ssbi_writel(unsigned val,unsigned offset)
{
	MmioWrite32(SSBI_REG(offset),val);
}

UINT32 ssbi_wait_mask(UINT32 set_mask, UINT32 clr_mask)
{
	UINT32 timeout = SSBI_TIMEOUT_US;
	UINT32 val;

	while (timeout--) {
		val = ssbi_readl(SSBI2_STATUS);
		if (((val & set_mask) == set_mask) && ((val & clr_mask) == 0))
			return 0;
		//NanoSecondDelay(1);
	}
	DEBUG((EFI_D_ERROR, "%s: timeout (status %x set_mask %x clr_mask %x)\n", __func__, ssbi_readl(SSBI2_STATUS), set_mask, clr_mask));
	return -1;
}

UINT32 msm_ssbi_read(UINT16 addr, UINT8 *buf, UINT32 len)
{
	unsigned long flags;
	UINT32 read_cmd = SSBI_CMD_RDWRN | ((addr & 0xff) << 16);
	UINT32 mode2;
	UINT32 ret = 0;

	//remote_spin_lock_irqsave(&>rspin_lock, flags);

	mode2 = ssbi_readl(SSBI2_MODE2);
	if (mode2 & SSBI_MODE2_SSBI2_MODE) {
		mode2 = (mode2 & 0xf) | (((addr >> 8) & 0x7f) << 4);
		ssbi_writel( mode2, SSBI2_MODE2);
	}

	while (len) {
		ret = ssbi_wait_mask(SSBI_STATUS_READY, 0);
		if (ret)
			goto err;

		ssbi_writel(read_cmd, SSBI2_CMD);
		ret = ssbi_wait_mask(SSBI_STATUS_RD_READY, 0);
		if (ret)
			goto err;
		*buf++ = ssbi_readl(SSBI2_RD) & 0xff;
		len--;
	}

err:
	//remote_spin_unlock_irqrestore(&ssbi->rspin_lock, flags);
	return ret;
}

UINT32 msm_ssbi_write(UINT16 addr, UINT8 *buf, UINT32 len)
{
	unsigned long flags;
	UINT32 mode2;
	UINT32 ret = 0;

	//remote_spin_lock_irqsave(&ssbi->rspin_lock, flags);

	mode2 = ssbi_readl(SSBI2_MODE2);
	if (mode2 & SSBI_MODE2_SSBI2_MODE) {
		mode2 = (mode2 & 0xf) | (((addr >> 8) & 0x7f) << 4);
		ssbi_writel(mode2, SSBI2_MODE2);
	}

	while (len) {
		ret = ssbi_wait_mask(SSBI_STATUS_READY, 0);
		if (ret)
			goto err;

		ssbi_writel(((addr & 0xff) << 16) | *buf, SSBI2_CMD);
		ret = ssbi_wait_mask(0, SSBI_STATUS_MCHN_BUSY);
		if (ret)
			goto err;
		buf++;
		len--;
	}

err:
	//remote_spin_unlock_irqrestore(&ssbi->rspin_lock, flags);
	return ret;
}

SSBI_PROTOCOL gSsbiProtocol = {
  msm_ssbi_read,
  msm_ssbi_write
};

EFI_STATUS
EFIAPI
SsbiDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
  EFI_STATUS  Status = EFI_SUCCESS;
  EFI_HANDLE  Handle = NULL;

  Status = gBS->InstallMultipleProtocolInterfaces(
  &Handle, &gSsbiProtocolGuid, &gSsbiProtocol, NULL);
  ASSERT_EFI_ERROR(Status);


	return Status;
}