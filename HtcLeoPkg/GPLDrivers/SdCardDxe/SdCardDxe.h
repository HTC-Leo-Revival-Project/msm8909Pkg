// driver configs
//#define USE_PROC_COMM
//#define USE_DM
//#define USE_HIGH_SPEED_MODE

#include <Library/UbootEnvLib.h>
// must come in order
#include <Library/part.h>
#include <Library/mmc.h>
#include <Library/adm.h>
#ifdef USE_PROC_COMM
#include <Library/pcom_clients.h>
#endif /*USE_PROC_COMM */

#include <Library/gpio.h>
#include <Chipset/iomap.h>
#include <Library/reg.h>
#include <Chipset/gpio.h>

#define BLOCK_SIZE 512
#define SDC_INSTANCE 2

// Function prototypes
block_dev_desc_t *mmc_get_dev();

int mmc_legacy_init(int verbose);