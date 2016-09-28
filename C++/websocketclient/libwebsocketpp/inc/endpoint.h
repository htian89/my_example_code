#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

#include "client.h"
#include "connection.h"
#include "uri.h"
#include <semaphore.h>

#define MAX_CONNECTION_COUNT 1
#define ENDPOINT_HANDLE  struct _ENDPOINT_s *

typedef struct
{
	struct _CONNECTION_s * hConn;
	int used;
}EP_Connection_Pool_s;

typedef struct _ENDPOINT_s
{
	EP_Connection_Pool_s connection_pool[MAX_CONNECTION_COUNT];
	struct _CLIENT_s * hClient;	

	RS_s (*create_connection)(ENDPOINT_HANDLE, URI_Handle);
	RS_s (*remove_connection)(ENDPOINT_HANDLE, URI_Handle);
	
}ENDPOINT_s, *ENDPOINT_Handle;

RS_s endpoint_init(ENDPOINT_Handle *hEp);
RS_s endpoint_uninit(ENDPOINT_Handle hEp);

#endif