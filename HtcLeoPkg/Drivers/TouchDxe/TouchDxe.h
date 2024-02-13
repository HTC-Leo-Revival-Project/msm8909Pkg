#define TOUCH_TYPE_UNKNOWN  0
#define TOUCH_TYPE_B8       1
#define TOUCH_TYPE_68       2
#define TOUCH_TYPE_2A       3

#define TYPE_2A_DEVID	(0x2A >> 1)
#define TYPE_B8_DEVID	(0xB8 >> 1)
#define TYPE_68_DEVID	(0x68 >> 1)

#define MAKEWORD(a, b)  ((uint16_t)(((uint8_t)(a)) | ((uint16_t)((uint8_t)(b))) << 8))

//global variables
int ts_type;
int retry;