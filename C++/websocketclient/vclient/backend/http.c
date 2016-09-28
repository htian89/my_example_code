#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "tcp.h"
#include "../common/ds_vclient.h"

#include "../common/log.h"

static NETWORK *HttpParseUrl(const char *url, char *uri)
{
    NETWORK *network;
	char *p, *str;
	char tmp[512];
	memset(tmp,0, sizeof(tmp));
	strcpy(tmp, url);

    network =  (NETWORK *)malloc(sizeof(NETWORK));

	str = tmp;
	if ((p = strstr(str, "://")) != NULL)
	{
		char https[6];
		memcpy(https, str, strlen(str) - strlen(p));
		https[strlen(str) - strlen(p)] = '\0';
		if (strcmp(https, "https") == 0)
            network->stPresentServer.isHttps = 1;//network->isHttps = 1;
		else if (strcmp(https, "http") == 0)
            network->stPresentServer.isHttps = 0;//network->isHttps = 0;
		str += strlen(https) + 3;
	}

	if ((p = strstr(str, ":")) != NULL)
	{
        memcpy(network->stPresentServer.serverAddress, str, strlen(str) - strlen(p));
        network->stPresentServer.serverAddress[strlen(str) - strlen(p)] = '\0';
        str += strlen(network->stPresentServer.serverAddress) + 1;
//		memcpy(network->presentServer, str, strlen(str) - strlen(p));
//		network->presentServer[strlen(str) - strlen(p)] = '\0';
//		str += strlen(network->presentServer) + 1;
	}

	if ((p = strstr(str, "/")) != NULL)
	{
        memcpy(network->stPresentServer.port, str, strlen(str) - strlen(p));
        network->stPresentServer.port[strlen(str) - strlen(p)] = '\0';
        str += strlen(network->stPresentServer.port);
//		memcpy(network->port, str, strlen(str) - strlen(p));
//		network->port[strlen(str) - strlen(p)] = '\0';
//		str += strlen(network->port);
	}

	memcpy(uri, str, strlen(str));
	uri[strlen(str)] = '\0';

	return network;
}

static int HttpCommon(const char *url, const char *input,
			   char *output, int *outputLength,
			   const char *httpStyle, int isGetDelete)
{
    NETWORK *network;
    TcpConnection *connection = NULL;
	char uri[1024];
	char str[1024];
	int errorCode;

	char http_request_data[2048];


	memset(http_request_data, 0, 2048);
	memset(uri, 0, 1024);


	network = HttpParseUrl(url, uri);
	if ((connection = TcpConnect(network)) == NULL)
	{
#ifdef DEBUG_MODE
	printf("error in tcpConnect function\n");
#endif
        LOG_ERR("tcp connect failed");
		return -1;
	}

	strcpy(http_request_data, httpStyle);


	strcat(http_request_data, " ");
	strcat(http_request_data, uri);

	if (isGetDelete)
	{
		if (input != NULL)
		{
            sprintf(str, "?%s", input);
			strcat(http_request_data,str);
		}
	}

    strcat(http_request_data, " HTTP/1.1\r\n");
    strcat(http_request_data, "User-Agent: Mozilla/5.0\r\n");

    if (isGetDelete)
        sprintf(str, "Host: %s:%s\r\n", network->stPresentServer.serverAddress, network->stPresentServer.port);//sprintf(str, "Host: %s:%s\r\n", network->presentServer, network->port);
    else
    {
        sprintf(str, "Host: %s\r\n", network->stPresentServer.serverAddress);//sprintf(str, "Host: %s\r\n", network->presentServer);
        strcat(http_request_data, str);
        if(NULL!= input)
            sprintf(str, "Content-Length: %d\r\n", strlen(input));
        else
            strcpy(str, "Content-Length: 0\r\n");
    }
    strcat(http_request_data, str);

    if (!isGetDelete)
        strcat(http_request_data,"Content-Type: application/xml\r\n");
    strcat(http_request_data,"\r\n");
    if (!isGetDelete && NULL!=input)
        strcat(http_request_data,input);

    printf("http_request_data;%s\n", http_request_data);
    TcpSendSocket(connection, network->stPresentServer.isHttps, http_request_data);//TcpSendSocket(connection, network->isHttps, http_request_data);
    errorCode = TcpReceiveSocket(connection, network->stPresentServer.isHttps, output, outputLength);//errorCode = TcpReceiveSocket(connection, network->isHttps, output, outputLength);
    if(errorCode == -1)
	{
#ifdef DEBUG_MODE
	printf("error in TcpReceiveSocket function \n");
#endif
	}
    TcpDisconnect(connection, network->stPresentServer.isHttps);//TcpDisconnect(connection, network->isHttps);
    free(network);

	return errorCode;
}

int HttpPost(const char *url, const char *input, char *output, int *outputLength)
{
	return HttpCommon(url, input, output, outputLength, "POST", 0);
}

int HttpPut(const char *url, const char *input, char *output, int *outputLength)
{
	return HttpCommon(url, input, output, outputLength, "PUT", 0);
}

int HttpGet(const char *url, const char *input, char *output, int *outputLength)
{
	return HttpCommon(url, input, output, outputLength, "GET", 1);
}

int HttpDelete(const char *url, const char *input, char *output, int *outputLength)
{
	return HttpCommon(url, input, output, outputLength, "DELETE", 1);
}
