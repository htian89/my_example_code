#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "common.h"
#include "endpoint.h"

#define CONN_HANDLE  struct _CONNECTION_s *

typedef struct _CONNECTION_s 
{
	int start;
	struct _URI_s * hUri;
	struct _ENDPOINT_s * hEp;
	
	RS_s (*connection)(CONN_HANDLE);	
	RS_s (*write)(CONN_HANDLE, const unsigned char *);
	RS_s (*read)(CONN_HANDLE, const unsigned char *);
}CONNECTION_s, * CONN_Handle;

RS_s new_connection(CONN_Handle *hConn, int socketfd);

RS_s free_connection(CONN_Handle hConn);

#endif
