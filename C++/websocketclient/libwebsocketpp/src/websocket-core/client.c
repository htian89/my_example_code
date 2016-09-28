#include "client.h"
#include "connection.h"
#include "base64.h"
#include "common.h"
#include "socket.h"
#include "http.h"
#include "frame.h"


#define LOCK(lock)  {do {sem_wait(&lock);}while (0);}
#define UNLOCK(lock)  {do {sem_post(&lock);}while (0);}


static RS_s set_status(CLIENT_Handle hClient, STATUS_e status)
{
	LOCK(hClient->lock);
	hClient->eStatus = status;
	UNLOCK(hClient->lock);

	return RS_OK;
}

static STATUS_e get_status(CLIENT_Handle hClient)
{
	STATUS_e temp;
	
	LOCK(hClient->lock);
	temp = hClient->eStatus;
	UNLOCK(hClient->lock);

	return temp;
}


static RS_s write_request(CLIENT_Handle hClient, int waitTime)
{
	HTTP_Request_Handle hRequest = NULL;
	HTTP_Common_Handle hCommon = NULL;
	RS_s ret = RS_OK;

	DBG_PRINT("START...\n");

	ret = http_request_init(&hRequest);
	RETURNIF((ret != RS_OK || hRequest == NULL), ret, "");

	DBG_PRINT("http_request_init finished...\n");
	do 
	{
		int flag = 0;
		int i = 0;
		int raw_key[4];
		char handshake_key[50];
		char request_raw[512];
		
		hCommon = hRequest->hCommon;
		RETURNIF(hCommon == NULL, RS_ERROR, "");
		CHECK_RUN(hRequest->set_method(hRequest, "GET"));
		CHECK_RUN(hRequest->set_uri(hRequest, hClient->hUri->resource));
		CHECK_RUN(hCommon->set_version(hCommon, "HTTP/1.1"));
		CHECK_RUN(hCommon->add_header(hCommon, "Upgrade", "websocket"));
		CHECK_RUN(hCommon->add_header(hCommon, "Connection", "Upgrade"));
		CHECK_RUN(hCommon->add_header(hCommon, "Sec-WebSocket-Version", "13"));
		CHECK_RUN(hCommon->add_header(hCommon, "Host", hClient->hUri->host));
		CHECK_RUN(hCommon->add_header(hCommon, "Origin", hClient->origin));

		while (i < hClient->sub_protocol_count)
		{
			DBG_PRINT("hClient->sub_protocol[%d]:%s\n", i, hClient->sub_protocol[i]);
			if (!flag)
			{
				CHECK_RUN(hCommon->add_header(hCommon, "Sec-WebSocket-Protocol", hClient->sub_protocol[i]));
				flag = 1;
			}
			else
			{
		
				CHECK_RUN(hCommon->add_header(hCommon, "Sec-WebSocket-Protocol", hClient->sub_protocol[i]));
			}
			i++;
		}

		for (i = 0; i < 4; i++)
		{
			raw_key[i] = my_rand(0xFFFFFFFF);
			usleep(1);
		}
		snprintf(handshake_key, 50, "%d-%d-%d-%d", raw_key[0], raw_key[1], raw_key[2], raw_key[3]);
	//	b64_encode(raw_key, 4*4, handshake_key, 50);
		
		DBG_PRINT("handshake_key--->%s\n", handshake_key);
		CHECK_RUN(hCommon->add_header(hCommon, "Sec-WebSocket-Key", handshake_key));
		CHECK_RUN(hCommon->add_header(hCommon, "User-Agent", "FW-websocket"));
		
		memset(request_raw, 0, 512);
		CHECK_RUN(hRequest->raw(hRequest, request_raw, 512));

		//this will change to which using connection write	
		if (hClient->hUri->secure)
		{
			//CHECK_RUN(socket_send(int socketfd, char * buffer, int len))
		}
		else
		{
			DBG_PRINT("request_raw data is :\n%s\n", request_raw);
			socket_send(hClient->socketfd, request_raw, strlen(request_raw));
		}        
		
		http_request_uninit(hRequest);
//		DBG_PRINT("END...\n");
		return RS_OK;
		
	}while (0);

	http_request_uninit(hRequest);
	return RS_FAIL;
}

static RS_s handle_response(CLIENT_Handle hClient, int waitTime)
{
	char response[512];
	HTTP_Response_Handle hResponse = NULL;
	RS_s ret = RS_OK;

	memset(response, 0, 512);
	if (socket_recv(hClient->socketfd, response, 512) < 0)
	{
		return RS_FAIL;	
	}

	DBG_PRINT("server response is:\n%s\n", response);

//	memset(response, 0, 512);
//	socket_recv(hClient->socketfd, response, 512);
//	DBG_PRINT("server data is:\n%s\n", response);  

	ret = http_response_init(&hResponse);
	RETURNIF(ret != RS_OK, ret, "");
	
	return hResponse->parse(hResponse, response, 512);;
}



static RS_s __connection(CLIENT_Handle hClient)
{
	RETURNIF(hClient == NULL, RS_ERROR, "");
	RS_s ret = RS_OK;
	DBG_PRINT("START....\n");
	ret = write_request(hClient, 1000);
	RETURNIF(ret != RS_OK, ret, "");
	ret = handle_response(hClient, 1000);
	RETURNIF(ret !=RS_OK, ret, "");
	DBG_PRINT("END....\n");
	return RS_OK;
}

static RS_s __dis_connection(CLIENT_Handle hClient)
{
	FRAME_Handle hFrame = NULL;
	char *reason = "hahahah";

	DBG_PRINT("...\n");
	
	hClient->set_status(hClient, CLOSING);
		
	frame_init(&hFrame);

	hFrame->prepare_close_frame(hFrame, reason, strlen(reason));	
	send_frame(hClient->socketfd, hFrame);

	hFrame->reset(hFrame);
	//recv_frame(hClient->socketfd, hFrame);

	//hFrame->process_close_frame(hFrame);

	hClient->set_status(hClient, CLOSED);
	
	return RS_OK;
}

static RS_s __send(CLIENT_Handle hClient, const char *payload, U64 len)
{
	RS_s ret = RS_OK;
	FRAME_Handle hFrame = NULL;

//	DBG_PRINT("START..1..\n");
	
	ret = frame_init(&hFrame);
	RETURNIF(ret != RS_OK, ret, "");
		
	ret = hFrame->prepare_data_frame(hFrame, UTF_8_STR, payload, len);
	RETURNIF(ret != RS_OK, ret, "");
	
	// print_frame( hFrame);
	//////test
	send_frame(hClient->socketfd, hFrame);

//	DBG_PRINT("END....\n");

	ret = frame_uninit(hFrame);
	
	return RS_OK;
		
}


static int  __recv(CLIENT_Handle hClient, const char *payload, int len)
{
	RS_s ret = RS_OK;	
	FRAME_Handle hFrame = NULL;

	ret = frame_init(&hFrame);
	RETURNIF(ret != RS_OK, -1, "");
	
	ret = recv_frame(hClient->socketfd, hFrame);
	RETURNIF(ret != RS_OK, -1, "");

	if (len >= hFrame->content.payload_size)
	{
		memcpy((char *)payload, hFrame->content.payload_data, hFrame->content.payload_size);
		return hFrame->content.payload_size;
	}
	else
	{
		DBG_PRINT("*****Lost %d bytes data\n", hFrame->content.payload_size - len);
		memcpy((char *)payload, hFrame->content.payload_data, len);
		return len;
	}
	//print_frame(hFrame);

	return 0;
}

static RS_s  __add_sub_protocol(CLIENT_Handle hClient,  char **sub_protocol)
{
	RETURNIF(hClient->sub_protocol_count == 0, RS_OK, "");
	int i = 0; 
	int ret = 0;
	int len;
	
	ret = __check_malloc((char **)&hClient->sub_protocol, hClient->sub_protocol_count * sizeof(char *), MALLOC_NORMAL);
	RETURNIF(ret != RS_OK,  RS_ERROR, "");

	DBG_PRINT("hClient->sub_protocol_count:%d\n", hClient->sub_protocol_count);
	for (i = 0; i < hClient->sub_protocol_count; i++)
	{
	    len = strlen(sub_protocol + i) + 1;
		ret = __check_malloc(&hClient->sub_protocol[i], len, MALLOC_EXTEND);
		RETURNIF(ret != RS_OK,  RS_ERROR, "");
		strncpy(hClient->sub_protocol[i], sub_protocol+i, len);
	}

	return RS_OK;
}

static RS_s __set_origin(CLIENT_Handle hClient, char *origin)
{
	
	if (!origin)
	{
		return RS_FAIL;
	}

	hClient->origin = (char *)my_malloc(strlen(origin) + 1);
	if (!hClient->origin)
		return RS_FAIL;

	strcpy(hClient->origin, origin);
	
	return RS_OK;
}

static RS_s release_sub_protocol(CLIENT_Handle hClient)
{
	int i = 0;

	for (i = 0; i < hClient->sub_protocol_count; i++)
	{
		if (hClient->sub_protocol[i])
		{
			my_free(hClient->sub_protocol[i]);
		}
		else
		{
			WARNING("hClient->sub_protocol_count[i] should not be null");
		}
	}

	return RS_OK;
}

RS_s client_init(CLIENT_Handle *hClient)
{
	CLIENT_Handle _hClient = NULL;
	URI_Handle hUri;
	ENDPOINT_Handle hEp;
//	DBG_PRINT("start...\n");
	do 
	{
		_hClient = (CLIENT_Handle)my_malloc(sizeof(CLIENT_s));
		RETURNIF(_hClient == NULL, RS_ERROR, "");

		CHECK_RUN(uri_init(&hUri));
		CHECK_RUN(endpoint_init(&hEp));
		RETURNIF(sem_init(&_hClient->lock, 0, 1) != 0, RS_ERROR, "");
		
		_hClient->connection = __connection;
		_hClient->send = __send;
		_hClient->recv = __recv;
		_hClient->add_sub_protocol = __add_sub_protocol;
		_hClient->dis_connection = __dis_connection;
		_hClient->get_status = get_status;
		_hClient->set_status = set_status;
		_hClient->set_origin = __set_origin;
			
		_hClient->hUri = hUri;
		_hClient->hEp = hEp;
		
		_hClient->init_ok = 1;
		 set_status(_hClient, CLOSED);
		*hClient = _hClient;
		return RS_OK;
	}while (0);

	return RS_FAIL;
}

RS_s client_uninit(CLIENT_Handle hClient)
{

	if (hClient->hEp)
		endpoint_uninit(hClient->hEp);

	if (hClient->hUri)
		uri_uninit(hClient->hUri);

	if (hClient->origin)
		my_free(hClient->origin);

	release_sub_protocol(hClient);

	my_free((char *)hClient);

	return RS_OK;
}





