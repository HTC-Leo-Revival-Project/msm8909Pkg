#ifndef __DEV_GPIO_KEYPAD_H
#define __DEV_GPIO_KEYPAD_H


#define GPIOKPF_ACTIVE_HIGH		(1U << 0)
#define GPIOKPF_DRIVE_INACTIVE		(1U << 1)

typedef enum {
  KEY_VOLUMEUP,
  KEY_VOLUMEDOWN,
  KEY_SOFT1,
  KEY_SEND,
  KEY_CLEAR,
  KEY_BACK,
  KEY_HOME,
} HTCLEO_BUTTON_TYPE;

typedef unsigned long time_t;

struct gpio_keypad_info {
	/* size must be ninputs * noutputs */
	const HTCLEO_BUTTON_TYPE *keymap;
	unsigned *input_gpios;
	unsigned *output_gpios;
	int ninputs;
	int noutputs;
	/* time to wait before reading inputs after driving each output */
	time_t settle_time;
	time_t poll_time;
	unsigned flags;
	/*
	 * Can be used by board to be notified without using extra resources
	 * Used for key backlight, power off etc..
	 */
	//void (*notify_fn)(unsigned key_code, unsigned state);
};

#endif