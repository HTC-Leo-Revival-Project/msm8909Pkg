/*
 * Copyright (c) 2011, Shantanu Gupta <shans95g@gmail.com>
 */
#include <PiDxe.h>
#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Chipset/msm_i2c.h>
#include <Library/DS2746.h>
#include <Protocol/HtcLeoI2C.h>

// Cached copy of the i2c protocol
HTCLEO_I2C_PROTOCOL *gI2C = NULL;

uint32_t ds2746_voltage(uint8_t addr) {
	uint32_t vol;
	uint8_t s0, s1;

	gI2C->Read(addr, DS2746_VOLTAGE_MSB, &s0, 1);
	gI2C->Read(addr, DS2746_VOLTAGE_LSB, &s1, 1);

	vol = s0 << 8;
	vol |= s1;

	return ((vol >> 4) * DS2746_VOLTAGE_RES) / 1000;
}

/*
  //min,	max
	7150,	15500,	// Sony 1300mAh (Formosa)	7.1k ~ 15k
	27500,	49500,	// Sony 1300mAh (HTE)		28k  ~ 49.5k
	15500,	27500,	// Sanyo 1300mAh (HTE)		16k  ~ 27k
	100,	7150,	// Samsung 1230mAh			0.1k ~ 7k
	0,		100,	// HTC Extended 2300mAh		0k   ~ 0.1k
*/
/*
// The resistances in mOHM
const uint16_t rsns[] = {
	25,			// 25 mOHM
	20,			// 20 mOHM
	15,			// 15 mOHM
	10,			// 10 mOHM
	5,			// 5 mOHM
};
*/
int16_t ds2746_current(uint8_t addr, uint16_t resistance) {
	int16_t cur;
	int8_t s0, s1;

	gI2C->Read(addr, DS2746_CURRENT_MSB, &s0, 1);
	gI2C->Read(addr, DS2746_CURRENT_LSB, &s1, 1);

	cur = s0 << 8;
	cur |= s1;
	
	return (((cur >> 2) * DS2746_CURRENT_ACCUM_RES) / resistance);
}

int16_t ds2745_temperature(uint8_t addr) {
	int16_t temp;
	int8_t s0, s1;

	gI2C->Read(addr, DS2745_TEMPERATURE_MSB, &s0, 1);
	gI2C->Read(addr, DS2745_TEMPERATURE_LSB, &s1, 1);

	temp = s0 << 8;
	temp |= s1;
	
	return ((temp >> 5) * DS2745_TEMPERATURE_RES);
}

RETURN_STATUS
EFIAPI
DS2746LibConstructor(VOID)
{
  EFI_STATUS           Status = EFI_SUCCESS;

  // Find the i2c protocol.  ASSERT if not found.
  Status = gBS->LocateProtocol (&gHtcLeoI2CProtocolGuid, NULL, (VOID **)&gI2C);
  ASSERT_EFI_ERROR (Status);

  return Status;
}