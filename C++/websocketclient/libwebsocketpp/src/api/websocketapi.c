#include "websocketapi.h"
#include "client.h"
#include "connection.h"
#include "base64.h"
#include "common.h"
#include "socket.h"
#include "http.h"

WS_HANDLE ws_connection(int socketfd, const char * ip, const int port)
{
	CLIENT_Handle hClient = NULL;
	int i = 0;
//	DBG_PRINT("start...\n")
	do 
	{
		CHECK_RUN(client_init(&hClient));

		if (socketfd) 
		{
			hClient->socketfd = socketfd;
		}
		else if (socketfd <= 0)
		{
			// socket ����
		}

		char sub_protocol[][5] = {{"char"}};

		hClient->hUri->secure = 0;
		hClient->hUri->port = port;
		hClient->hUri->set_host(hClient->hUri, ip);
		hClient->hUri->set_resource(hClient->hUri, "/char");
		hClient->sub_protocol_count = 1;
		hClient->add_sub_protocol(hClient, sub_protocol);
		hClient->set_origin(hClient, "http://zaphoyd.com");
		hClient->set_status(hClient, CONNECTING);
		CHECK_RUN(hClient->connection(hClient));
		hClient->set_status(hClient, CONNECTED);

		DBG_PRINT("WS_HANDLE:0x%08x\n", hClient);
		return (WS_HANDLE)(hClient);	
	}while (0);

	if (hClient)
	{
		client_uninit(hClient);
	}
//	DBG_PRINT("end...\n");
	return 0;
	
}

void ws_debug_enable(WS_HANDLE handle)
{
	set_debug_enable();
}

int ws_recv(WS_HANDLE handle, char *payload, int len)
{
	CLIENT_Handle hClient = (CLIENT_Handle)handle;

	if (ws_can_recv(handle))
	{
		return hClient->recv(hClient, payload, len);
	}

	return -1;
}

int ws_send(WS_HANDLE handle, const char *payload, int len)
{
	CLIENT_Handle hClient = (CLIENT_Handle)handle;

	hClient->send(hClient, payload, len);
	return 0;
}

int ws_disconnection(WS_HANDLE handle)
{
	CLIENT_Handle hClient = (CLIENT_Handle)handle;
	hClient->dis_connection(hClient);
	while (ws_can_recv(handle))
	{
		DBG_PRINT("wait ws_can_recv NULL\n");
		sleep(1);
	}
	client_uninit(hClient);
	return 0;
}

int ws_can_recv(WS_HANDLE handle)
{
	CLIENT_Handle hClient = (CLIENT_Handle)handle;
//	DBG_PRINT("status:%d\n", hClient->get_status(hClient));
	if (hClient->get_status(hClient) == CONNECTED)
	{
		return 1;
	}
	
	return 0;
}

