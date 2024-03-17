#ifndef __MSM_I2C_H__
#define __MSM_I2C_H__

#include <Library/LKEnvLib.h>

#define MSM_I2C_READ_RETRY_TIMES 16
#define MSM_I2C_WRITE_RETRY_TIMES 16

#define I2C_M_TEN           0x0010  /* this is a ten bit chip address */
#define I2C_M_RD            0x0001  /* read data, from slave to master */
#define I2C_M_NOSTART       0x4000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR  0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK    0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK     0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN      0x0400  /* length will be first received byte */

struct i2c_msg {
	uint16_t addr;	// slave address
	uint16_t flags;
	uint16_t len;	// msg length
	uint8_t *buf;	// pointer to msg data
};

struct msm_i2c_pdata {
	int i2c_clock;
	int clk_nr;
	int irq_nr;
	int scl_gpio;
	int sda_gpio;
	void (*set_mux_to_i2c)(int mux_to_i2c);
	void *i2c_base;
};

struct msm_i2c_dev {
	struct i2c_msg *msg;
	struct msm_i2c_pdata *pdata;
	int rem;
	int pos;
	int cnt;
	int ret;
	bool need_flush;
	int flush_cnt;
};

int msm_i2c_probe(struct msm_i2c_pdata*);
void msm_i2c_remove(void);
int msm_i2c_write(int chip, void *buf, size_t count);
int msm_i2c_read(int chip, uint8_t reg, void *buf, size_t count);
int msm_i2c_xfer(struct i2c_msg msgs[], int num);

#endif //__MSM_I2C_H__
