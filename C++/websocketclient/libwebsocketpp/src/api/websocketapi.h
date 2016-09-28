#ifndef __WEBSOCKETAPI_H__
#define __WEBSOCKETAPI_H__

typedef int WS_HANDLE;

typedef struct _WS_Conn_Param_s
{
	int secure;  // if this is a secure connection
	int port;    // the port
	char *host;  // the host
	char *resource;  // the resource

	int sub_protocol_count;
	char **sub_protocol;
	char *origin;
}WS_Conn_Param_s;

WS_HANDLE ws_connection(int socketfd, const char * ip, const int port);
int ws_recv(WS_HANDLE handle, char *payload, int len);
int ws_send(WS_HANDLE handle, const char *payload, int len);
int ws_disconnection(WS_HANDLE handle);
int ws_can_recv(WS_HANDLE handle);

void ws_debug_enable(WS_HANDLE handle);

#endif
