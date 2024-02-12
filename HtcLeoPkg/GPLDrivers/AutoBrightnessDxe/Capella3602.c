#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Device/microp.h>
#include <Chipset/timer.h>
#include <Protocol/HtcLeoMicroP.h>

#define LSENSOR_POLL_PROMESHUTOK   1000


static UINT16 lsensor_adc_table[10] = 
{
	0, 5, 20, 70, 150, 240, 330, 425, 515, 590
};

static struct lsensor_data 
{
	UINT32 old_level;
	INTN enabled;
	INTN opened;
} the_data;

// range: adc: 0..1023, value: 0..9
static void map_adc_to_level(UINT32 adc, UINT32 *value)
{
	INTN i;

	if (adc > 1024) {
		*value = 5; // set some good value at error
		return;
	}

	for (i = 9; i >= 0; i--) {
		if (adc >= lsensor_adc_table[i]) {
			*value = i;
			return;
		}
	}
	*value = 5;  // set some good value at error
}

INTN lightsensor_read_value(UINT32 *val)
{
	INTN ret;
	UINT8 data[2];

	if (!val)
		return -1;

	ret = microp_i2c_read(MICROP_I2C_RCMD_LSENSOR, data, 2);
	if (ret < 0) {
		return -1;
	}

	*val = data[1] | (data[0] << 8);
	
	return 0;
}

// static enum handler_return lightsensor_poll_function(struct timer *timer, time_t now, void *arg)
// {
// 	struct lsensor_data* p = &the_data;
// 	UINT32 adc = 0, level = 0;

// 	if (!p->enabled)
// 		goto error;
      
// 	lightsensor_read_value(&adc);
// 	map_adc_to_level(adc, &level);
// 	if (level != the_data.old_level) {
// 		the_data.old_level = level;
// 		htcleo_panel_set_brightness(level == 0 ? 1 : level);
// 	}
// 	timer_set_oneshot(timer, 500, lightsensor_poll_function, NULL);

// error:
// 	return INTN_RESCHEDULE;
// }


static INTN lightsensor_enable(void)
{
	struct lsensor_data* p = &the_data;
	INTN rc = -1;

	if (p->enabled)
		return 0;
	
	rc = capella_cm3602_power(LS_PWR_ON, 1);
	if (rc < 0)
		return -1;
	
	the_data.old_level = -1;
	p->enabled = 1;

	return 0;
}

static INTN lightsensor_disable(void)
{
	struct lsensor_data* p = &the_data;
	INTN rc = -1;

	if (!p->enabled)
		return 0;
	
	rc = capella_cm3602_power(LS_PWR_ON, 0);
	if (rc < 0)
		return -1;

	p->enabled = 0;
	//timer_cancel(&lsensor_poll_timer);
	
	return 0;
}


static INTN lightsensor_open(void *arg)
{
	INTN rc = 0;
	
	//mutex_acquire(&api_lock);
	
	if (the_data.opened)
		rc = -1;

	the_data.opened = 1;
	
	//mutex_release(&api_lock);
	
	return rc;
}

static INTN lightsensor_release(void *arg)
{
	//mutex_acquire(&api_lock);
	
	the_data.opened = 0;
	
	//mutex_release(&api_lock);
	
	return 0;
}

INTN htcleo_lsensor_probe(void)
{  
	INTN ret = -1;    
	
	the_data.old_level = -1;
	the_data.enabled=0;
	the_data.opened=0;
	
	ret = lightsensor_enable();
	if (ret) {
		DEBUG((EFI_D_INFO, "lightsensor_enable failed\n"));
		return -1;
	}

	// enter_critical_section();
	// timer_initialize(&lsensor_poll_timer);
	// timer_set_oneshot(&lsensor_poll_timer, 0, lightsensor_poll_function, NULL);
	// exit_critical_section();

    //toDo poll lightsensor here with a timer


	
	return ret;
}