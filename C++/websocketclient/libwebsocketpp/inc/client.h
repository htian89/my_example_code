#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "endpoint.h"
#include "uri.h"
#include "common.h"

#define CLIENT_HANDLE struct _CLIENT_s *

typedef enum
{
	CONNECTING, 
	CONNECTED,
	CLOSING,
	CLOSED,
}STATUS_e;

typedef struct _CLIENT_s
{
	int socketfd;
	//SSL_Handle hSLL;
	struct _ENDPOINT_s * hEp;
	struct _URI_s * hUri;
	
	char *origin;
	char **sub_protocol;
	int sub_protocol_count;

	int private_data_len;
	char *private_data;

	int init_ok;
	STATUS_e eStatus;

	sem_t lock;

	RS_s (*add_sub_protocol)(CLIENT_HANDLE, char **);
	RS_s (*connection)(CLIENT_HANDLE);
	RS_s (*send)(CLIENT_HANDLE, char *payload, U64 len);
	int (*recv)(CLIENT_HANDLE, const char *payload, int len);
	RS_s (*dis_connection)(CLIENT_HANDLE);
	STATUS_e (*get_status)(CLIENT_HANDLE);
	RS_s (*set_status)(CLIENT_HANDLE, STATUS_e status);
	RS_s (*set_origin)(CLIENT_HANDLE, char *origin);
	
}CLIENT_s, *CLIENT_Handle;

RS_s client_init(CLIENT_Handle *hClient);
RS_s client_uninit(CLIENT_Handle hClient);
#endif