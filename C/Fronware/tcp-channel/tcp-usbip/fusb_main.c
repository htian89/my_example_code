#include "fusb_main.h"
#include "bind-driver.h"
#include "utils.h"
#define USBIP_CHANNEL_READY	0
#define USBIP_EXPORT_DEVICE	1
#define USBIP_DATA			    2
#define USBIP_DETACH_DEVICE	3
#define USBIP_REMAP_DEVICE	4
#define USBIP_SERVER_PORT		3240

TCPUsbPlugin * plugin_overall = NULL;
FILE *pf_stream = NULL;

static void show_help(void)
{
	printf("Usage: bind-driver [OPTION]\n");
	printf("Change driver binding for USB/IP.\n");
	printf("  --usbip busid        make a device exportable\n");
	printf("  --usbip vid:pid      make a device exportable\n");
	printf("  --other busid        unbind a usb device\n");
	printf("  --bind  busid        make a device exportable\n");
	printf("  --list               print usb devices and their drivers\n");
	printf("  --uuid               uuid of haproxy channel\n");
}

static void show_start(void)
{
	LLOG(1, "*****************************************************\n");
	LLOG(1, "*****************************************************\n");
	LLOG(1, "**                                                 **\n");
	LLOG(1, "**                  usbip start                    **\n");
	LLOG(1, "**                                                 **\n");
	LLOG(1, "*****************************************************\n");
	LLOG(1, "*****************************************************\n");
}

int open_printf_stream(void)
{
	pf_stream = fopen(USBIP_PRINTF_FILE, "w+");
	if(pf_stream == NULL)
	{
		err("open %s err\n", USBIP_PRINTF_FILE);
		return -1;
	}
	setlinebuf(pf_stream);
	LLOG(1, "USBIPPID:\"%d\"     \n",getpid());
	show_start();
	return 0;
}

#if 0
int file_lock(void)
{
	FILE *fp_lock = NULL;
	int i = 0;
	while(access(USBIP_FILE_LOCK, 0) == 0)
	{
		printf("tcp-usbip is running\n");
		sleep(2);
		i++;
		if(i == 30)
			exit(0);
	}
	fp_lock= fopen(USBIP_FILE_LOCK, "w+");
	fclose(fp_lock);
	return 0; 
}
#endif

int file_lock(void)
{
	int fp_lock;
	int lock_result;
	struct flock lock;
	if(access(USBIP_FILE_LOCK, 0))
	{
		if(access(USBIP_FOLDER, 0))
		{
			printf("create %s\n",USBIP_FOLDER);
			mkdir(USBIP_FOLDER, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		printf("create %s\n",USBIP_FILE_LOCK);
		FILE *lock = fopen(USBIP_FILE_LOCK, "w+");
		fclose(lock);
	}
	fp_lock= open(USBIP_FILE_LOCK, O_RDWR);
	if(fp_lock < 0)
	{
		printf("Open file lock failed.\n");
		exit(1);
	}
	printf("waiting for file lock\n");
	lock_result = lockf(fp_lock, F_LOCK, 0);
	if(lock_result < 0)
	{
		printf("Exec lockf function failed.\n");
		exit(1);
	}
	printf("unlock\n");
	return 0; 
}

int usbip_tcp_client(TCPUsbPlugin * plugin)
{
	struct sockaddr_in usbip_server_addr;
	int usbip_server_addr_len;
	int sockfd;
	const int val = 1;
	int ret;
	if((sockfd= socket(AF_INET,SOCK_STREAM,0))< 0){
		LLOG_ERR("create sockfd_usbip_client error!\n");
		return 1;
	}
	bzero(&usbip_server_addr,sizeof(usbip_server_addr));
	usbip_server_addr.sin_family = AF_INET;
	usbip_server_addr.sin_port = htons(plugin->serverport);
	usbip_server_addr.sin_addr.s_addr = inet_addr(plugin->serverip);
	usbip_server_addr_len = sizeof(usbip_server_addr);

	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
	if (ret < 0)
		LLOG_ERR("setsockopt: TCP_NODELAY\n");

	ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (ret < 0)
		LLOG_ERR("setsockopt: SO_KEEPALIVE\n");

	LLOG(12,"before connect socked_usbip_client=%d\n",sockfd);
	if(connect(sockfd,(struct sockaddr * ) & usbip_server_addr,usbip_server_addr_len)<0)
	{
		LLOG_ERR(" can't connect to IP\n");
		close(sockfd);
		return 1;
	}
    if(strlen(plugin->uuid) == UUID_SIZE)
	{
	    ret = send(sockfd, plugin->uuid, UUID_SIZE, 0);
	    if(ret != UUID_SIZE)
	    {
	        close(sockfd);
	        return 1;
	    }
	}
	plugin->sockfd_usbip_client = sockfd;
	return 0;
}

int usb_send_packet(TCPUsbPlugin * plugin, int type, int flag, char * data, int length)
{
	char * out_data;
	char * ptr;
	int size;
	int len,rest;
	int i = 0;
	LLOG(2, "usb_send_packet: type=%d, flag=%d, length=%d\n\n",
		type, flag, length);
	
	out_data = malloc(length+8);
	if (data != 0)
	{
		memcpy(out_data + 8, data, length);
	}
	size = 8 + length;
	SET_UINT16(out_data, 0, (uint16)type);
	SET_UINT16(out_data, 2, (uint16)flag);
	SET_UINT32(out_data, 4, (uint32)length);
	ptr = out_data;
	rest = size;
	
	while (rest > 0)
	{
		LLOG(2,"sending.........\n");
		len = send(plugin->sockfd_usbip_client,ptr,rest,0);
		if (len < 0)
		{
			char header[8];
			
			LLOG(2,"sending............ len = %d\n",len);
			close(plugin->sockfd_usbip_client);
			if(++i ==5)
			{
				return 1;
			}
			sleep(1);
			usbip_tcp_client(plugin);
			recv(plugin->sockfd_usbip_client,header,8,0);
			continue;
		}
		rest =rest - len;
		ptr += len;
		LLOG(2,"sending............ rest = %d\n",rest);
	}	
	
	free(out_data);
	return 0;
}

int usb_recv_packet(TCPUsbPlugin * plugin)
{
	int sockfd;
	int ret = 0;
	uint16 type;
	uint32 flag;
	uint32 length;
	unsigned char header[8];
	sockfd = plugin->sockfd_usbip_client;
	LLOG(12,"usb_recv_packet:sockfd_usbip_client = %d\n",sockfd);
	do
	{
		ret = recv(sockfd,header,8,0);
		if(ret < 0)
		{
			LLOG_ERR("recv ret < 0\n");
			continue;		
		}
		type = GET_UINT16(header, 0);
		flag = GET_UINT16(header, 2);
		length = GET_UINT32(header, 4);

		LLOG(2, "fusb : thread_process_message: type=%d flag=%d length=%d\n",
				type, flag, length);
		
		switch (type)
		{
			case USBIP_CHANNEL_READY:
				LLOG_ERR(0,("USBIP_CHANNEL_READY \n"));
				send_all_usb_info(plugin);
				return 0;
			default:
				LLOG_ERR(0, ("thread_process_message: type %d not supported\n", type));
				return 1;
		}
	}
	while(1);
	LLOG(0,"usb_recv_packet out\n");
}

static void free_usbtype_list(TCPUsbPlugin *plugin)
{
	USBTypeList *pList;
LLOG(12,"fusb_main.c:free_usbtype_list\n");
	if(!plugin || !plugin->pUSBTypeList)
		return;
	pList = plugin->pUSBTypeList;
	while (pList)
	{
		USBTypeList *pTemp = pList->next;
		free(pList);
		pList = pTemp;
	}
	plugin->pUSBTypeList = NULL;
}

void free_busid_list(TCPUsbPlugin *plugin)
{
	USBBusidList *pList;
LLOG(12,"fusb_main.c:free_busid_list\n");
	if(!plugin || !plugin->busidlist)
		return;
	pList = plugin->busidlist;
	while (pList)
	{
		USBBusidList *pTemp = pList->next;
		free(pList);
		pList = pTemp;
	}
	plugin->numBusid = 0;
	plugin->busidlist = NULL;
}

#if 1
void add_busid_list(TCPUsbPlugin *plugin)
{
	USBBusidList *pList;
	if(!plugin || plugin->busidlist)
		return;
	DIR *dir;
	struct sigaction act;
	dir = opendir("/sys/bus/usb/devices/");
	if (!dir)
		LLOG_ERR("opendir: %s", strerror(errno));

	int nCount = 0;
	for (;;)
	{
		struct dirent *dirent;
		char *busid;

		dirent = readdir(dir);
		if (!dirent)
			break;

		busid = dirent->d_name;
		if (!is_usb_device(busid))
		{
			continue;
		}
		if(!plugin->busidlist)
		{
			plugin->busidlist = (USBBusidList*)malloc(sizeof(USBBusidList));
			memset(plugin->busidlist,0, sizeof(USBBusidList));
			memcpy(plugin->busidlist->busid, busid, MAX_USBID);
			plugin->numBusid++;
			pList = plugin->busidlist;
		}		
		else
		{
			pList->next = (USBBusidList*)malloc(sizeof(USBBusidList));
			memset(pList->next,0, sizeof(USBBusidList));
			memcpy(pList->next->busid, busid, MAX_USBID);
			plugin->numBusid++;
			pList = pList->next;
		}
	}
}
#endif

#if 0
void add_busid_list(TCPUsbPlugin *plugin)
{
	USBBusidList *pList;
	if(!plugin || plugin->busidlist)
		return;
	
LLOG(12,("fusb_main.c:add_busid_list\n"));
	struct dlist *busid_list;
	struct sysfs_device *sysfs_dev;
	struct sysfs_bus *ubus;
	ubus = sysfs_open_bus("usb");
	if (!ubus) {
		err("could not open bus");
		return;
	}
	busid_list = sysfs_get_bus_devices(ubus);
	if (!busid_list) {
		LLOG_ERR("openlink: %s err",USB_DEVICES_PATH);
		return;
	}
	dlist_for_each_data(busid_list, sysfs_dev, struct sysfs_device)
	{
		if (!is_usb_device(sysfs_dev->bus_id))
		{
			continue;
		}
		if(!plugin->busidlist)
		{
			plugin->busidlist = (USBBusidList*)malloc(sizeof(USBBusidList));
			memset(plugin->busidlist,0, sizeof(USBBusidList));
			memcpy(plugin->busidlist->busid, sysfs_dev->bus_id, MAX_USBID);
			plugin->numBusid++;
			pList = plugin->busidlist;
		}		
		else
		{
			pList->next = (USBBusidList*)malloc(sizeof(USBBusidList));
			memset(pList->next,0, sizeof(USBBusidList));
			memcpy(pList->next->busid, sysfs_dev->bus_id, MAX_USBID);
			plugin->numBusid++;
			pList = pList->next;
		}
	}
	
	sysfs_close_bus(ubus);
}
#endif

static void disconnect_signal_process(int sig)
{	
LLOG(12,"fusb_main.c:disconnect_signal_process\n");
	TCPUsbPlugin *plugin = plugin_overall;
//	system("rm -f /home/.tcp-usbip/Single_lock");  
	if(plugin)
	{
		free_usbtype_list(plugin);
		free_busid_list(plugin);
		device_free(plugin);
		if(plugin->sockfd_usbip_client){
			close(plugin->sockfd_usbip_client);	
			plugin->sockfd_usbip_client = 0;
		}
		if(plugin->sockfd_hotplug){
			close(plugin->sockfd_hotplug);
			plugin->sockfd_hotplug = 0;
		}
		free(plugin);
	}
	LLOG(1,"free success\n");
	if(pf_stream){
		fclose(pf_stream);
		pf_stream = NULL;
	}
	exit(0);
}

static void close_usbip_process(TCPUsbPlugin *plugin)
{
LLOG(12,"fusb_main.c:close_usbip_process\n");
}

void add_usb_type(TCPUsbPlugin *plugin, char* VID, char* PID)
{
	USBTypeList *pList = NULL;
LLOG(12,"fusb_main.c:add_usb_type\n");
	if(plugin == NULL)
		return;

	if ((strlen(VID)!=4)||(strlen(PID)!=4))
	{
		LLOG_ERR("add_usb_type:irregular vid and pid\n");
		return;
	}

	if(plugin->pUSBTypeList == NULL)
	{
		plugin->pUSBTypeList = (USBTypeList*)malloc(sizeof(USBTypeList));
		memcpy(plugin->pUSBTypeList->type.PID, PID, 8);
		memcpy(plugin->pUSBTypeList->type.VID, VID, 8);
		plugin->pUSBTypeList->next = NULL;
	}
	else
	{
		char szVID[MAX_PATH] = {0};
		char szPID[MAX_PATH] = {0};
		int i;
		pList = plugin->pUSBTypeList;
		while (pList)
		{
			for (i = 0; i < strlen(pList->type.PID); i++)
				szPID[i] = tolower(pList->type.PID[i]);
			szPID[strlen(pList->type.PID)] = '\0';

			for (i = 0; i < strlen(pList->type.VID); i++)
				szVID[i] = tolower(pList->type.VID[i]);
			szVID[strlen(pList->type.VID)] = '\0';

			if (strstr(PID, szPID) != NULL && strstr(VID,szVID ) != NULL)
				return ;
			pList = pList->next;
		}
		pList = plugin->pUSBTypeList;
		while (1)
		{
			if(pList->next == NULL)
			{
				pList->next = (USBTypeList*)malloc(sizeof(USBTypeList));
				memcpy(pList->next->type.PID, PID, 8);
				memcpy(pList->next->type.VID, VID, 8);
				LLOG(11,"add_usb_type:%s:%s\n",pList->next->type.PID,pList->next->type.VID);
				pList->next->next = NULL;
				break;
			}
			pList = pList->next;
		}	
	}
}


static int usb_load_device_plugin(TCPUsbPlugin * plugin)
{
	if(tcp_usb_device_entry(plugin))
	{
		LLOG_ERR("tcp_usb_device_entry: error\n");
		return 1;
	}
	if(usbip_hotplug(plugin))
	{
		LLOG_ERR("usbip_hotplug: error\n");
		return 1;
	}
	return 0;
}

static int get_usbip_name(TCPUsbPlugin * plugin ,char *busid)
{
	TCP_PLUGIN_DATA *data = NULL;
	char name[10] = {'\0'};
	int len;
	char * iddata;
	iddata = busid;
	data = (TCP_PLUGIN_DATA *) malloc(sizeof(TCP_PLUGIN_DATA));
	memset(data, 0 , sizeof(TCP_PLUGIN_DATA));
	len=strlen(busid);
	if(len == 9)
	{
		while(len)
		{
			*iddata = tolower(*iddata);
			iddata++;
			len--;
		}
		memcpy(name,busid,9);
	}
	else
	{
		getdevicename(busid, name, sizeof(name));
	}
	if(strlen(name) !=9)
	{
		LLOG_ERR("get_usbip_name: get wrong uspipname.\n");
		return 1;
	}	
	name[4]='\0';
	data->data[0] = &name[0];
	data->data[1] = &name[5];
	data->size = 8;
	if(data&&data->size)
	{
		add_usb_type(plugin, data->data[0], data->data[1]);
	}
	free(data);
	return 0;
}

int use_device_by_usbip_tcp(PARA_Input * setting )
{
	TCP_PLUGIN_DATA *data = NULL;
	TCPUsbPlugin * plugin = NULL;
	int ret = 0;
	int num_usbid;
    signal(SIGINT, disconnect_signal_process);
    signal(SIGTERM, disconnect_signal_process);

	LLOG(10, "Tcp Usbip Channel Entry:\n");

	plugin = (TCPUsbPlugin *) malloc(sizeof(TCPUsbPlugin));
	memset(plugin, 0, sizeof(TCPUsbPlugin));

	plugin_overall = plugin;

	plugin->maplist = NULL;
	plugin->numDevice = 0;
	plugin->pUSBTypeList = NULL;
	plugin->busidlist = NULL;
	
	plugin->serverport = setting->serverport;
	plugin->listflag = setting->listflag;
	strcpy(plugin->serverip,setting->serverip);
	strcpy(plugin->uuid, setting->uuid);

	for(num_usbid=0;num_usbid<setting->busid_num;num_usbid++)
	{
		if(get_usbip_name(plugin,setting->nbusid[num_usbid]))
			LLOG_ERR("can't get %s usbip name\n",setting->nbusid[num_usbid]);				
	}
	free(setting);
	ret = usb_load_device_plugin(plugin);
	if(ret == 1)
	{
		LLOG_ERR("use_device_by_usbip_tcp:err\n");
		disconnect_signal_process(0);
	}
	return 0;
}

int main(int argc, char **argv)
{
	char *remote_host = NULL;
	char *data = NULL;
	int index =1;
	int i = 0;
	int debug_flag = 0;
	PARA_Input * setting;
	enum {
		cmd_unknown = 0,
		cmd_use_by_usbip,
		cmd_use_by_other,
		cmd_bind,
		cmd_list,
		cmd_help,
	} cmd = cmd_unknown;

	if (geteuid() != 0)
		err("main: running non-root?\n");
	if(argc<index +1)
	{
		err("no parameters specified!\n");
		cmd = cmd_help;
	}
	setting = (PARA_Input *) malloc(sizeof(PARA_Input));
	memset(setting,0,sizeof(PARA_Input));
	strcpy(setting->serverip,"127.0.0.1");
	setting->serverport = USBIP_SERVER_PORT;
	setting->listflag = WHITELIST;
	setting->busid_num = i;

	while(index < argc)
	{
		if(strcmp("--list",argv[index]) == 0)
		{
			cmd = cmd_list;
			break;
		}
		else if((strcmp("-h",argv[index]) ==0)||(strcmp("--help",argv[index])==0))
		{
			cmd = cmd_help;
			break;
		}
		else if(strcmp("--stop",argv[index]) ==0)
		{
			FILE *pidf;
			char buff[20] = {0};
			char pid[10] = {0};
			int usbip_pid;
			printf("read pid\n");
			pidf = fopen(USBIP_PRINTF_FILE, "r");
			if(pidf == NULL)
			{
				err("open %s err\n", USBIP_PRINTF_FILE);
				return -1;
			}
			if((fgets(buff, 20, pidf)) == NULL)
			{
				err("get buff failed \n");
				return -1;
			}
			char *temph;
			char *tempe;
			printf("%s \n", buff);
			temph = strstr(buff, "\"");
			if(!temph)
					return 1;
			temph++;	   
			tempe = strstr(temph, "\"");
			strncpy(pid, temph, tempe - temph);
			usbip_pid = atoi(pid);
			printf("usbip pid is %d\n",usbip_pid);
			kill(usbip_pid, SIGINT);
			fclose(pidf);
			return 1;
		}
		else if((strcmp("-D",argv[index]) ==0)||(strcmp("--debug",argv[index])==0))
		{
			debug_flag = 1;
			break;
		}
		else if(strcmp("--usbip",argv[index]) == 0)
		{
			cmd = cmd_use_by_usbip;
			index ++;
			while((index < argc) && (strcmp("--",argv[index])!= 0)&&(i<MAX_USBID))
			{
				setting->nbusid[i] = argv[index];
				index++;
				i++;			
			}

			setting->busid_num = i;
		}
		else if(strcmp("--flag",argv[index]) == 0)
		{
			cmd = cmd_use_by_usbip;
			if(++index == argc)
			{
				err("missing flag\n");
				return 1;
			}
			setting->listflag = atoi(argv[index]);
			if(setting->listflag != WHITELIST && setting->listflag != BLACKLIST)
			{
				err("err flag\n");
				return 1;
			}
		}
		else if(strcmp("--port",argv[index]) == 0)
		{
			if(++index == argc)
			{
				err("missing port\n");
				return 1;
			}
			setting->serverport = atoi(argv[index]);
		}
		
		else if(argv[index][0] != '-')
		{
			strncpy(setting->serverip,argv[index],sizeof(setting->serverip)-1);
			
		}
		else if(strcmp("--other",argv[index]) == 0)
		{
			if(++index == argc)
			{
				err("missing other busid\n");
				return 1;
			}
			cmd = cmd_use_by_other;
			setting->busid = argv[index];
			break;
		}
		else if(strcmp("--bind",argv[index]) ==0)
		{
			if(++index == argc)
			{
				err("missing bind busid\n");
				return 1;
			}
			cmd = cmd_bind;
			setting->busid = argv[index];
			break;
		}
        else if(strcmp("--uuid",argv[index]) ==0)
        {
            if(++index == argc)
            {
                err("missing uuid\n");
                return 1;
            }
            strncpy(setting->uuid, argv[index], UUID_SIZE);
        }
		else
		{
			err("parameters input wrong!\n");
			cmd = cmd_help;
			break;
		}
		index++;
	}

	switch (cmd) {
		case cmd_use_by_usbip:
			if(!setting->busid_num && (setting->listflag == WHITELIST))
			{
				err("missing usbip busid\n");
				return 1;
			}
			file_lock();
			if(!debug_flag)
			{
				if(open_printf_stream())
				{
					err("open_printf_stream:err\n");
					return 1;
				}
			}
			use_device_by_usbip_tcp(setting);	
			break;
		case cmd_use_by_other:
			use_device_by_other(setting->busid);	
			break;
		case cmd_list:
			show_devices();
			break;
		case cmd_bind:
			use_device_by_usbip(setting->busid);
			break;
		case cmd_help: /* fallthrough */
		case cmd_unknown:
			show_help();
			break;
		default:
			err(10,"main: NOT REACHED\n");
	}
	return 0;
}
