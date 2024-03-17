#include <Base.h>
#include <Library/LKEnvLib.h>
//#include <Library/QcomClockLib.h>
#include <Library/QcomBoardLib.h>
//#include <Library/QcomGpioTlmmLib.h>
#include <Library/QcomPlatformUsbLib.h>
#include <Target/board.h>

#include <Library/pcom.h>
#include <Library/pcom_clients.h>

/*
static struct pda_power_supply htcleo_power_supply = {
	.is_ac_online = &htcleo_ac_online,
	.is_usb_online = &htcleo_usb_online,
	.charger_state = &htcleo_charger_state,
	.set_charger_state = &htcleo_set_charger,
	.want_charging = &htcleo_want_charging,
};

static struct msm_hsusb_pdata htcleo_hsusb_pdata = {
	.set_ulpi_state = NULL,
	.power_supply = &htcleo_power_supply,
};
*/

/*
static struct udc_device htcleo_udc_device[] = {
	// Testing stuff...
	{
		.vendor_id = 0x18d1,
		.product_id = 0xD00D,
		.version_id = 0x0100,
		.manufacturer = "HTC Corporation",
		.product = "HD2",
	},
	{
		.vendor_id = 0x18d1,
		.product_id = 0x0D02,
		.version_id = 0x0001,
		.manufacturer = "Google",
		.product = "Android",
	},
	{
		.vendor_id = 0x0bb4,
		.product_id = 0x0C02,
		.version_id = 0x0100,
		.manufacturer = "HTC",
		.product = "Android Phone",
	},
	{
		.vendor_id = 0x05c6,
		.product_id = 0x9026,
		.version_id = 0x0100,
		.manufacturer = "Qualcomm Incorporated",
		.product = "Qualcomm HSUSB Device",
	},
};

void htcleo_udc_init(void)
{
	udc_init(&htcleo_udc_device[device_info.udc]);
}
*/

/* Do target specific usb initialization */
STATIC VOID target_usb_init(target_usb_iface_t *iface)
{
  //msm_hsusb_init(&htcleo_hsusb_pdata);
}

STATIC VOID hsusb_clock_init(target_usb_iface_t *iface)
{
  pcom_enable_hsusb_clk();
}

EFI_STATUS LibQcomPlatformUsbGetInterface(target_usb_iface_t *iface)
{
  iface->controller = L"ci";
  iface->usb_init   = target_usb_init;
  iface->clock_init = hsusb_clock_init;

  return EFI_SUCCESS;
}

