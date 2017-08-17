/*
 * dell_led.c - Dell lightLED Driver
 *
 * Copyright (C) 2010 Dell Inc.
 * Foggy liu<chang_liu4@dell.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 */

#include <linux/acpi.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/dell-led.h>
#include <linux/platform_device.h>


MODULE_AUTHOR("Foggy_Liu");
MODULE_DESCRIPTION("Dell LED Control Driver");
MODULE_LICENSE("GPL");


#define WMAX_CONTROL_GUID		"A70591CE-A997-11DA-B012-B622A1EF5492"

MODULE_ALIAS("wmi:" WMAX_CONTROL_GUID);



#define WMAX_METHOD_BRIGHTNESS		0x3
#define WMAX_METHOD_ZONE_CONTROL	0x4

#define LED_MASK 0x00000002
#define APPLY_STATUS 0x0000



/* Error Result Codes: */
#define INVALID_DEVICE_ID	250
#define INVALID_PARAMETER	251
#define INVALID_BUFFER		252
#define INTERFACE_ERROR		253
#define UNSUPPORTED_COMMAND	254
#define UNSPECIFIED_ERROR	255

/* Device ID */
#define DEVICE_ID_PANEL_BACK	1



#define CMD_LED_ON	0x00FF
#define CMD_LED_OFF	0x0000


#define GLOBAL_MIC_MUTE_ENABLE	0x364
#define GLOBAL_MIC_MUTE_DISABLE	0x365




struct wmax_led_args {
	u32 led_mask;        //LIght Bar led mask
	u16 command;       //ON or OFF command
	u16 state;               //Apply mode
} ;



static struct platform_device *platform_device;
static u8 current_brightness;



static struct platform_driver platform_driver = {
	.driver = {
		   .name = "redskull-wmi",
	 }
};



static int dell_led_perform_fn(u32 ledmask,
		u16 command,
		u16 state
		)
{
	int method_id;
	char *guid;
	
	struct acpi_buffer input;
	acpi_status status;

	//struct wmax_led_args args;
	struct wmax_led_args wmax_basic_args;
	wmax_basic_args.led_mask= ledmask;
	wmax_basic_args.command= command;
	wmax_basic_args.state= state;
	guid = WMAX_CONTROL_GUID;
	method_id = WMAX_METHOD_ZONE_CONTROL;	
	
	input.length = sizeof(struct wmax_led_args);
	input.pointer = &wmax_basic_args;
	


	pr_debug("Red Skull-wmi: command %d\n",wmax_basic_args.command);
	
	pr_debug("Red Skull-wmi: guid %s method %d\n", guid, method_id);

	printk ("Red Skull-wmi: command %d\n",wmax_basic_args.command);

	status = wmi_evaluate_method(guid, 1, method_id, &input, NULL);
	if (ACPI_FAILURE(status))
		pr_err("Red Skull WMI: zone1 set failure: %u\n", status);
	return ACPI_FAILURE(status);

}

static int led_on(void)
{
	return dell_led_perform_fn(LED_MASK,CMD_LED_ON,APPLY_STATUS);			/* not used */
}

static int led_off(void)
{
	return dell_led_perform_fn(LED_MASK,CMD_LED_OFF,APPLY_STATUS);			/* not used */
}


#if 0
static int led_blink(unsigned char on_eighths,
		unsigned char off_eighths)
{
	return dell_led_perform_fn(5,	/* Length of command */
		INTERFACE_ERROR,	/* Init to  INTERFACE_ERROR */
		DEVICE_ID_PANEL_BACK,	/* Device ID */
		CMD_LED_BLINK,		/* Command */
		on_eighths,		/* blink on in eigths of a second */
		off_eighths);		/* blink off in eights of a second */
}
#endif

static void dell_led_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	printk ("dell_led_set: value %d\n",value);
	current_brightness=value;
	if (value == LED_OFF)
		led_off();
	else
		led_on();
}

static enum led_brightness global_led_get(struct led_classdev *led_cdev)
{
	return current_brightness;
}


#if 0
static int dell_led_blink(struct led_classdev *led_cdev,
		unsigned long *delay_on,
		unsigned long *delay_off)
{
	unsigned long on_eighths;
	unsigned long off_eighths;

	/* The Dell LED delay is based on 125ms intervals.
	   Need to round up to next interval. */

	on_eighths = (*delay_on + 124) / 125;
	if (0 == on_eighths)
		on_eighths = 1;
	if (on_eighths > 255)
		on_eighths = 255;
	*delay_on = on_eighths * 125;

	off_eighths = (*delay_off + 124) / 125;
	if (0 == off_eighths)
		off_eighths = 1;
	if (off_eighths > 255)
		off_eighths = 255;
	*delay_off = off_eighths * 125;

	led_blink(on_eighths, off_eighths);

	return 0;
}

#endif

static struct led_classdev dell_led = {
	.name		= "dell::lightBar",
	.brightness	= LED_FULL,
	.max_brightness = LED_FULL,
	.brightness_set = dell_led_set,
	.brightness_get = global_led_get,
	.flags		= LED_CORE_SUSPENDRESUME,
};

static int __init dell_led_init(void)
{
	int ret;

	if (!wmi_has_guid(WMAX_CONTROL_GUID))
		return -ENODEV;

#if 1

	ret = platform_driver_register(&platform_driver);
	if (ret)
		goto fail_platform_driver;
	platform_device = platform_device_alloc("redskull-wmi", -1);
	if (!platform_device) {
		ret = -ENOMEM;
		goto fail_platform_device1;
	}
	ret = platform_device_add(platform_device);
	if (ret)
		goto fail_platform_device2;
#endif

#if 1
	if (wmi_has_guid(WMAX_CONTROL_GUID)) {

		led_classdev_register(&platform_device->dev, &dell_led);
	}
#endif
	printk("enter dell_led_init.");

	current_brightness=LED_OFF;
	//led_classdev_register(&platform_device->dev, &dell_led);
	
	return 0;
	
#if 1
fail_platform_device2:
	platform_device_put(platform_device);
fail_platform_device1:
	platform_driver_unregister(&platform_driver);
fail_platform_driver:
	return ret;
#endif
}

static void __exit dell_led_exit(void)
{

	if(platform_device){
		led_classdev_unregister(&dell_led);
		platform_device_unregister(platform_device);
		platform_driver_unregister(&platform_driver);
	}
#if 0
	if (wmi_has_guid(WMAX_CONTROL_GUID)) {
		error = led_off();
		if (error == 0)
	}
#endif
}

module_init(dell_led_init);
module_exit(dell_led_exit);
