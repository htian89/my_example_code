/*
 * $Id: bind-driver.c 85 2009-01-13 17:10:51Z hirofuchi $
 *
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */
#include "usbip_common.h"
#include "bind-driver.h"
#include "fusb_main.h"
const char match_busid_path[] = "/sys/bus/usb/drivers/usbip/match_busid";
const char unbind_path_format[] = "/sys/bus/usb/devices/%s/driver/unbind";
const char bind_path_format[] = "/sys/bus/usb/drivers/%s/bind";

int modify_match_busid(char *busid, int add)
{
	struct sysfs_attribute *match_busid_attr;
	int ret;
	char buff[BUS_ID_SIZE + 4];

	/* BUS_IS_SIZE includes NULL termination? */
	if (strnlen(busid, BUS_ID_SIZE) > BUS_ID_SIZE - 1) {
		LLOG_ERR("too long busid");
		return -1;
	}

	match_busid_attr = sysfs_open_attribute(match_busid_path);
	if (!match_busid_attr)
	{
		LLOG_ERR("problem getting match_busid attribute: %s",
		    strerror(errno));
		return -1;
	}

	if (add)
		snprintf(buff, BUS_ID_SIZE + 4, "add %s", busid);
	else
		snprintf(buff, BUS_ID_SIZE + 4, "del %s", busid);

	LLOG(1, "write \"%s\" to %s \n", buff, match_busid_path);

	ret = sysfs_write_attribute(match_busid_attr, buff, sizeof(buff));
	if (ret < 0) {
		LLOG_ERR("failed to write match_busid: %s", strerror(errno));
		sysfs_close_attribute(match_busid_attr);
		return -1;
	}
	sysfs_close_attribute(match_busid_attr);

	return 0;
}

/* buggy driver may cause dead lock */
int unbind_interface_busid(char *busid)
{
	char unbind_path[PATH_MAX];
	int fd;
	int ret;

	snprintf(unbind_path, sizeof(unbind_path), unbind_path_format, busid);

	fd = open(unbind_path, O_WRONLY);
	if (fd < 0) {
		LLOG_ERR("opening unbind_path failed: %d", fd);
		return -1;
	}

	ret = write(fd, busid, strnlen(busid, BUS_ID_SIZE));
	if (ret < 0) {
		LLOG_ERR("write to unbind_path failed: %d", ret);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int unbind_interface(char *busid, int configvalue, int interface)
{
	char inf_busid[BUS_ID_SIZE];
	LLOG(1,"unbinding interface \n");
	snprintf(inf_busid, BUS_ID_SIZE, "%s:%d.%d", busid, configvalue, interface);
	return unbind_interface_busid(inf_busid);
}



int bind_interface_busid(char *busid, char *driver)
{
	char bind_path[PATH_MAX];
	int fd;
	int ret;

	snprintf(bind_path, sizeof(bind_path), bind_path_format, driver);

	fd = open(bind_path, O_WRONLY);
	if (fd < 0)
		return -1;

	ret = write(fd, busid, strnlen(busid, BUS_ID_SIZE));
	if (ret < 0) {
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

 int bind_interface(char *busid, int configvalue, int interface, char *driver)
{
	char inf_busid[BUS_ID_SIZE];

	snprintf(inf_busid, BUS_ID_SIZE, "%s:%d.%d", busid, configvalue, interface);

	return bind_interface_busid(inf_busid, driver);
}

#if 1
int unbind(char *busid)
{
	int configvalue = 0;
	int ninterface = 0;
	int devclass = 0;
	int i;
	int failed = 0;
	configvalue = read_bConfigurationValue(busid);
	ninterface  = read_bNumInterfaces(busid);
	devclass  = read_bDeviceClass(busid);

	if (configvalue < 0 || ninterface < 0 || devclass < 0) {
		LLOG_ERR("read config and ninf value, removed?");
		return -1;
	}

	if (devclass == 0x09) {
		LLOG(1, "skip unbinding of hub");
		return -1;
	}

	for (i = 0; i < ninterface; i++) {
		char driver[DRIVER_SIZE];
		int ret;	
		bzero(driver, DRIVER_SIZE);		

		getdriver(busid, configvalue, i, driver, DRIVER_SIZE);

		LLOG(1," %s:%d.%d	-> %s \n", busid, configvalue, i, driver);

		if (!strncmp("none", driver, DRIVER_SIZE))
		{
			continue; /* unbound interface */
		}
		/* unbinding */
		ret = unbind_interface(busid, configvalue, i);
		if (ret < 0) {
			LLOG_ERR("unbind driver at %s:%d.%d failed",
					busid, configvalue, i);
			failed = 1;
		}
	}
	if (failed)
		return -1;
	else
		return 0;
}
#endif

#if 0
/* buggy driver may cause dead lock */
int unbind(char *busid)
{
	char bus_type[] = "usb";
	char intf_busid[SYSFS_BUS_ID_SIZE];
	struct sysfs_device *busid_dev;
	struct sysfs_device *intf_dev;
	struct sysfs_driver *intf_drv;
	struct sysfs_attribute *unbind_attr;
	struct sysfs_attribute *bConfValue;
	struct sysfs_attribute *bDevClass;
	struct sysfs_attribute *bNumIntfs;
	int i, rc;
	enum unbind_status status = UNBIND_ST_OK;

	busid_dev = sysfs_open_device(bus_type, busid);
	if (!busid_dev) {
		dbg("sysfs_open_device %s failed: %s", busid, strerror(errno));
		return -1;
	}

	bConfValue = sysfs_get_device_attr(busid_dev, "bConfigurationValue");
	bDevClass  = sysfs_get_device_attr(busid_dev, "bDeviceClass");
	bNumIntfs  = sysfs_get_device_attr(busid_dev, "bNumInterfaces");
	if (!bConfValue || !bDevClass || !bNumIntfs) {
		dbg("problem getting device attributes: %s",
		    strerror(errno));
		goto err_close_busid_dev;
	}

	if (!strncmp(bDevClass->value, "09", bDevClass->len)) {
		dbg("skip unbinding of hub");
		goto err_close_busid_dev;
	}

	for (i = 0; i < atoi(bNumIntfs->value); i++) {
		snprintf(intf_busid, SYSFS_BUS_ID_SIZE, "%s:%.1s.%d", busid,
			 bConfValue->value, i);
		intf_dev = sysfs_open_device(bus_type, intf_busid);
		if (!intf_dev) {
			dbg("could not open interface device: %s",
			    strerror(errno));
			goto err_close_busid_dev;
		}

		dbg("%s -> %s", intf_dev->name,  intf_dev->driver_name);

		if (!strncmp("unknown", intf_dev->driver_name, SYSFS_NAME_LEN))
			/* unbound interface */
			continue;

		if (!strncmp(USBIP_HOST_DRV_NAME, intf_dev->driver_name,
			     SYSFS_NAME_LEN)) {
			/* already bound to usbip-host */
			status = UNBIND_ST_USBIP_HOST;
			continue;
		}

		/* unbinding */
		intf_drv = sysfs_open_driver(bus_type, intf_dev->driver_name);
		if (!intf_drv) {
			dbg("could not open interface driver on %s: %s",
			    intf_dev->name, strerror(errno));
			goto err_close_intf_dev;
		}

		unbind_attr = sysfs_get_driver_attr(intf_drv, "unbind");
		if (!unbind_attr) {
			dbg("problem getting interface driver attribute: %s",
			    strerror(errno));
			goto err_close_intf_drv;
		}

		rc = sysfs_write_attribute(unbind_attr, intf_dev->bus_id,
					   SYSFS_BUS_ID_SIZE);
		if (rc < 0) {
			/* NOTE: why keep unbinding other interfaces? */
			dbg("unbind driver at %s failed", intf_dev->bus_id);
			status = UNBIND_ST_FAILED;
		}

		sysfs_close_driver(intf_drv);
		sysfs_close_device(intf_dev);
	}

	goto out;

err_close_intf_drv:
	sysfs_close_driver(intf_drv);
err_close_intf_dev:
	sysfs_close_device(intf_dev);
err_close_busid_dev:
	status = UNBIND_ST_FAILED;
out:
	sysfs_close_device(busid_dev);

	return status;
}
#endif

/* call at unbound state */
int bind_to_usbip(char *busid)
{
	int configvalue = 0;
	int ninterface = 0;
	int i;
	int failed = 0;

	configvalue = read_bConfigurationValue(busid);
	ninterface  = read_bNumInterfaces(busid);

	if (configvalue < 0 || ninterface < 0) {
		LLOG_ERR("read config and ninf value, removed?\n");
		return -1;
	}

	for (i = 0; i < ninterface; i++) {
		int ret;

		ret = bind_interface(busid, configvalue, i, "usbip");
		if (ret < 0) {
			LLOG_ERR("bind usbip at %s:%d.%d, failed\n",
					busid, configvalue, i);
			failed = 1;
			/* need to contine binding at other interfaces */
		}
	}

	if (failed)
		return -1;
	else
		return 0;
}

int use_device_by_usbip(char *busid)
{
	int ret;
	LLOG(1,"unbind\n");
	ret = unbind(busid);
	if (ret < 0) {
		LLOG_ERR("unbind drivers of %s, failed\n", busid);
		return -1;
	}
	LLOG(1,"modify_match_busid\n");
	ret = modify_match_busid(busid, 1);
	if (ret < 0) {
		LLOG_ERR("add %s to match_busid, failed\n", busid);
		return -1;
	}
	LLOG(1,"bind_to_usbip\n");
	ret = bind_to_usbip(busid);
	if (ret < 0) {
		LLOG_ERR("bind usbip to %s, failed\n", busid);
		ret = unbind(busid);
		if (ret < 0) {
			LLOG_ERR("unbind drivers of %s, failed\n", busid);
			return -1;
		}
		use_device_by_other(busid);
		return -1;
	}
	return 0;
}

#if 1
int use_device_by_other(char *busid)
{
	int ret;
	int config;
	/* read and write the same config value to kick probing */
	config = read_bConfigurationValue(busid);
	if (config < 0) {
		LLOG_ERR("read bConfigurationValue of %s, failed\n", busid);
		return -1;
	}

	ret = write_bConfigurationValue(busid, config);
	if (ret < 0) {
		LLOG_ERR("read bConfigurationValue of %s, failed\n", busid);
		return -1;
	}
	
	LLOG(1,"bind %s to other drivers than usbip, complete!\n", busid);

	return 0;
}
#endif

#if 0
int use_device_by_other(char *busid)
{
	char bus_type[] = "usb";
	struct sysfs_driver *usbip_host_drv;
	struct sysfs_device *dev;
	struct dlist *devlist;
	int verified = 0;
	int rc, ret = -1;

	char attr_name[] = "bConfigurationValue";
	char sysfs_mntpath[SYSFS_PATH_MAX];
	char busid_attr_path[SYSFS_PATH_MAX];
	struct sysfs_attribute *busid_attr;
	char *val = NULL;
	int len;

	/* verify the busid device is using usbip-host */
	usbip_host_drv = sysfs_open_driver(bus_type, USBIP_HOST_DRV_NAME);
	if (!usbip_host_drv) {
		err("could not open %s driver: %s", USBIP_HOST_DRV_NAME,
		    strerror(errno));
		return -1;
	}

	devlist = sysfs_get_driver_devices(usbip_host_drv);
	if (!devlist) {
		err("%s is not in use by any devices", USBIP_HOST_DRV_NAME);
		goto err_close_usbip_host_drv;
	}

	dlist_for_each_data(devlist, dev, struct sysfs_device) {
		if (!strncmp(busid, dev->name, strlen(busid)) &&
		    !strncmp(dev->driver_name, USBIP_HOST_DRV_NAME,
			     strlen(USBIP_HOST_DRV_NAME))) {
			verified = 1;
			break;
		}
	}

	if (!verified) {
		err("device on busid %s is not using %s", busid,
		    USBIP_HOST_DRV_NAME);
		goto err_close_usbip_host_drv;
	}

	/*
	 * NOTE: A read and write of an attribute value of the device busid
	 * refers to must be done to start probing. That way a rebind of the
	 * default driver for the device occurs.
	 *
	 * This seems very hackish and adds a lot of pointless code. I think it
	 * should be done in the kernel by the driver after del_match_busid is
	 * finished!
	 */

	rc = sysfs_get_mnt_path(sysfs_mntpath, SYSFS_PATH_MAX);
	if (rc < 0) {
		err("sysfs must be mounted: %s", strerror(errno));
		return -1;
	}

	snprintf(busid_attr_path, sizeof(busid_attr_path), "%s/%s/%s/%s/%s/%s",
		 sysfs_mntpath, SYSFS_BUS_NAME, bus_type, SYSFS_DEVICES_NAME,
		 busid, attr_name);

	/* read a device attribute */
	busid_attr = sysfs_open_attribute(busid_attr_path);
	if (!busid_attr) {
		err("could not open %s/%s: %s", busid, attr_name,
		    strerror(errno));
		return -1;
	}

	if (sysfs_read_attribute(busid_attr) < 0) {
		err("problem reading attribute: %s", strerror(errno));
		goto err_out;
	}

	len = busid_attr->len;
	val = malloc(len);
	*val = *busid_attr->value;
	sysfs_close_attribute(busid_attr);

	/* notify driver of unbind */
//	rc = modify_match_busid(busid, 0);
//	if (rc < 0) {
//		err("unable to unbind device on %s", busid);
//		goto err_out;
//	}

	/* write the device attribute */
	busid_attr = sysfs_open_attribute(busid_attr_path);
	if (!busid_attr) {
		err("could not open %s/%s: %s", busid, attr_name,
		    strerror(errno));
		return -1;
	}

	rc = sysfs_write_attribute(busid_attr, val, len);
	if (rc < 0) {
		err("problem writing attribute: %s", strerror(errno));
		goto err_out;
	}
	sysfs_close_attribute(busid_attr);

	ret = 0;
	printf("unbind device on busid %s: complete\n", busid);

err_out:
	free(val);
err_close_usbip_host_drv:
	sysfs_close_driver(usbip_host_drv);

	return ret;
}
#endif

int is_usb_device(char *busid)
{
	int ret;

	regex_t regex;
	regmatch_t pmatch[1];

	ret = regcomp(&regex, "^[0-9]+-[0-9]+(\\.[0-9]+)*$", REG_NOSUB|REG_EXTENDED);
	if (ret < 0)
		LLOG_ERR("regcomp: %s\n", strerror(errno));

	ret = regexec(&regex, busid, 0, pmatch, 0);
	if (ret)
	{
		regfree(&regex);
		return 0;	/* not matched */
	}
	regfree(&regex);
	return 1;
}

int show_devices(void)
{
	struct dlist *busid_list;
	struct sysfs_device *sysfs_dev;
	struct sysfs_bus *ubus;
	ubus = sysfs_open_bus("usb");
	if (!ubus) {
		err("could not open bus");
		return -1;
	}
	busid_list = sysfs_get_bus_devices(ubus);
	if (!busid_list) {
		err("openlink: %s err",USB_DEVICES_PATH);
		sysfs_close_bus(ubus);
		return -1;
	}
	printf("List USB devices\n");
	dlist_for_each_data(busid_list, sysfs_dev, struct sysfs_device)
	{
		char *busid;
		busid = sysfs_dev->bus_id;
		if (is_usb_device(busid)) {
			char name[100] = {'\0'};
			char driver[100] =  {'\0'};
			int conf, ninf = 0;
			int i;

			conf = read_bConfigurationValue(busid);
			ninf = read_bNumInterfaces(busid);

			getdevicename(busid, name, sizeof(name));

			printf(" - busid %s (%s)\n", busid, name);

			for (i = 0; i < ninf; i++) {
				getdriver(busid, conf, i, driver, sizeof(driver));
				printf("         %s:%d.%d -> %s\n", busid, conf, i, driver);
			}
			printf("\n");
		}
	}
	sysfs_close_bus(ubus);
	return 0;
}
