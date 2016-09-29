#ifndef __BIND_DRIVER_H
#define __BIND_DRIVER_H

#include "utils.h"
#include <regex.h>
#include <dirent.h>

#define DRIVER_SIZE 100

enum unbind_status {
	UNBIND_ST_OK,
	UNBIND_ST_USBIP_HOST,
	UNBIND_ST_FAILED
};


int show_devices(void);

int modify_match_busid(char *busid, int add);

int unbind_interface_busid(char *busid);

int unbind_interface(char *busid, int configvalue, int interface);
int bind_interface_busid(char *busid, char *driver);

int bind_interface(char *busid, int configvalue, int interface, char *driver);
int unbind(char *busid);

int bind_to_usbip(char *busid);

int use_device_by_usbip(char *busid);
int is_usb_device(char *busid);

int use_device_by_other(char *busid);

#endif
