#ifndef __USB_MAIN_H
#define __USB_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/netlink.h>

#define USB_DEVICES_PATH "/sys/bus/usb/devices"
#define USBIP_FOLDER "/opt/tcp-usbip"
#define USBIP_PRINTF_FILE USBIP_FOLDER"/usbipchannel.log"
#define USBIP_FILE_LOCK USBIP_FOLDER"/Single_lock"

extern FILE *pf_stream;

#define LLOG_ERR(_args, ...) \
	  if (pf_stream) { fprintf(pf_stream, _args, ##__VA_ARGS__);} \
	  	else {printf(_args, ##__VA_ARGS__);}
#define LOG_LEVEL 13

#ifdef DEBUG
#define LLOG(_level, _args, ...) \
	 if (pf_stream && _level < LOG_LEVEL) { fprintf(pf_stream, _args, ##__VA_ARGS__);}  \
	 	else if(_level < LOG_LEVEL){printf(_args, ##__VA_ARGS__);}
#else
#define LLOG(_level, _args, ...) \
	(void)(0)
#endif

#define MAX_USBID 32
#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define WHITELIST 0
#define BLACKLIST 1

#define UEVENT_BUFFER_SIZE      2048
#define UUID_SIZE 36

typedef unsigned int uint32; 
typedef unsigned short uint16;
typedef unsigned char uint8;

#define GET_UINT16(_p1, _offset) ( \
	(uint16) (*(((uint8 *) _p1) + _offset)) + \
	((uint16) (*(((uint8 *) _p1) + _offset + 1)) << 8))
#define GET_UINT32(_p1, _offset) ( \
	(uint32) (*(((uint8 *) _p1) + _offset)) + \
	((uint32) (*(((uint8 *) _p1) + _offset + 1)) << 8) + \
	((uint32) (*(((uint8 *) _p1) + _offset + 2)) << 16) + \
	((uint32) (*(((uint8 *) _p1) + _offset + 3)) << 24))

#define SET_UINT16(_p1, _offset, _value) \
	*(((uint8 *) _p1) + _offset) = (uint8) (((uint16) (_value)) & 0xff), \
	*(((uint8 *) _p1) + _offset + 1) = (uint8) ((((uint16) (_value)) >> 8) & 0xff)
#define SET_UINT32(_p1, _offset, _value) \
	*(((uint8 *) _p1) + _offset) = (uint8) (((uint32) (_value)) & 0xff), \
	*(((uint8 *) _p1) + _offset + 1) = (uint8) ((((uint32) (_value)) >> 8) & 0xff), \
	*(((uint8 *) _p1) + _offset + 2) = (uint8) ((((uint32) (_value)) >> 16) & 0xff), \
	*(((uint8 *) _p1) + _offset + 3) = (uint8) ((((uint32) (_value)) >> 24) & 0xff)


typedef struct parameters_input
{
	char *nbusid[MAX_USBID];
	char *busid;
	int busid_num;
	int serverport;
	char listflag;
	char serverip[64];	
	char uuid[UUID_SIZE + 1];
}PARA_Input;


typedef struct _usbType
{
	char PID[8];
	char VID[8];
} USBType;

typedef struct _usbTypeList 
{
	USBType type;
	struct _usbTypeList *next;
} USBTypeList;

typedef struct _usbBusidList 
{
	char busid[MAX_USBID];
	struct _usbBusidList *next;
} USBBusidList;

typedef struct usbip_exported_device
{
	uint32 busnum;
	uint32 devnum;
	uint32 idVendor;
	uint32 idProduct;
	uint32 bcdDevice;
	uint32 speed;
	uint32 bNumInterfaces;
	uint32 int0_class;
	uint32 int0_subclass;
	uint32 int0_protocol;
}USBIP_EXPORT_DEV;

//2012-6-29 The struct usbip_error_device is added by syt for recovering from the error status.
typedef struct usbip_error_device
{
	uint32	devid;	//the mapped error device id
} USBIP_ERROR_DEV;

typedef struct udev_and_sockfd
{
	USBIP_EXPORT_DEV usb_dev;
	int clientfd;
}Udev_and_sockfd;


typedef struct _usbmap_list
{
	char szDevice[MAX_USBID];
	char mbusid[MAX_USBID];
	uint32 busnum;
	uint32 devnum;
	int clientfd;
	uint32 direction;
	uint32 besend;
	Udev_and_sockfd *udev_fd;
	void* plugin;
	struct _usbmap_list *next;
} USBMapList;


struct tcp_usb_plugin
{
	int sockfd_usbip_client;
	int sockfd_hotplug;
	int serverport;
	char listflag;
	char serverip[64];
	char uuid[UUID_SIZE + 1];
	FILE *fd_lock;
	
	USBTypeList *pUSBTypeList;
	USBMapList *maplist;
	USBBusidList *busidlist;
	int numBusid;
	int numDevice;
};
typedef struct tcp_usb_plugin TCPUsbPlugin;

typedef struct _TCP_PLUGIN_DATA
{
	uint16 size;
	void * data[4];
}
TCP_PLUGIN_DATA;

int usb_send_packet(TCPUsbPlugin * plugin, int type, int flag, char * data, int length);

int tcp_usb_device_entry(TCPUsbPlugin * plugin);

int dev_can_load(TCPUsbPlugin* plugin, char *id);

int usbip_tcp_client(TCPUsbPlugin * plugin);

void device_free(TCPUsbPlugin *plugin);

int load_inf_file(char *busid);

void free_busid_list(TCPUsbPlugin *plugin);

void add_busid_list(TCPUsbPlugin *plugin);

int usbip_hotplug(TCPUsbPlugin *plugin);


#endif
