/*
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DS2746_H__
#define __DS2746_H__

#define DS2746_DATA_SIZE			0x12
#define DS2746_STATUS_REG			0x01
#define DS2746_AUX0_MSB				0x08
#define DS2746_AUX0_LSB				0x09
#define DS2746_AUX1_MSB				0x0a
#define DS2746_AUX1_LSB				0x0b
#define DS2745_TEMPERATURE_MSB		0x0a // aux1 on 2746 is temperature on 2745
#define DS2745_TEMPERATURE_LSB		0x0b
#define DS2746_VOLTAGE_MSB			0x0c
#define DS2746_VOLTAGE_LSB			0x0d
#define DS2746_CURRENT_MSB			0x0e
#define DS2746_CURRENT_LSB			0x0f
#define DS2746_CURRENT_ACCUM_MSB	0x10
#define DS2746_CURRENT_ACCUM_LSB	0x11
#define DS2746_OFFSET_BIAS			0x61
#define DS2746_ACCUM_BIAS			0x62

#define DS2746_DEFAULT_RSNS				1500
#define DS2746_DEFAULT_BATTERY_RATING	1500
#define DS2746_HIGH_VOLTAGE				4200 // mV
#define DS2746_LOW_VOLTAGE				3300

#define DS2746_CURRENT_ACCUM_RES	645
#define DS2746_VOLTAGE_RES			2440
#define DS2745_CURRENT_ACCUM_RES	1562500
#define DS2745_TEMPERATURE_RES		125

UINT32 ds2746_voltage(UINT8 addr);
INT16  ds2746_current(UINT8 addr, UINT16 resistance);
INT16  ds2745_temperature(UINT8 addr);

#endif //__DS2746_H__