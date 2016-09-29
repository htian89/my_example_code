/*
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#ifndef _USBIP_STUB_DRIVER_H
#define _USBIP_STUB_DRIVER_H

#include "usbip_common.h"

struct usbip_stub_driver {
	int ndevs;
	struct sysfs_driver *sysfs_driver;

	struct dlist *edev_list;	/* list of exported device */
};

struct usbip_exported_device_linux {
	struct sysfs_device *sudev;

	int32_t status;
	struct usb_device    udev;
	struct usb_interface uinf[];
};


extern struct usbip_stub_driver *stub_driver;

int usbip_stub_driver_open(void);
void usbip_stub_driver_close(void);

int usbip_stub_refresh_device_list(void);
int usbip_stub_export_device(struct usbip_exported_device_linux *edev, int sockfd);
int usbip_stub_close_device(struct usbip_exported_device_linux *edev, char *buff);

struct usbip_exported_device_linux *usbip_stub_get_device(int num);

#endif
