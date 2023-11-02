/* Port of msm-i2c.c to UEFI
 *
 * LK port: Copyright (C) 2011 Alexander Tarasikov <alexander.tarasikov@gmail.com>
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <Library/ArmLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Library/pcom.h>
#include <Library/gpio.h>
#include <Library/InterruptsLib.h>
#include <Library/LKEnvLib.h>
#include <Library/reg.h>
#include <Device/Gpio.h>

#include <Chipset/msm_i2c.h>
#include <Chipset/iomap.h>
#include <Chipset/timer.h>
#include <Chipset/clock.h>

#include <Protocol/HardwareInterrupt.h>
#include <Protocol/EmbeddedClock.h>
#include <Protocol/HtcLeoI2C.h>

#include <Chipset/gpio.h>
#include <Chipset/irqs.h>
#include <Protocol/GpioTlmm.h>

// Cached copy of the Hardware Gpio protocol instance
TLMM_GPIO *gGpio = NULL;
EMBEDDED_CLOCK_PROTOCOL  *gClock = NULL;
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

#define DEBUG_I2C 0

#if DEBUG_I2C
	#define I2C_DBG(lvl, fmt, x...) dprintf(lvl, fmt, ##x)
	#define I2C_ERR(fmt, x...) 		dprintf(ALWAYS, fmt, ##x)
	#define I2C_DBG_FUNC_LINE() 	dprintf(ALWAYS, "%s: %d\n", __func__, __LINE__)
#else
	#define I2C_DBG(lvl, fmt, x...)
	#define I2C_ERR(fmt, x...)
	#define I2C_DBG_FUNC_LINE()
#endif

enum {
	I2C_WRITE_DATA 			= 0x00,
	I2C_CLK_CTL 			= 0x04,
	I2C_STATUS 				= 0x08,
	I2C_READ_DATA 			= 0x0c,
	I2C_INTERFACE_SELECT 	= 0x10,

	I2C_WRITE_DATA_DATA_BYTE 			= 0xff,
	I2C_WRITE_DATA_ADDR_BYTE 			= 1U << 8,
	I2C_WRITE_DATA_LAST_BYTE			= 1U << 9,

	I2C_CLK_CTL_FS_DIVIDER_VALUE 		= 0xff,
	I2C_CLK_CTL_HS_DIVIDER_VALUE 		= 7U << 8,

	I2C_STATUS_WR_BUFFER_FULL 			= 1U << 0,
	I2C_STATUS_RD_BUFFER_FULL 			= 1U << 1,
	I2C_STATUS_BUS_ERROR 				= 1U << 2,
	I2C_STATUS_PACKET_NACKED 			= 1U << 3,
	I2C_STATUS_ARB_LOST 				= 1U << 4,
	I2C_STATUS_INVALID_WRITE 			= 1U << 5,
	I2C_STATUS_FAILED 					= 3U << 6,
	I2C_STATUS_BUS_ACTIVE 				= 1U << 8,
	I2C_STATUS_BUS_MASTER 				= 1U << 9,
	I2C_STATUS_ERROR_MASK 				= 0xfc,

	I2C_INTERFACE_SELECT_INTF_SELECT	= 1U << 0,
	I2C_INTERFACE_SELECT_SCL 			= 1U << 8,
	I2C_INTERFACE_SELECT_SDA 			= 1U << 9,
};

static struct msm_i2c_dev dev;
static int timeout;

#if DEBUG_I2C
static void dump_status(uint32_t status)
{
	I2C_DBG(DEBUGLEVEL, "STATUS (0x%.8x): ", status);
	if (status & I2C_STATUS_BUS_MASTER)
		I2C_DBG(DEBUGLEVEL, "MST ");
	if (status & I2C_STATUS_BUS_ACTIVE)
		I2C_DBG(DEBUGLEVEL, "ACT ");
	if (status & I2C_STATUS_INVALID_WRITE)
		I2C_DBG(DEBUGLEVEL, "INV_WR ");
	if (status & I2C_STATUS_ARB_LOST)
		I2C_DBG(DEBUGLEVEL, "ARB_LST ");
	if (status & I2C_STATUS_PACKET_NACKED)
		I2C_DBG(DEBUGLEVEL, "NAK ");
	if (status & I2C_STATUS_BUS_ERROR)
		I2C_DBG(DEBUGLEVEL, "BUS_ERR ");
	if (status & I2C_STATUS_RD_BUFFER_FULL)
		I2C_DBG(DEBUGLEVEL, "RD_FULL ");
	if (status & I2C_STATUS_WR_BUFFER_FULL)
		I2C_DBG(DEBUGLEVEL, "WR_FULL ");
	if (status & I2C_STATUS_FAILED)
		I2C_DBG(DEBUGLEVEL, "FAIL 0x%x", (status & I2C_STATUS_FAILED));
	I2C_DBG(DEBUGLEVEL, "\n");
}
#endif

static void msm_i2c_write_delay(void)
{
	/* If scl is still high we have >4us (for 100kbps) to write the data
	 * register before we risk hitting a bug where the controller releases
	 * scl to soon after driving sda low. Writing the data after the
	 * scheduled release time for scl also avoids the bug.
	 */
	if (readl(dev.pdata->i2c_base + I2C_INTERFACE_SELECT) & I2C_INTERFACE_SELECT_SCL)
		return;
		
	NanoSecondDelay(6);
}

static bool msm_i2c_fill_write_buffer(void)
{
	uint16_t val;
	if (dev.pos < 0) {
		val = I2C_WRITE_DATA_ADDR_BYTE | dev.msg->addr << 1;
		if (dev.msg->flags & I2C_M_RD)
			val |= 1;
			
		if (dev.rem == 1 && dev.msg->len == 0)
			val |= I2C_WRITE_DATA_LAST_BYTE;
			
		msm_i2c_write_delay();
		writel(val, dev.pdata->i2c_base + I2C_WRITE_DATA);
		dev.pos++;
		
		return true;
	}

	if (dev.msg->flags & I2C_M_RD)
		return false;

	if (!dev.cnt)
		return false;

	/* Ready to take a byte */
	val = dev.msg->buf[dev.pos];
	if (dev.cnt == 1 && dev.rem == 1)
		val |= I2C_WRITE_DATA_LAST_BYTE;

	msm_i2c_write_delay();
	writel(val, dev.pdata->i2c_base + I2C_WRITE_DATA);
	dev.pos++;
	dev.cnt--;
	
	return true;
}

static void msm_i2c_read_buffer(void)
{
	/*
	 * Theres something in the FIFO.
	 * Are we expecting data or flush crap?
	 */
	I2C_DBG_FUNC_LINE();
	if ((dev.msg->flags & I2C_M_RD) && dev.pos >= 0 && dev.cnt) {
		switch (dev.cnt) {
			case 1:
				if (dev.pos != 0)
					break;
				dev.need_flush = true;
				/* fall-trough */
			case 2:
				writel(I2C_WRITE_DATA_LAST_BYTE, dev.pdata->i2c_base + I2C_WRITE_DATA);
		}
		dev.msg->buf[dev.pos] = readl(dev.pdata->i2c_base + I2C_READ_DATA);
		dev.cnt--;
		dev.pos++;
	} else { /* FLUSH */
		if (dev.flush_cnt & 1) {
			/*
			* Stop requests are sometimes ignored, but writing
			* more than one request generates a write error.
			*/
			writel(I2C_WRITE_DATA_LAST_BYTE, dev.pdata->i2c_base + I2C_WRITE_DATA);
		}
		
		readl(dev.pdata->i2c_base + I2C_READ_DATA);
		
		if (dev.need_flush)
			dev.need_flush = false;
		else
			dev.flush_cnt++;
	}
}

static void msm_i2c_interrupt_locked(void)
{
	uint32_t status	= readl(dev.pdata->i2c_base + I2C_STATUS);
	bool not_done = true;

#if DEBUG_I2C
	dump_status(status);
#endif
	if (!dev.msg) {
		I2C_DBG(DEBUGLEVEL, "IRQ but nothing to do!, status %x\n", status);
		return;
	}
	
	if (status & I2C_STATUS_ERROR_MASK)
		goto out_err;

	if (!(status & I2C_STATUS_WR_BUFFER_FULL))
		not_done = msm_i2c_fill_write_buffer();
		
	if (status & I2C_STATUS_RD_BUFFER_FULL)
		msm_i2c_read_buffer();

	if (dev.pos >= 0 && dev.cnt == 0) {
		if (dev.rem > 1) {
			dev.rem--;
			dev.msg++;
			dev.pos = -1;
			dev.cnt = dev.msg->len;
		}
		else if (!not_done && !dev.need_flush) {
			timeout = 0;
			return;
		}
	}
	return;

out_err:
	I2C_ERR("error, status %x\n", status);
	dev.ret = ERROR;
	timeout = ERR_TIMED_OUT;
}

VOID
EFIAPI
I2CInterruptHandler (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
  )
{
	msm_i2c_interrupt_locked();
}

static int msm_i2c_poll_notbusy(int warn)
{
	uint32_t retries = 0;
	while (retries != 200) {
		uint32_t status = readl(dev.pdata->i2c_base + I2C_STATUS);

		if (!(status & I2C_STATUS_BUS_ACTIVE)) {
			if (retries && warn){
				I2C_DBG(DEBUGLEVEL, "Warning bus was busy (%d)\n", retries);
				return 0;
			}
		}
		
		if (retries++ > 100)
			MicroSecondDelay(10);
	}
	
	I2C_ERR("Error waiting for notbusy\n");
	return ERR_TIMED_OUT;
}

static int msm_i2c_recover_bus_busy(void)
{
	int i;
	bool gpio_clk_status = false;
	uint32_t status = readl(dev.pdata->i2c_base + I2C_STATUS);
	I2C_DBG_FUNC_LINE();

	if (!(status & (I2C_STATUS_BUS_ACTIVE | I2C_STATUS_WR_BUFFER_FULL)))
		return 0;

	if (dev.pdata->set_mux_to_i2c)
		dev.pdata->set_mux_to_i2c(0);

	if (status & I2C_STATUS_RD_BUFFER_FULL) {
		I2C_DBG(DEBUGLEVEL, "Read buffer full, status %x, intf %x\n",  status, readl(dev.pdata->i2c_base + I2C_INTERFACE_SELECT));
		writel(I2C_WRITE_DATA_LAST_BYTE, dev.pdata->i2c_base + I2C_WRITE_DATA);
		readl(dev.pdata->i2c_base + I2C_READ_DATA);
	}
	else if (status & I2C_STATUS_BUS_MASTER) {
		I2C_DBG(DEBUGLEVEL, "Still the bus master, status %x, intf %x\n", status, readl(dev.pdata->i2c_base + I2C_INTERFACE_SELECT));
		writel(I2C_WRITE_DATA_LAST_BYTE | 0xff, dev.pdata->i2c_base + I2C_WRITE_DATA);
	}

	I2C_DBG(DEBUGLEVEL, "i2c_scl: %d, i2c_sda: %d\n", gGpio->Get(dev.pdata->scl_gpio), gGpio->Get(dev.pdata->sda_gpio));

	for (i = 0; i < 9; i++) {
		if (gGpio->Get(dev.pdata->sda_gpio) && gpio_clk_status)
			break;
			
		gGpio->Set(dev.pdata->scl_gpio, 0);
		NanoSecondDelay(5);
		
		gGpio->Set(dev.pdata->sda_gpio, 0);
		NanoSecondDelay(5);
		
		gGpio->Config(dev.pdata->scl_gpio, GPIO_INPUT);
		NanoSecondDelay(5);
		
		if (!gGpio->Get(dev.pdata->scl_gpio))
			NanoSecondDelay(20);
			
		if (!gGpio->Get(dev.pdata->scl_gpio))
			MicroSecondDelay(10);
			
		gpio_clk_status = gGpio->Get(dev.pdata->scl_gpio);
		gGpio->Config(dev.pdata->sda_gpio, GPIO_INPUT);
		NanoSecondDelay(5);
	}
	
	if (dev.pdata->set_mux_to_i2c)
		dev.pdata->set_mux_to_i2c(1);

	NanoSecondDelay(10);

	status = readl(dev.pdata->i2c_base + I2C_STATUS);
	if (!(status & I2C_STATUS_BUS_ACTIVE)) {
		I2C_DBG(DEBUGLEVEL, "Bus busy cleared after %d clock cycles, status %x, intf %x\n", i, status, readl(dev.pdata->i2c_base + I2C_INTERFACE_SELECT));
		return 0;
	}
	
	I2C_DBG(DEBUGLEVEL, "Bus still busy, status %x, intf %x\n", status, readl(dev.pdata->i2c_base + I2C_INTERFACE_SELECT));
	return ERR_NOT_READY;
}

int msm_i2c_xfer(struct i2c_msg msgs[], int num)
{
	int ret, ret_wait;

	gClock->ClkEnable(dev.pdata->clk_nr);
	gInterrupt->EnableInterruptSource(gInterrupt, dev.pdata->irq_nr);

	ret = msm_i2c_poll_notbusy(1);

	if (ret) {
		ret = msm_i2c_recover_bus_busy();
		if (ret)
			goto err;
	}

	if (dev.flush_cnt) {
		I2C_DBG(DEBUGLEVEL, "%d unrequested bytes read\n", dev.flush_cnt);
	}
	dev.msg = msgs;
	dev.rem = num;
	dev.pos = -1;
	dev.ret = num;
	dev.need_flush = false;
	dev.flush_cnt = 0;
	dev.cnt = msgs->len;
	msm_i2c_interrupt_locked();

	/*
	 * Now that we've setup the xfer, the ISR will transfer the data
	 * and wake us up with dev.err set if there was an error
	 */
	ret_wait = msm_i2c_poll_notbusy(0); /* Read may not have stopped in time */

	if (dev.flush_cnt) {
		I2C_DBG(DEBUGLEVEL, "%d unrequested bytes read\n", dev.flush_cnt);
	}
	ret = dev.ret;
	dev.msg = NULL;
	dev.rem = 0;
	dev.pos = 0;
	dev.ret = 0;
	dev.flush_cnt = 0;
	dev.cnt = 0;

	if (ret_wait) {
		I2C_DBG(DEBUGLEVEL, "Still busy after xfer completion\n");
		ret_wait = msm_i2c_recover_bus_busy();
		if (ret_wait)
			goto err;
	}
	if (timeout == ERR_TIMED_OUT) {
		I2C_DBG(DEBUGLEVEL, "Transaction timed out\n");
		ret = ERR_TIMED_OUT;
	}
	if (ret < 0) {
		I2C_ERR("Error during data xfer (%d)\n", ret);
		msm_i2c_recover_bus_busy();
	}
	/* if (timeout == ERR_TIMED_OUT) {
		I2C_DBG(DEBUGLEVEL, "Transaction timed out\n");
		ret = 2;
		msm_i2c_recover_bus_busy();
	}
	if (timeout == ERR_TIMED_OUT) {
		I2C_DBG(DEBUGLEVEL, "Transaction timed out\n");
		ret = ERR_TIMED_OUT;
	}
	if (ret < 0) {
		I2C_ERR("Error during data xfer (%d)\n", ret);
		msm_i2c_recover_bus_busy();
	} */
err:
	gInterrupt->DisableInterruptSource(gInterrupt, dev.pdata->irq_nr);
	gClock->ClkDisable(dev.pdata->clk_nr);
	
	return ret;
}

//struct mutex msm_i2c_rw_mutex;
int msm_i2c_write(int chip, void *buf, UINTN count)
{
	int rc = ERR_NOT_READY;
	if (!dev.pdata) {
		I2C_DBG(ALWAYS, "[MSM-I2C]: %s: called when driver is not installed\n", __func__);
		return rc;
	}

	struct i2c_msg msg[] = {
		{.addr = chip,	.flags = 0,		.len = count,	.buf = buf,}
	};
	
	int retry;
	for (retry = 0; retry <= MSM_I2C_WRITE_RETRY_TIMES; retry++) {
		if (msm_i2c_xfer(msg, 1) == 1) {
			rc = 0;
			break;
		}
		MicroSecondDelay(10);
		I2C_DBG(DEBUGLEVEL, "%s, i2c write retry\n", __func__);
	}

	return rc;
}

int msm_i2c_read(int chip, UINT8 reg, void *buf, UINTN count)
{
	int rc = ERR_NOT_READY;
	if (!dev.pdata) {
		I2C_DBG(ALWAYS, "[MSM-I2C]: %s: called when driver is not installed\n", __func__);
		return rc;
	}

	struct i2c_msg msgs[] = {
		{.addr = chip,	.flags = 0,			.len = 1,		.buf = &reg,},
		{.addr = chip,	.flags = I2C_M_RD,	.len = count,	.buf = buf, },
	};
	
	int retry;
	for (retry = 0; retry <= MSM_I2C_READ_RETRY_TIMES; retry++) {
		if (msm_i2c_xfer(msgs, 2) == 2) {
			rc = 0;
			break;
		}
		MicroSecondDelay(10);
		I2C_DBG(DEBUGLEVEL, "%s, i2c read retry\n", __func__);
	}
	
	return rc;
}

int msm_i2c_probe(struct msm_i2c_pdata* pdata)
{
	if (dev.pdata) {
		I2C_DBG(ALWAYS, "[MSM-I2C]: already installed\n");
		return -1;
	}
	if (!pdata->set_mux_to_i2c)
		return -1;

	dev.pdata = pdata;
	
	gInterrupt->DisableInterruptSource(gInterrupt, dev.pdata->irq_nr);
	dev.pdata->set_mux_to_i2c(0);
	gClock->ClkEnable(dev.pdata->clk_nr);
	
	int i2c_clk 	= 19200000;
	int target_clk 	= 100000;
	if (dev.pdata->i2c_clock) {
		if (dev.pdata->i2c_clock < 100000 || dev.pdata->i2c_clock > 400000)
			target_clk = 100000;
		else
			target_clk = dev.pdata->i2c_clock;
	}
	int fs_div = ((i2c_clk / target_clk) / 2) - 3;
	int hs_div = 3;
	int clk_ctl = ((hs_div & 0x7) << 8) | (fs_div & 0xff);
	writel(clk_ctl, dev.pdata->i2c_base + I2C_CLK_CTL);
	I2C_DBG(DEBUGLEVEL, "msm_i2c_probe: clk_ctl %x, %d Hz\n", clk_ctl, i2c_clk / (2 * ((clk_ctl & 0xff) + 3)));

	gClock->ClkDisable(dev.pdata->clk_nr);
	gInterrupt->RegisterInterruptSource(gInterrupt, dev.pdata->irq_nr, I2CInterruptHandler);

	return 0;
}

void msm_i2c_remove() {
	if (!dev.pdata)
	{
		return;//if driver is not installed
	}

	gInterrupt->DisableInterruptSource(gInterrupt, dev.pdata->irq_nr);
	gClock->ClkDisable(dev.pdata->clk_nr);
	dev.pdata->set_mux_to_i2c(0);
	dev.pdata = NULL;
}

void
msm_set_i2c_mux(int mux_to_i2c) {
	if (mux_to_i2c) {
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
	} else {
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
	}
}

static struct msm_i2c_pdata i2c_pdata = {
	.i2c_clock = 400000,
	.clk_nr	= I2C_CLK,
	.irq_nr = INT_PWB_I2C,
	.scl_gpio = GPIO_I2C_CLK,
	.sda_gpio = GPIO_I2C_DAT,
	.set_mux_to_i2c = &msm_set_i2c_mux,
	.i2c_base = (void*)MSM_I2C_BASE,
};

RETURN_STATUS
EFIAPI
MsmI2cInitialize(VOID)
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Find the interrupt controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  ASSERT_EFI_ERROR (Status);

  // Find the clock controller protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gEmbeddedClockProtocolGuid, NULL, (VOID **)&gClock);
  ASSERT_EFI_ERROR (Status);

  msm_i2c_probe(&i2c_pdata);

  return Status;
}

HTCLEO_I2C_PROTOCOL gHtcLeoI2CProtocol = {
  msm_i2c_write,
  msm_i2c_read,
  msm_i2c_xfer
};

EFI_STATUS
EFIAPI
I2CDxeInitialize(
	IN EFI_HANDLE         ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS  Status = EFI_SUCCESS;
	EFI_HANDLE Handle = NULL;

	// Find the interrupt controller protocol.  ASSERT if not found.
  	Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  	ASSERT_EFI_ERROR (Status);

  	// Find the clock controller protocol.  ASSERT if not found.
  	Status = gBS->LocateProtocol (&gEmbeddedClockProtocolGuid, NULL, (VOID **)&gClock);
  	ASSERT_EFI_ERROR (Status);

	// Find the gpio controller protocol.  ASSERT if not found.
    Status = gBS->LocateProtocol (&gTlmmGpioProtocolGuid, NULL, (VOID **)&gGpio);
    ASSERT_EFI_ERROR (Status);

  	msm_i2c_probe(&i2c_pdata);

	Status = gBS->InstallMultipleProtocolInterfaces(
	&Handle, &gHtcLeoI2CProtocolGuid, &gHtcLeoI2CProtocol, NULL);
	ASSERT_EFI_ERROR(Status);

	return Status;
}