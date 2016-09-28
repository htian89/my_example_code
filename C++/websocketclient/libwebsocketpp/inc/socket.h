#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "common.h"

//_s socket_send(int socketfd, char * buffer, int len);
int socket_send(int socketfd, char * buffer, int len);

int socket_recv(int socketfd, char *buffer, int len);

#endif