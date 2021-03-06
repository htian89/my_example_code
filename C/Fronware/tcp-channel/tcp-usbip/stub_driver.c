/*
 * Copyright (C) 2005-2007 Takahiro Hirofuchi
 */

#include "usbip_common.h"
#include "stub_driver.h"
#include "fusb_main.h"
#include "fusb_insmod.h"

static const char *usbip_stub_driver_name = "usbip";


struct usbip_stub_driver *stub_driver;

static struct sysfs_driver *open_sysfs_stub_driver(void)
{
	int ret;

	char sysfs_mntpath[SYSFS_PATH_MAX];
	char stub_driver_path[SYSFS_PATH_MAX];
	struct sysfs_driver *stub_driver;


	ret = sysfs_get_mnt_path(sysfs_mntpath, SYSFS_PATH_MAX);
	if (ret < 0) {
		err("sysfs must be mounted");
		return NULL;
	}

	snprintf(stub_driver_path, SYSFS_PATH_MAX, "%s/%s/usb/%s/%s",
			sysfs_mntpath, SYSFS_BUS_NAME, SYSFS_DRIVERS_NAME,
			usbip_stub_driver_name);

    stub_driver = sysfs_open_driver_path(stub_driver_path);
    if (!stub_driver) {
        LLOG(1, "load usbip-core.ko and usbip.ko\n");
        insmod_module(USBIP_FOLDER"/usbip-core.ko");
        insmod_module(USBIP_FOLDER"/usbip.ko");

        sleep(1);

        stub_driver = sysfs_open_driver_path(stub_driver_path);
        if (!stub_driver) {
            err("load usbip-core.ko and usbip.ko failed");
            return NULL;
        }
    }


	return stub_driver;
}



/* only the first interface value is true! */
static int32_t read_attr_usbip_status(struct usb_device *udev)
{
	char attrpath[SYSFS_PATH_MAX];
	struct sysfs_attribute *attr;
	int value = 0;
	int  ret;

	snprintf(attrpath, SYSFS_PATH_MAX, "%s/%s:%d.%d/usbip_status",
			udev->path, udev->busid,
			udev->bConfigurationValue,
			0);

	attr = sysfs_open_attribute(attrpath);
	if (!attr) {
		err("open %s", attrpath);
		return -1;
	}

	ret = sysfs_read_attribute(attr);
	if (ret) {
		err("read %s", attrpath);
		sysfs_close_attribute(attr);
		return -1;
	}

	value = atoi(attr->value);

	sysfs_close_attribute(attr);

	return value;
}


static void usbip_exported_device_delete(void *dev)
{
	struct usbip_exported_device_linux *edev =
		(struct usbip_exported_device_linux *) dev;

	sysfs_close_device(edev->sudev);
	free(dev);
}


static struct usbip_exported_device_linux *usbip_exported_device_new(char *sdevpath)
{
	struct usbip_exported_device_linux *edev = NULL;
	
	edev = (struct usbip_exported_device_linux *) calloc(1, sizeof(*edev));
	if (!edev) {
		err("alloc device");
		return NULL;
	}
    LLOG(1, "usbip_exported_device_new: open %s\n", sdevpath);

	edev->sudev = sysfs_open_device_path(sdevpath);
	if (!edev->sudev) {
		err("open %s", sdevpath);
		goto err;
	}

	read_usb_device(edev->sudev, &edev->udev);	
	edev->status = read_attr_usbip_status(&edev->udev);
	if (edev->status < 0)
		goto err;
	/* reallocate buffer to include usb interface data */
	size_t size = sizeof(*edev) + edev->udev.bNumInterfaces * sizeof(struct usb_interface);
	edev = (struct usbip_exported_device_linux *) realloc(edev, size);
	if (!edev) {
		err("alloc device");
		goto err;
	}

	int i;
	for (i=0; i < edev->udev.bNumInterfaces; i++)
		read_usb_interface(&edev->udev, i, &edev->uinf[i]);

	return edev;

err:
	if (edev && edev->sudev)
		sysfs_close_device(edev->sudev);
	if (edev)
		free(edev);
	return NULL;
}


static int check_new(struct dlist *dlist, struct sysfs_device *target)
{
	struct sysfs_device *dev;

	dlist_for_each_data(dlist, dev, struct sysfs_device) {
		if (!strncmp(dev->bus_id, target->bus_id, SYSFS_BUS_ID_SIZE))
			/* found. not new */
			return 0;
	}

	return 1;
}

static void delete_nothing(void *dev)
{
	/* do not delete anything. but, its container will be deleted. */
}

static int refresh_exported_devices(void)
{
	struct sysfs_device	*suinf;  /* sysfs_device of usb_interface */
	struct dlist		*suinf_list;

	struct sysfs_device	*sudev;  /* sysfs_device of usb_device */
	struct dlist		*sudev_list;


	sudev_list = dlist_new_with_delete(sizeof(struct sysfs_device), delete_nothing);

	suinf_list = sysfs_get_driver_devices(stub_driver->sysfs_driver);
	if (!suinf_list) {
		printf("Bind usbip.ko to a usb device to be exportable!\n");
		goto bye;
	}

	/* collect unique USB devices (not interfaces) */
	dlist_for_each_data(suinf_list, suinf, struct sysfs_device) {

		/* get usb device of this usb interface */
		sudev = sysfs_get_device_parent(suinf);
		if (!sudev) {
			err("get parent dev of %s", suinf->name);
			continue;
		}

		if (check_new(sudev_list, sudev)) {
			dlist_unshift(sudev_list, sudev);
		}
	}

	dlist_for_each_data(sudev_list, sudev, struct sysfs_device) {
		struct usbip_exported_device_linux *edev;

		edev = usbip_exported_device_new(sudev->path);
		if (!edev) {
			err("usbip_exported_device_linux new");
			continue;
		}

		dlist_unshift(stub_driver->edev_list, (void *) edev);
		stub_driver->ndevs++;
	}

	dlist_destroy(sudev_list);

bye:
	return 0;
}

int usbip_stub_refresh_device_list(void)
{
	int ret;
	if (stub_driver->edev_list)
		dlist_destroy(stub_driver->edev_list);
	stub_driver->ndevs = 0;

	stub_driver->edev_list = dlist_new_with_delete(sizeof(struct usbip_exported_device_linux),
			usbip_exported_device_delete);
	if (!stub_driver->edev_list) {
		err("alloc dlist");
		return -1;
	}

	ret = refresh_exported_devices();
	if (ret < 0)
		return ret;

	return 0;
}

int usbip_stub_driver_open(void)
{
	int ret;


	stub_driver = (struct usbip_stub_driver *) calloc(1, sizeof(*stub_driver));
	if (!stub_driver) {
		err("alloc stub_driver");
		return -1;
	}

	stub_driver->sysfs_driver = open_sysfs_stub_driver();
	if (!stub_driver->sysfs_driver)
		goto err;

	return 0;


err:
	if (stub_driver->sysfs_driver)
		sysfs_close_driver(stub_driver->sysfs_driver);
	free(stub_driver);

	stub_driver = NULL;
	return -1;
}


void usbip_stub_driver_close(void)
{
	if (!stub_driver)
		return;
	
	if (stub_driver->edev_list)
		dlist_destroy(stub_driver->edev_list);
	if (stub_driver->sysfs_driver)
		sysfs_close_driver(stub_driver->sysfs_driver);
	free(stub_driver);

	stub_driver = NULL;
}

int usbip_stub_close_device(struct usbip_exported_device_linux *edev, char *buff)
{
	char attrpath[SYSFS_PATH_MAX];
	struct sysfs_attribute *attr;
	int ret;

	/* only the first interface is true */
	snprintf(attrpath, sizeof(attrpath), "%s/%s:%d.%d/%s",
			edev->udev.path,
			edev->udev.busid,
			edev->udev.bConfigurationValue, 0,
			"usbip_close");
	LLOG_ERR("usbip_stub_close_device: open %s\n ",attrpath);
	attr = sysfs_open_attribute(attrpath);
	if (!attr) {
		err("open %s", attrpath);
		return -1;
	}

	ret = sysfs_write_attribute(attr, buff, 8);
	if (ret < 0) {
		err("close to %s", attrpath);
		goto err_write_close;
	}

	info("disconnect %s", edev->udev.busid);
printf("disconnect %s\n", edev->udev.busid);

err_write_close:
	sysfs_close_attribute(attr);

	return ret;
}

#if 0
int usbip_stub_export_device(struct usbip_exported_device_linux *edev, Udev_and_sockfd *pDev_fd)
{
	char attrpath[SYSFS_PATH_MAX];
	struct sysfs_attribute *attr;
	int ret;


	if (edev->status != SDEV_ST_AVAILABLE) {
		info("device not available, %s", edev->udev.busid);
		switch( edev->status ) {
			case SDEV_ST_ERROR:
				info("     status SDEV_ST_ERROR");
				break;
			case SDEV_ST_USED:
				info("     status SDEV_ST_USED");
				break;
			default:
				info("     status unknown: 0x%x", edev->status);
		}
		return -1;
	}

	/* only the first interface is true */
	snprintf(attrpath, sizeof(attrpath), "%s/%s:%d.%d/%s",
			edev->udev.path,
			edev->udev.busid,
			edev->udev.bConfigurationValue, 0,
			"usbip_sockfd");
	printf("usbip_stub_export_device: open %s\n ",attrpath);
	attr = sysfs_open_attribute(attrpath);
	if (!attr) {
		err("open %s", attrpath);
		return -1;
	}

	char *buf = (char *)pDev_fd;
	ret = sysfs_write_attribute(attr, buf, sizeof(Udev_and_sockfd));
	if (ret < 0) {
		err("write sockfd %d to %s", pDev_fd->clientfd, attrpath);
		goto err_write_sockfd;
	}

	info("connect %s", edev->udev.busid);

err_write_sockfd:
	sysfs_close_attribute(attr);

	return ret;
}
#endif

int usbip_stub_export_device(struct usbip_exported_device_linux *edev, int sockfd)
{
	char attrpath[SYSFS_PATH_MAX];
	struct sysfs_attribute *attr;
	char sockfd_buff[30];
	int ret;

	/* only the first interface is true */
	snprintf(attrpath, sizeof(attrpath), "%s/%s:%d.%d/%s",
			edev->udev.path,
			edev->udev.busid,
			edev->udev.bConfigurationValue, 0,
			"usbip_sockfd");
	attr = sysfs_open_attribute(attrpath);
	if (!attr) {
		err("open %s", attrpath);
		return -1;
	}

	snprintf(sockfd_buff, sizeof(sockfd_buff), "%d\n", sockfd);

	ret = sysfs_write_attribute(attr, sockfd_buff, strlen(sockfd_buff));
	if (ret < 0) {
		err("write sockfd %s to %s", sockfd_buff, attrpath);
		goto err_write_sockfd;
	}

	info("connect %s", edev->udev.busid);

err_write_sockfd:
	sysfs_close_attribute(attr);

	return ret;
}


struct usbip_exported_device_linux *usbip_stub_get_device(int num)
{
	struct usbip_exported_device_linux *edev;
	struct dlist		*dlist = stub_driver->edev_list;
	int count = 0;

	dlist_for_each_data(dlist, edev, struct usbip_exported_device_linux) {
		if (num == count)
			return edev;
		else
			count++ ;
	}

	return NULL;
}
