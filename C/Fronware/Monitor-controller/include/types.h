#ifndef __TYPES_H
#define __TYPES_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <sys/epoll.h>

#define IP_SIZE 16
#define MAX_EVENTS 128
#define BUFSIZE 8192

#define MSG_HEADER_LEN 24

#define LLOG_ERR(fmt, args...) \
    printf("ERR: %13s:%4d %-12s: " fmt "\n", \
        __FILE__, __LINE__, __FUNCTION__, ##args)

#define DEBUG
#ifdef DEBUG
#define LLOG(fmt, args...) \
    printf("%13s:%4d %-12s: " fmt "\n", \
        __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define LLOG(_args, ...) \
    (void)(0)
#endif

enum{
    NO_ERROR = 0,
    CONNECT_FAILED = 3100,
    NO_THIS_MONITOR,
    DISCONNECT_FREE_MONITOR,
    MSG_FORMAT_ERR,
    UNSUPPORT_ACTION,
};

enum{
    FREE_MONITOR = 0,
    USED_MONITOR,
};

enum{
    CONNECT_BY_FAP = 0,
    CONNECT_BY_VNC,
    CONNECT_BY_CAMERA,
};

typedef struct Controller {
    int epoll_fd;
    int listener;
    int vaccess_fd;
    GdkScreen *screen;
    int monitor_num;
    char recvbuf[BUFSIZE];
    char sendbuf[BUFSIZE];
} Controller;

typedef struct Monitor {
    int number; 
    GdkRectangle geometry;
    int8_t status;
    int8_t mode;
    char targetIp[IP_SIZE];
    int targetPort;
    pid_t pid;
} Monitor;

extern Controller *controller;
extern Monitor *monitor;

#endif
