#ifndef __VACCESS_H
#define __VACCESS_H

#include <types.h>

int setnonblocking(int fd);
int add_to_epoll(int fd, int events);
int _send(int fd, char *out, int size);
gboolean listener_init();
int wait_for_connect();

#endif
