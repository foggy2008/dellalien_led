/*
 * dell_led.c - Dell LED Driver
 *
 * Copyright (C) 2010 Dell Inc.
 * Louis Davis <louis_davis@dell.com>
 * Jim Dailey <jim_dailey@dell.com>
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
//#include "../../platform/x86/dell-smbios.h"
#include <linux/platform_device.h>


MODULE_AUTHOR("Foggy_Liu");
MODULE_DESCRIPTION("Dell LED Control Driver");
MODULE_LICENSE("GPL");

//#define DELL_LED_BIOS_GUID "F6E4FE6E-909D-47cb-8BAB-C9F6F2F8D396"
//#define DELL_APP_GUID "A80593CE-A997-11DA-B012-B622A1EF5492"
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

/* LED Commands */
//#define CMD_LED_ON	16
//#define CMD_LED_OFF	17
//#define CMD_LED_BLINK	18

#define CMD_LED_ON	0x00FF
#define CMD_LED_OFF	0x0000


#define GLOBAL_MIC_MUTE_ENABLE	0x364
#define GLOBAL_MIC_MUTE_DISABLE	0x365

#if 0
static int dell_micmute_led_set(int state)
{
	struct calling_interface_buffer *buffer;
	struct calling_interface_token *token;

	if (!wmi_has_guid(DELL_APP_GUID))
		return -ENODEV;

	if (state == 0)
		token = dell_smbios_find_token(GLOBAL_MIC_MUTE_DISABLE);
	else if (state == 1)
		token = dell_smbios_find_token(GLOBAL_MIC_MUTE_ENABLE);
	else
		return -EINVAL;

	if (!token)
		return -ENODEV;

	buffer = dell_smbios_get_buffer();
	buffer->input[0] = token->location;
	buffer->input[1] = token->value;
	dell_smbios_send_request(1, 0);
	dell_smbios_release_buffer();

	return state;
}

int dell_app_wmi_led_set(int whichled, int on)
{
	int state = 0;

	switch (whichled) {
	case DELL_LED_MICMUTE:
		state = dell_micmute_led_set(on);
		break;
	default:
		pr_warn("led type %x is not supported\n", whichled);
		break;
	}

	return state;
}
EXPORT_SYMBOL_GPL(dell_app_wmi_led_set);
#endif

#if 0
struct bios_args {
	unsigned char length;
	unsigned char result_code;
	unsigned char device_id;
	unsigned char command;
	unsigned char on_time;
	unsigned char off_time;
};
#endif

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
	//struct wmax_led_args *bios_return;
	//u8 return_code;
	int method_id;
	char *guid;
	//union acpi_object *obj;
	//struct acpi_buffer output = { ACPI_ALLOCATE_BUFFER, NULL };
	struct acpi_buffer input;
	acpi_status status;

	//struct bios_args args;
	struct wmax_led_args wmax_basic_args;
	wmax_basic_args.led_mask= ledmask;
	wmax_basic_args.command= command;
	wmax_basic_args.state= state;
	guid = WMAX_CONTROL_GUID;
	method_id = WMAX_METHOD_ZONE_CONTROL;	
	
	input.length = sizeof(struct wmax_led_args);
	input.pointer = &wmax_basic_args;
	

#if 0
	status = wmi_evaluate_method(DELL_LED_BIOS_GUID,
		1,
		1,
		&input,
		&output);

	if (ACPI_FAILURE(status))
		return status;

	obj = output.pointer;

	if (!obj)
		return -EINVAL;
	else if (obj->type != ACPI_TYPE_BUFFER) {
		kfree(obj);
		return -EINVAL;
	}

	bios_return = ((struct wmax_led_args *)obj->buffer.pointer);
	return_code = bios_return->result_code;

	kfree(obj);

	return return_code;
#endif
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
	.name		= "dell::lid_foggy2",
	.brightness	= LED_FULL,
	.max_brightness = LED_FULL,
	.brightness_set = dell_led_set,
	.brightness_get = global_led_get,
	.flags		= LED_CORE_SUSPENDRESUME,
};

static int __init dell_led_init(void)
{
	int ret;// error;

#if 0
	//if (!wmi_has_guid(DELL_LED_BIOS_GUID) && !wmi_has_guid(DELL_APP_GUID))
	//	return -ENODEV;
#endif

#if 1
	if (!wmi_has_guid(WMAX_CONTROL_GUID))
		return -ENODEV;
#endif


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

#if 0
	if (wmi_has_guid(WMAX_CONTROL_GUID)) {
		error = led_off();
		if (error != 0)
			return -ENODEV;

		led_classdev_register(&platform_device->dev, &dell_led);
	}
#endif
	printk("enter dell_led_init1.");

	current_brightness=LED_OFF;
	led_classdev_register(&platform_device->dev, &dell_led);
	
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
//	int error = 0;

	if(platform_device){
		led_classdev_unregister(&dell_led);
		platform_device_unregister(platform_device);
		platform_driver_unregister(&platform_driver);
	}
#if 0
	if (wmi_has_guid(WMAX_CONTROL_GUID)) {
		error = led_off();
		if (error == 0)
		//	led_classdev_unregister(&dell_led);
	}
#endif
}

module_init(dell_led_init);
module_exit(dell_led_exit);
