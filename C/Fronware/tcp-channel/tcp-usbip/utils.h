#ifndef __UTILS_H
#define __UTILS_H


#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sysfs/libsysfs.h>

/* Be sync to kernel header */
#define BUS_ID_SIZE 32

int read_string(char *path, char *string, size_t len);
int read_integer(char *path);
int getdevicename(char *busid, char *name, size_t len);
int getdriver(char *busid, int conf, int infnum, char *driver, size_t len);
int read_bNumInterfaces(char *busid);
int read_bConfigurationValue(char *busid);
int write_integer(char *path, int value);
int write_bConfigurationValue(char *busid, int config);
int read_bDeviceClass(char *busid);
int readline(int sockfd, char *str, int strlen);
int writeline(int sockfd, char *buff, int bufflen);

#endif
