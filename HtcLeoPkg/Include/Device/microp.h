/*
 * Copyright (c) 2012, Shantanu Gupta <shans95g@gmail.com>
 * Based on the open source driver from HTC
 *
 */
 
#define MICROP_LSENSOR_ADC_CHAN				6
#define MICROP_REMOTE_KEY_ADC_CHAN			7

#define MICROP_I2C_ADDR						0x66
#define MICROP_I2C_WCMD_MISC				0x20
#define MICROP_I2C_WCMD_SPI_EN				0x21
#define MICROP_I2C_WCMD_LCM_BL_MANU_CTL		0x22
#define MICROP_I2C_WCMD_AUTO_BL_CTL			0x23
#define MICROP_I2C_RCMD_SPI_BL_STATUS		0x24
#define MICROP_I2C_WCMD_LED_PWM				0x25
#define MICROP_I2C_WCMD_BL_EN				0x26
#define MICROP_I2C_RCMD_VERSION				0x30
#define MICROP_I2C_RCMD_LSENSOR				0x33
#define MICROP_I2C_WCMD_ADC_TABLE			0x42
#define MICROP_I2C_WCMD_LED_CTRL			0x51
#define MICROP_I2C_WCMD_LED_MODE			0x53
#define MICROP_I2C_RCMD_GREEN_LED_REMAIN_TIME	0x54
#define MICROP_I2C_RCMD_AMBER_LED_REMAIN_TIME	0x55
#define MICROP_I2C_RCMD_LED_REMAIN_TIME			0x56
#define MICROP_I2C_RCMD_BLUE_LED_REMAIN_TIME	0x57
#define MICROP_I2C_RCMD_LED_STATUS			0x58
#define MICROP_I2C_WCMD_JOGBALL_LED_MODE	0x5A
#define MICROP_I2C_WCMD_JOGBALL_LED_PWM_SET	0x5C
#define MICROP_I2C_WCMD_READ_ADC_VALUE_REQ	0x60
#define MICROP_I2C_RCMD_ADC_VALUE			0x62
#define MICROP_I2C_WCMD_REMOTEKEY_TABLE		0x63
#define MICROP_I2C_WCMD_ADC_REQ				0x64
#define MICROP_I2C_WCMD_LCM_BURST			0x6A
#define MICROP_I2C_WCMD_LCM_BURST_EN		0x6B
#define MICROP_I2C_WCMD_LCM_REGISTER		0x70
#define MICROP_I2C_WCMD_GSENSOR_REG			0x73
#define MICROP_I2C_WCMD_GSENSOR_REG_DATA_REQ	0x74
#define MICROP_I2C_RCMD_GSENSOR_REG_DATA	0x75
#define MICROP_I2C_WCMD_GSENSOR_DATA_REQ	0x76
#define MICROP_I2C_RCMD_GSENSOR_X_DATA		0x77
#define MICROP_I2C_RCMD_GSENSOR_Y_DATA		0x78
#define MICROP_I2C_RCMD_GSENSOR_Z_DATA		0x79
#define MICROP_I2C_RCMD_GSENSOR_DATA		0x7A
#define MICROP_I2C_WCMD_OJ_REG				0x7B
#define MICROP_I2C_WCMD_OJ_REG_DATA_REQ		0x7C
#define MICROP_I2C_RCMD_OJ_REG_DATA			0x7D
#define MICROP_I2C_WCMD_OJ_POS_DATA_REQ		0x7E
#define MICROP_I2C_RCMD_OJ_POS_DATA			0x7F
#define MICROP_I2C_WCMD_GPI_INT_CTL_EN		0x80
#define MICROP_I2C_WCMD_GPI_INT_CTL_DIS		0x81
#define MICROP_I2C_RCMD_GPI_INT_STATUS		0x82
#define MICROP_I2C_RCMD_GPIO_STATUS			0x83
#define MICROP_I2C_WCMD_GPI_INT_STATUS_CLR	0x84
#define MICROP_I2C_RCMD_GPI_INT_SETTING		0x85
#define MICROP_I2C_RCMD_REMOTE_KEYCODE		0x87
#define MICROP_I2C_WCMD_REMOTE_KEY_DEBN_TIME	0x88
#define MICROP_I2C_WCMD_REMOTE_PLUG_DEBN_TIME	0x89
#define MICROP_I2C_WCMD_SIMCARD_DEBN_TIME	0x8A
#define MICROP_I2C_WCMD_GPO_LED_STATUS_EN	0x90
#define MICROP_I2C_WCMD_GPO_LED_STATUS_DIS	0x91
#define MICROP_I2C_RCMD_GPO_LED_STATUS		0x92
#define MICROP_I2C_WCMD_OJ_INT_STATUS		0xA8
#define MICROP_I2C_RCMD_MOBEAM_STATUS		0xB1
#define MICROP_I2C_WCMD_MOBEAM_DL			0xB2
#define MICROP_I2C_WCMD_MOBEAM_SEND			0xB3

#define IRQ_GSENSOR				(1<<10)
#define IRQ_LSENSOR  			(1<<09)
#define IRQ_REMOTEKEY			(1<<07)
#define IRQ_HEADSETIN			(1<<02)
#define IRQ_PROXIMITY   		(1<<01)
#define IRQ_SDCARD	    		(1<<00)

#define READ_GPI_STATE_HPIN		(1<<2)
#define READ_GPI_STATE_SDCARD	(1<<0)

#define PS_PWR_ON				(1<<0)
#define LS_PWR_ON				(1<<1)

// Specific defines from board_htcleo.h
#define HTCLEO_GPIO_UP_RESET_N		91

// Colors
#define LED_OFF   0
#define LED_GREEN 1
#define LED_AMBER 2
#define LS_PWR_ON				(1<<1)

struct microp_platform_data {
	int chip;
	UINT32 gpio_reset;
};
int capella_cm3602_power(UINT8 pwr_device, UINT8 enable);
int microp_i2c_read(UINT8 addr, UINT8 *data, int length);
int microp_i2c_write(UINT8 addr, UINT8 *data, int length);
void microp_i2c_probe(struct microp_platform_data *kpdata);
int microp_gpo_enable(UINT16 gpo_mask);
int microp_gpo_disable(UINT16 gpo_mask);
int microp_read_gpo_status(UINT16 *status);