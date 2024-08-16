#ifndef __PLATFORM_MSM7230_SSBI_HW_H
#define __PLATFORM_MSM7230_SSBI_HW_H


/* SSBI 2.0 controller registers */
#define SSBI2_CTL			0x0000
#define SSBI2_RESET			0x0004
#define SSBI2_CMD			0x0008
#define SSBI2_RD			0x0010
#define SSBI2_STATUS			0x0014
#define SSBI2_PRIORITIES		0x0018
#define SSBI2_MODE2			0x001C

/* SSBI_CMD fields */
#define SSBI_CMD_SEND_TERM_SYM		(1 << 27)
#define SSBI_CMD_WAKEUP_SLAVE		(1 << 26)
#define SSBI_CMD_USE_ENABLE		(1 << 25)
#define SSBI_CMD_RDWRN			(1 << 24)

/* SSBI_STATUS fields */
#define SSBI_STATUS_DATA_IN		(1 << 4)
#define SSBI_STATUS_RD_CLOBBERED	(1 << 3)
#define SSBI_STATUS_RD_READY		(1 << 2)
#define SSBI_STATUS_READY		(1 << 1)
#define SSBI_STATUS_MCHN_BUSY		(1 << 0)

/* SSBI_RD fields */
#define SSBI_RD_USE_ENABLE		(1 << 25)
#define SSBI_RD_RDWRN			(1 << 24)

/* SSBI_MODE2 fields */
#define SSBI_MODE2_SSBI2_MODE		(1 << 0)

#define SSBI_TIMEOUT_US			100
#endif