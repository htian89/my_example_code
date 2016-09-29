#include "usbip_header_linux.h"

#include "fusb_main.h"
/* 	read_usb_device(sudev, &idev->udev)	*/
#include "usbip_common.h"
/*	bind_to_usbip(szDev)	*/
#include "bind-driver.h"
/*	read_bConfigurationValue(busid);
	read_bNunInterfaces(busid);
	dlist	*/
#include "utils.h"
/*	is_usb_device()		*/
#include "stub_driver.h"
/* ustip_header */

#define USBIP_CHANNEL_READY	0
#define USBIP_EXPORT_DEVICE	1
#define USBIP_DATA		2
#define USBIP_DETACH_DEVICE	3

TCPUsbPlugin *usbdev_plugin;

int hotplug_flag = 0;

USBMapList *has_the_dev(TCPUsbPlugin* plugin, char *szDev, char *busid)
{
	USBMapList *pList = NULL;
LLOG(12,"fusb_linux.c:has_the_dev\n");
	if (!plugin)
		return NULL;
	pList = plugin->maplist;
	while (pList != NULL)
	{
		if ((strstr(pList->szDevice, szDev) != NULL &&
			strstr(pList->mbusid, busid) != NULL) ||
			(strstr(busid, pList->mbusid) != NULL &&
			strstr(szDev, pList->szDevice) != NULL))
			return pList;
		pList = pList->next;
	}
	return NULL;
}

int dev_can_load(TCPUsbPlugin* plugin, char *id)
{
	USBTypeList *pTypeList;
	char szBuff[MAX_PATH] = {0};
	char szVID[MAX_PATH] = {0};
	char szPID[MAX_PATH] = {0};
LLOG(12,"fusb_linux.c:dev_can_load\n");
	if (!plugin)
		return 0;
	pTypeList = plugin->pUSBTypeList;
	strcpy(szBuff, id);
	int i;
	for(i = 0; i < strlen(szBuff); i++)
		szBuff[i] = tolower(szBuff[i]);
	LLOG(10,"dev_can_load szBuff=%s\n",szBuff);

	while (pTypeList)
	{
		for (i = 0; i < strlen(pTypeList->type.PID); i++)
			szPID[i] = tolower(pTypeList->type.PID[i]);
		szPID[strlen(pTypeList->type.PID)] = '\0';

		for (i = 0; i < strlen(pTypeList->type.VID); i++)
			szVID[i] = tolower(pTypeList->type.VID[i]);
		szVID[strlen(pTypeList->type.VID)] = '\0';

		if (strstr(szBuff, szPID) != NULL && strstr(szBuff,szVID ) != NULL)
			return 1;
		pTypeList = pTypeList->next;
	}
	return 0;
}

int load_inf_file(char *busid)
{
LLOG(12,"fusb_linux.c:load_inf_file\n");

	char driver[DRIVER_SIZE] =  {'\0'};
	int conf, ninf = 0;
	int i;
	conf = read_bConfigurationValue(busid);
	ninf = read_bNumInterfaces(busid);
	if(ninf == 0)
		return 3;
#ifdef FRONVIEW
	for (i = 0; i < ninf; i++) {
		getdriver(busid, conf, i, driver, DRIVER_SIZE);
		if(strstr("usbip",driver) != NULL && strstr(driver,"usbip") != NULL)
		{
			LLOG(1,"driver is usbip\n");
			break;
		}
		else if((strstr("hub",driver) != NULL && strstr(driver,"hub") != NULL) || (strstr("usbhid",driver) != NULL && strstr(driver,"usbhid") != NULL))
		{
			return 2;
		}
		else
		{	
			if(!hotplug_flag){
				if(use_device_by_other(busid) == -1)
					return 0;
				usleep(10000);
			}
			if(use_device_by_usbip(busid) == -1)
				return 0;
			break;
		}
	}
	return 1;
#else
	for (i = 0; i < ninf; i++) {
		getdriver(busid, conf, i, driver, DRIVER_SIZE);
		if((strstr("hub",driver) != NULL && strstr(driver,"hub") != NULL) || (strstr("usbhid",driver) != NULL && strstr(driver,"usbhid") != NULL))
		{
			return 2;
		}
		else
		{
			if (use_device_by_usbip(busid) == -1)
			{	
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
#endif
}

USBMapList *add_map_to_list(TCPUsbPlugin *plugin, char *id, char *busid)
{
	USBMapList *pMapList;
	struct usbip_exported_device_linux *idev;
	int found,ret;
LLOG(12,"fusb_linux.c:add_map_to_list\n");
	found = 0;
	if (!plugin)
		return NULL;
	if(plugin->listflag == WHITELIST)
	{
		if (!dev_can_load(plugin, id))
		{
			LLOG(10,"vid and pid not match\n");
			return NULL;
		}
	}

	ret = load_inf_file(busid);
	if(!plugin)
		exit(0);
	if (ret == 0)
	{
		LLOG_ERR("load_inf_file error\n");
		return NULL;
	}
	else if(ret == 2)
	{
		LLOG(10,"load_inf_file hub or usbhid\n");
		return NULL;
	}
	else if(ret == 3)
	{
		LLOG(10,"load_inf_file ninf is zero\n");
		return NULL;		
	}
	else
	{
		LLOG(10,"load_inf_file over\n");
	}

	ret = usbip_stub_refresh_device_list();
	if (ret < 0)
		LLOG_ERR("refresh device list failed");

	dlist_for_each_data(stub_driver->edev_list, idev, struct usbip_exported_device_linux)
	{
		if (!strncmp(busid, idev->udev.busid, SYSFS_BUS_ID_SIZE))
		{
			found = 1;
			break;
		}
	}
	
	if (!found)
	{
		LLOG_ERR("add map to list dlist error!\n");
		return NULL;
	}
	
	if (plugin->maplist == NULL)
	{
		plugin->maplist = (USBMapList*)malloc(sizeof(USBMapList));
		memset(plugin->maplist,0, sizeof(USBMapList));
		memcpy(plugin->maplist->szDevice, id, MAX_USBID);
		memcpy(plugin->maplist->mbusid, busid, MAX_USBID);
		plugin->maplist->busnum = idev->udev.busnum;
		plugin->maplist->devnum = idev->udev.devnum;
		plugin->maplist->besend = 0;
		plugin->maplist->plugin = plugin;
		plugin->numDevice++;
		plugin->maplist->next = NULL;
		return plugin->maplist;
	}
	else
	{
		pMapList = plugin->maplist;
		while (pMapList)
		{
			if (pMapList->next == NULL)
			{
				pMapList->next = (USBMapList*)malloc(sizeof(USBMapList));
				memset(pMapList->next, 0, sizeof(USBMapList));
				memcpy(pMapList->next->szDevice, id, MAX_USBID);
				memcpy(pMapList->next->mbusid, busid, MAX_USBID);
				pMapList->next->busnum = idev->udev.busnum;
				pMapList->next->devnum = idev->udev.devnum;
				pMapList->next->besend = 0;
				pMapList->next->plugin = plugin;
				plugin->numDevice++;
				pMapList->next->next = NULL;
				return pMapList->next;
			}
			pMapList = pMapList->next;
		}
	}
}

int find_usb_device(TCPUsbPlugin* plugin)
{
	LLOG(12,"fusb_linux.c:find_usb_device\n");
	int nCount = 0;
	char *busid;
	USBBusidList *pBusidList;
	int i;
	pBusidList = plugin->busidlist;
	for(i = 0; i < plugin->numBusid; i++)
	{
		busid = pBusidList->busid;		
		char name[BUS_ID_SIZE] = {'\0'};
		getdevicename(busid, name, sizeof(name));
		LLOG(10," -busid %s (%s)\n", busid, name);
		add_map_to_list(plugin,name, busid);
		pBusidList = pBusidList->next;
		nCount++;
	}
	return nCount;
}

int is_usb_connect(TCPUsbPlugin *plugin, char *usbid)
{
LLOG(12,"fusb_linux.c:is_usb_connect\n");
	char *busid;
	int found = 0;
	int i;
	USBBusidList *pBusidList;
	pBusidList = plugin->busidlist;
	for(i = 0; i < plugin->numBusid; i++)
	{
		busid = pBusidList->busid;
		if (strstr(usbid,busid) != NULL && strstr(busid, usbid) != NULL)
		{
			found = 1;
			break;
		}
		pBusidList = pBusidList->next;
	}
	if (found == 1)
		return 1;
	else
		return 0;
}

static int fresh_usb_list(TCPUsbPlugin *plugin)
{
	int nDevice;
LLOG(12,"fusb_linux.c:fresh_usb_list\n");
	if (!plugin)
		return 0;
	
	add_busid_list(plugin);	
	nDevice = find_usb_device(plugin);
	LLOG(10, "fresh_usb_list: nDevice=%d, plugin->numDevice=%d\n", nDevice, plugin->numDevice);
	return plugin->numDevice;
}

void send_all_usb_info(TCPUsbPlugin *plugin)
{
	USBMapList *pList = NULL;
	Udev_and_sockfd *pDev_fd = NULL;
	int found = 0;
	int ret;
	char header[8];
LLOG(12,"fusb_linux.c:send_all_usb_info\n");
	if (!plugin)
		return;
	if(!fresh_usb_list(plugin))
		return;
	pDev_fd = (Udev_and_sockfd *) malloc(sizeof(Udev_and_sockfd));
	pList = plugin->maplist;
	while (pList)
	{
		char busid[BUS_ID_SIZE];
		struct usbip_exported_device_linux *idev;
		if(pList->besend == 1)
		{
			LLOG(1,"%s has sent! \n", pList->mbusid);
			pList = pList->next;
			continue;
		}
		memcpy(busid, pList->mbusid, sizeof(pList->mbusid));
		dlist_for_each_data(stub_driver->edev_list, idev, struct usbip_exported_device_linux)
		{
			if (!strncmp(busid, idev->udev.busid, SYSFS_BUS_ID_SIZE))
				{
					found = 1;
					break;
				}
		}
					
		if (idev->status != SDEV_ST_AVAILABLE) {
			LLOG(1, "device not available, %s \n", idev->udev.busid);
			switch( idev->status ) {
				case SDEV_ST_ERROR:
					LLOG(1, "     status SDEV_ST_ERROR\n");
					break;
				case SDEV_ST_USED:
					LLOG(1, "     status SDEV_ST_USED\n");
					break;
				default:
					LLOG(1, "     status unknown: 0x%x\n", idev->status);
			}
			pList = pList->next;
			continue;
		}
		
		if (found)
		{			
			pDev_fd->usb_dev.busnum = htonl(idev->udev.busnum);
			pDev_fd->usb_dev.devnum = htonl(idev->udev.devnum);
			pDev_fd->usb_dev.idVendor = htonl(idev->udev.idVendor);
			pDev_fd->usb_dev.idProduct = htonl(idev->udev.idProduct);
			pDev_fd->usb_dev.bcdDevice = htonl(idev->udev.bcdDevice);
			pDev_fd->usb_dev.speed = htonl(idev->udev.speed);
			pDev_fd->usb_dev.bNumInterfaces = htonl(idev->udev.bNumInterfaces);
			pDev_fd->usb_dev.int0_class = htonl(idev->uinf[0].bInterfaceClass);
			pDev_fd->usb_dev.int0_subclass = htonl(idev->uinf[0].bInterfaceSubClass);
			pDev_fd->usb_dev.int0_protocol = htonl(idev->uinf[0].bInterfaceProtocol);
			
			int loglevel = 10;
			LLOG(loglevel,"pDev->busnum = 0x%x\n",pDev_fd->usb_dev.busnum);
			LLOG(loglevel,"pDev->devnum = 0x%x\n",pDev_fd->usb_dev.devnum);
			LLOG(loglevel,"pDev->idVendor = 0x%x\n",pDev_fd->usb_dev.idVendor);
			LLOG(loglevel,"pDev->idProduct = 0x%x\n",pDev_fd->usb_dev.idProduct);
			LLOG(loglevel,"pDev->bcdDevice = 0x%x\n",pDev_fd->usb_dev.bcdDevice);
			LLOG(loglevel,"pDev->speed = 0x%x\n",pDev_fd->usb_dev.speed);
			LLOG(loglevel,"pDev->bNumInterfaces = 0x%x\n",pDev_fd->usb_dev.bNumInterfaces);
			LLOG(loglevel,"pDev->int0_class = 0x%x\n",pDev_fd->usb_dev.int0_class);
			LLOG(loglevel,"pDev->int0_subclass = 0x%x\n",pDev_fd->usb_dev.int0_subclass);
			LLOG(loglevel,"pDev->int0_protocol = 0x%x\n",pDev_fd->usb_dev.int0_protocol);
			
			if(usbip_tcp_client(plugin))
			{
				LLOG_ERR("send_all_usb_info: connect failed\n");
				goto err;
			}
			ret = recv(plugin->sockfd_usbip_client,header,8,0);
			if(ret != 8)
			{
				LLOG_ERR("send_all_usb_info:recv failed\n");
				goto err;
			}
			LLOG(1,"send all usb info: type=%d flag=%d length=%d\n",
			GET_UINT16(header, 0), GET_UINT16(header, 2), GET_UINT32(header, 4));
			if(usb_send_packet(plugin, USBIP_EXPORT_DEVICE, 0, (char*)&pDev_fd->usb_dev, sizeof(USBIP_EXPORT_DEV)))
			{
				LLOG_ERR("send_all_usb_info: send failed\n");
				goto err;
			}
			pList->udev_fd = pDev_fd;
			pList->udev_fd->clientfd = plugin->sockfd_usbip_client;
			pList->clientfd = plugin->sockfd_usbip_client;
			pList->besend = 1;
			usbip_stub_export_device(idev, pList->clientfd);
			pList = pList->next;
		}
		else
		{
			LLOG_ERR("not found requested busid, %s\n" , busid);
			pList = pList->next;
		}
	}
err:
	free(pDev_fd);
	pDev_fd = NULL;
}

int send_one_usb_info(TCPUsbPlugin *plugin, USBMapList *pList)
{
	Udev_and_sockfd *pDev_fd = NULL;
	int found = 0;
	int ret;
	char header[8];
LLOG(12,"fusb_linux.c:send_one_usb_info\n");
	if (pList)
	{
		pDev_fd = (Udev_and_sockfd *) malloc(sizeof(Udev_and_sockfd));
		char busid[BUS_ID_SIZE];
		struct usbip_exported_device_linux *idev;
		if(pList->besend == 1)
		{
			LLOG(1,"%s has sent! \n", pList->mbusid);
			pList = pList->next;
			return 1;
		}
		memcpy(busid, pList->mbusid, sizeof(pList->mbusid));
		dlist_for_each_data(stub_driver->edev_list, idev, struct usbip_exported_device_linux)
		{
			if (!strncmp(busid, idev->udev.busid, SYSFS_BUS_ID_SIZE))
				{
					found = 1;
					break;
				}
		}
					
		if (idev->status != SDEV_ST_AVAILABLE) {
			LLOG(1, "device not available, %s \n", idev->udev.busid);
			switch( idev->status ) {
				case SDEV_ST_ERROR:
					LLOG(1, "     status SDEV_ST_ERROR\n");
					break;
				case SDEV_ST_USED:
					LLOG(1, "     status SDEV_ST_USED\n");
					break;
				default:
					LLOG(1, "     status unknown: 0x%x\n", idev->status);
			}
			return 1;
		}
		
		if (found)
		{			
			pDev_fd->usb_dev.busnum = htonl(idev->udev.busnum);
			pDev_fd->usb_dev.devnum = htonl(idev->udev.devnum);
			pDev_fd->usb_dev.idVendor = htonl(idev->udev.idVendor);
			pDev_fd->usb_dev.idProduct = htonl(idev->udev.idProduct);
			pDev_fd->usb_dev.bcdDevice = htonl(idev->udev.bcdDevice);
			pDev_fd->usb_dev.speed = htonl(idev->udev.speed);
			pDev_fd->usb_dev.bNumInterfaces = htonl(idev->udev.bNumInterfaces);
			pDev_fd->usb_dev.int0_class = htonl(idev->uinf[0].bInterfaceClass);
			pDev_fd->usb_dev.int0_subclass = htonl(idev->uinf[0].bInterfaceSubClass);
			pDev_fd->usb_dev.int0_protocol = htonl(idev->uinf[0].bInterfaceProtocol);
			
			int loglevel = 10;
			LLOG(loglevel,"pDev->busnum = 0x%x\n",pDev_fd->usb_dev.busnum);
			LLOG(loglevel,"pDev->devnum = 0x%x\n",pDev_fd->usb_dev.devnum);
			LLOG(loglevel,"pDev->idVendor = 0x%x\n",pDev_fd->usb_dev.idVendor);
			LLOG(loglevel,"pDev->idProduct = 0x%x\n",pDev_fd->usb_dev.idProduct);
			LLOG(loglevel,"pDev->bcdDevice = 0x%x\n",pDev_fd->usb_dev.bcdDevice);
			LLOG(loglevel,"pDev->speed = 0x%x\n",pDev_fd->usb_dev.speed);
			LLOG(loglevel,"pDev->bNumInterfaces = 0x%x\n",pDev_fd->usb_dev.bNumInterfaces);
			LLOG(loglevel,"pDev->int0_class = 0x%x\n",pDev_fd->usb_dev.int0_class);
			LLOG(loglevel,"pDev->int0_subclass = 0x%x\n",pDev_fd->usb_dev.int0_subclass);
			LLOG(loglevel,"pDev->int0_protocol = 0x%x\n",pDev_fd->usb_dev.int0_protocol);
			
			if(usbip_tcp_client(plugin))
			{
				LLOG_ERR("send_one_usb_info: connect failed\n");
				goto err;
			}
			ret = recv(plugin->sockfd_usbip_client,header,8,0);
			if(ret != 8)
			{
				LLOG_ERR("send_one_usb_info:recv failed\n");
				goto err;
			}
			LLOG(1,"send all usb info: type=%d flag=%d length=%d\n",
			GET_UINT16(header, 0), GET_UINT16(header, 2), GET_UINT32(header, 4));
			if(usb_send_packet(plugin, USBIP_EXPORT_DEVICE, 0, (char*)&pDev_fd->usb_dev, sizeof(USBIP_EXPORT_DEV)))
			{
				LLOG_ERR("send_one_usb_info: send failed\n");
				goto err;
			}
			pList->udev_fd = pDev_fd;
			pList->udev_fd->clientfd = plugin->sockfd_usbip_client;
			pList->clientfd = plugin->sockfd_usbip_client;
			pList->besend = 1;
			usbip_stub_export_device(idev, pList->clientfd);
		}
		else
		{
			LLOG_ERR("not found requested busid, %s\n" , busid);
		}
	}
err:
	free(pDev_fd);
	pDev_fd = NULL;
	return 0;
}

static int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
LLOG(12,"fusb_linux.c:init_hotplug_sock\n");

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1) {
        LLOG(1, "error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        LLOG(1, "bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

int get_busid_from_uevent(char *buf, char *busid)
{
    char flag[5] = {0};
    char *temp, *temp2;
	char *temph;
	char *tempe;
	LLOG(1, 	"%s \n", buf);
	temph = strstr(buf, "/usb");
	if(!temph)
			return 1;
	temph++;
	temph = strstr(temph, "/");
	if(!temph)
			return 1;

    strncpy(flag, temph, 4); //store busid ,but maybe it's not right busid, so need use it to judge what is the really busid
    
    //find the really busid
    temp = strstr(temph + 4, flag);
    while(temp)
    {
        temp2 = strstr(temp + 4, flag);
        if(temp && temp2)
            temph = temp;
        temp = temp2;
    }
	temph++;		   

	tempe = strstr(temph, "/");
	if(tempe)
	{
        *tempe = '\0';
	}
	strncpy(busid, temph, strlen(temph) + 1);
	return 0;
}

int usbip_hotplug(TCPUsbPlugin *plugin)
{
	send_all_usb_info(plugin);
LLOG(12,"fusb_linux.c:usbip_hotplug\n");
	hotplug_flag = 1;
	plugin->sockfd_hotplug = init_hotplug_sock();
	if(plugin->sockfd_hotplug < 0 )
		return 1;
	while(1)
	{
		char buf[UEVENT_BUFFER_SIZE*2] = {0};
		char busid[MAX_USBID] = {0};
		char name[MAX_USBID] = {0};
		USBMapList *pList;
		LLOG(1, "**************************over*************************\n");
		recv(plugin->sockfd_hotplug, buf, sizeof(buf), 0);
		LLOG(1, "\n************************hotplug************************\n");		
		if(get_busid_from_uevent(buf, busid))
			continue;
		getdevicename(busid, name, sizeof(name));
		LLOG(10," -busid %s (%s)\n", busid, name);		
		if(name[0] == 0)
		{
			LLOG(1, "                 *hotplug device out*\n");
			USBMapList *pList_pre;
			pList = has_the_dev(plugin,name,busid);
			if (!pList)
				continue;
			LLOG(10,"is disconnect pList->mbusid=%s\n", pList->mbusid);	
			if(pList == plugin->maplist)
			{
				plugin->maplist = pList->next;				
			}
			else
			{
				pList_pre = plugin->maplist;
				while(pList_pre)
				{
					if(pList_pre->next == pList)
						break;
					pList_pre = pList_pre->next;
				}
				pList_pre->next = pList->next;
			}			
			pList->clientfd = 0;
			pList->busnum = 0;
			pList->devnum = 0;
			pList->besend = 0;
			memset(pList->mbusid,0,sizeof(pList->mbusid));
			memset(pList->szDevice,0,sizeof(pList->szDevice));			
			free(pList);
			pList = NULL;
			plugin->numDevice--;
			LLOG(10,"remove a device\n");	
		}
		else
		{
			LLOG(1, "                 *hotplug device in*\n");
			if(!read_bNumInterfaces(busid))
				continue;
			if (has_the_dev(plugin,name,busid))
				continue;
			pList = add_map_to_list(plugin,name, busid);
			if (pList != NULL && pList->besend != 1)
			{
				send_one_usb_info(plugin, pList);
				pList->besend = 1;
			}						
		}
	}
	return 0;
}

int tcp_usb_device_entry(TCPUsbPlugin * plugin)
{
	int ret;
	usbdev_plugin = plugin;
LLOG(12,"fusb_linux.c:FUSBDeviceEntry\n");
	if (plugin == NULL)
	{
		LLOG_ERR("fusb_linux: unable to register device.\n");
		return 1;
	}

	ret = usbip_stub_driver_open();
	if (ret < 0)
	{
		LLOG_ERR("driver open failed");
		return 1;
	}
	return 0;
}

void device_free(TCPUsbPlugin *plugin)
{
	USBMapList *pMapList;
LLOG(12,"fusb_linux.c:device_free\n");
	if (!plugin)
		return;
	pMapList = plugin->maplist;
	while (pMapList)
	{
		close(pMapList->clientfd);
		unbind(pMapList->mbusid);
        pMapList = pMapList->next;
	}
    usbip_stub_driver_close();
    pMapList = plugin->maplist;
    usleep(500*1000);
	while (pMapList)
	{
		USBMapList *next = pMapList->next;       
		use_device_by_other(pMapList->mbusid);
		free(pMapList);
		LLOG(12,"free a device \n");
		pMapList = next;
	}
}

