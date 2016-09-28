#include "http.h"
#include "common.h"
/*

typedef struct 
{
	HTTP_RequestLine_s *request_line;
	HTTP_Header_s *header;
	char *raw_data;
	int raw_data_len;

	RS_s (*set_method)(const char * );
	RS_s (*get_method)(char *);
	RS_s (*set_uri)(const char *);
	RS_s (*get_uri)(char *);
	RS_s (*set_version)(const char *);
	RS_s (*get_version)(char *);
	RS_s (*add_header)(const char *, const char *);
	RS_s (*get_header)(const char *, char *);
	RS_s (*replace_header)(const char *, const char *);
	RS_s (*remove_header)(const char *);
	char * (*raw)(void);
}HTTP_s, *HTTP_Handle;

*/

static int __get_line(char *dest, char *src, int dest_len)
{
	RETURNIF(!(dest || src), -1, "");
	char *p_src = src;
	int line_len = 0;
	
	while (*p_src != '\0')
	{
		if (*p_src == '\n')
		{
			if (*p_src == '\r')
			{
				p_src --;
			}
			line_len = p_src-src;
			RETURNIF(line_len > dest_len, -1, "");
			snprintf(dest, line_len, "%s", src);
		}
		p_src++;
	}
	return line_len;
}

static int __find_location(HTTP_Common_Handle hCommon, const char *key)
{
	int index  = 0;
	int free_index = -1;
	char *_key = NULL;

	for (index = 0; index < MAX_HEADER_COUNT; index ++)
	{
		_key = hCommon->header[index].key;
		if (_key != NULL)
		{
			if (!strcmp(key, _key))
			{
				return index;
			}
		}
		else if (free_index == -1)
		{
			free_index = index;
		}
	}

	return free_index;
}

static RS_s __common_set(const char *src, char **p_dest)
{
	RETURNIF(src == NULL, RS_ERROR, "");
	
	int len = strlen(src)+1;
	RS_s ret = RS_OK;
	
	ret = __check_malloc(p_dest, len, MALLOC_NORMAL);
	RETURNIF(ret != RS_OK, ret, "");
	strncpy(*p_dest,src, len);

	return RS_OK;
}


static RS_s __set_method(HTTP_Request_Handle hHttp,  const char *method)
{
	return __common_set(method, &hHttp->method);
}

static RS_s __set_uri(HTTP_Request_Handle hHttp, const char *uri)
{
	return __common_set(uri, &hHttp->uri);
}
static RS_s __set_version(HTTP_Common_Handle hCommon,  const char *version)
{
	return __common_set(version, &hCommon->version);
}

static RS_s __add_header(HTTP_Common_Handle hCommon, const char *key, const char *val)
{
	RETURNIF(hCommon == NULL || key == NULL  || val == NULL, RS_ERROR, "");
	int index = 0;
	RS_s ret = RS_OK;
	int len_key = strlen(key)+1;
	int len_val = strlen(val)+1 + 1; //may be need to add ","

//	DBG_PRINT("start...\n");
	index = __find_location(hCommon, key);
	RETURNIF(index == -1, RS_FAIL, "");

	ret = __check_malloc(&hCommon->header[index].key, len_key, MALLOC_NORMAL);
	RETURNIF(ret != RS_OK, ret, "");

	ret = __check_malloc(&hCommon->header[index].value, len_val, MALLOC_EXTEND);
	RETURNIF(ret != RS_OK, ret, "");

	if (strlen(hCommon->header[index].key) == 0)
	{
		strncpy(hCommon->header[index].key, key, len_key);
	}

	if (strlen(hCommon->header[index].value) == 0)
	{
		strncpy(hCommon->header[index].value,val, len_val);
	}
	else 
	{
		strcat(hCommon->header[index].value, ",");
		strcat(hCommon->header[index].value, val);
	}

//	DBG_PRINT("end...\n");

	return RS_OK;
}

#if 1
static char * __get_header(HTTP_Common_Handle hCommon)
{

}

static RS_s __replace_header(HTTP_Common_Handle hCommon, const char *key, const char *val)
{
	RETURNIF(hCommon == NULL, RS_ERROR, "");

	int index = 0;
	RS_s ret = RS_OK;
	int len = strlen(val) + 1;

	index = __find_location(hCommon, key);
	RETURNIF(index == -1, RS_FAIL, "");

	RETURNIF(hCommon->header[index].key == NULL, RS_ERROR, "");
	ret = __check_malloc(&hCommon->header[index].value, len, MALLOC_REMALLOC);
	RETURNIF(ret != RS_OK, ret, "");

	strncpy(hCommon->header[index].value, val, len);

	return RS_OK;
}
#endif

static RS_s __raw_request(HTTP_Request_Handle hRequest, char *buffer, int len)
{
	RETURNIF(hRequest == NULL || buffer == NULL, RS_ERROR, "");
	int i = 0;

	RETURNIF (hRequest->method == NULL || hRequest->uri== NULL || hRequest->hCommon->version == NULL, RS_ERROR, "");
	snprintf(buffer, len, "%s %s %s \r\n", hRequest->method, hRequest->uri, hRequest->hCommon->version);

	for (i = 0; i < MAX_HEADER_COUNT; i++)
	{
		if ((hRequest->hCommon->header[i].key) && (hRequest->hCommon->header[i].value))
		{
			//snprintf(raw, len, "%s:%s\r\n", hHttp->header[i].key, hHttp->header[i].value);
			strcat(buffer,  hRequest->hCommon->header[i].key);
			strcat(buffer, ": ");
			strcat(buffer, hRequest->hCommon->header[i].value);
			strcat(buffer, "\r\n");
		}
	}

	strcat(buffer, "\r\n");
	return RS_OK;
}

static RS_s __raw_response(HTTP_Response_Handle hResponse, char *buffer, int len)
{
	return RS_OK;
}

static inline char * __get_str(char *buf, char *dst, char * flag)
{
	char *sub;

	if (buf == NULL || dst == NULL)
		return NULL;

	buf[0] = '\0';
	sub = strstr(dst, flag);
	if (sub == NULL) {
		DBG_PRINT("just 1  ...\n");
		return NULL;
	}

	snprintf(buf, sub-dst+1, "%s", dst);
	return sub;
}

static RS_s __parse_response(HTTP_Response_Handle hResponse, char *buffer, int len)
{
	char *p_buf = buffer;
        char *sub;
 	char temp[100];

	 sub = __get_str(temp, p_buf, "/");
	 if (!sub) {
	 	DBG_PRINT("find / error\n");
		return RS_FAIL;
	 }

	p_buf = sub+1;	
	if (strcmp("HTTP", temp) != 0) {
		DBG_PRINT("http header fail\n");
		return RS_FAIL;
	}
		
	 sub = __get_str(temp, p_buf, " ");
	 if (!sub) {
	 	DBG_PRINT("find bb error\n");
		return RS_FAIL;
	 }
	p_buf = sub+1;	
	if (strcmp("1.1", temp) != 0) {
		DBG_PRINT("http version fail\n");
		return RS_FAIL;
	}

	sub = __get_str(temp, p_buf, " ");
	 if (!sub) {
	 	DBG_PRINT("find bb error\n");
		return RS_FAIL;
	 }
	p_buf = sub+1;	
	if (strcmp("101", temp) != 0) {
		DBG_PRINT("server response error\n");
		return RS_FAIL;
	}

	sub = __get_str(temp, p_buf, "\n");
	 if (!sub) {
	 	DBG_PRINT("find \\n error\n");
		return RS_FAIL;
	 }
	p_buf = sub+1;	
	
//	if (strcmp("Switching Protocols", temp) != 0) {
//		DBG_PRINT("Switching Protocols error\n");
//		return RS_FAIL;
//	}
	
	return RS_OK;
}

static RS_s __set_status_msg(HTTP_Response_Handle hResponse, const char *status_msg)
{
	return RS_OK;
}

static RS_s http_common_init(HTTP_Common_Handle *hCommon)
{
	int i = 0;
	RS_s ret = RS_OK;
	HTTP_Common_Handle _hCommon = NULL;
	
	_hCommon = (HTTP_Common_Handle)my_malloc(sizeof(HTTP_Common_s));
	RETURNIF(ret != RS_OK, ret, "");
	
	_hCommon->add_header = __add_header;
//	_hCommon->remove_header 
	_hCommon->get_header = __get_header;
	_hCommon->replace_header = __replace_header;
	_hCommon->set_version = __set_version;

	*hCommon = _hCommon;	

	return RS_OK;
}

static RS_s http_common_uninit(HTTP_Common_Handle hCommon)
{
	int i = 0;
	
	if (hCommon->version)
	{
		my_free(hCommon->version);
	}

	for (i = 0; i < MAX_HEADER_COUNT; i++)
	{
		if (hCommon->header[i].key)
		{
			my_free(hCommon->header[i].key);
		}

		if (hCommon->header[i].value)
		{
			my_free(hCommon->header[i].value);
		}
	}

	my_free((char *)hCommon);
	
	return RS_OK;
}

RS_s http_request_init(HTTP_Request_Handle *hRequest)
{
	HTTP_Request_Handle _hRequest;
	int ret = RS_OK;

//	DBG_PRINT("START...\n");
	_hRequest = (HTTP_Request_Handle)my_malloc(sizeof(HTTP_Request_s));
	RETURNIF(_hRequest == NULL, RS_ERROR, "");

	ret = http_common_init(&_hRequest->hCommon);
	RETURNIF(ret != RS_OK,  RS_ERROR, "");

	_hRequest->set_method = __set_method;
	_hRequest->set_uri = __set_uri;
	_hRequest->raw = __raw_request;
	*hRequest  = _hRequest;

//	DBG_PRINT("END...\n");
	return RS_OK;
}

RS_s http_request_uninit(HTTP_Request_Handle hRequest)
{
	
	if (hRequest->method)
	{
		my_free(hRequest->method);
	}

	if (hRequest->uri)
	{
		my_free(hRequest->uri);
	}
	
	if (hRequest->hCommon)
	{
		http_common_uninit(hRequest->hCommon);
	}

	return RS_OK;
}

RS_s http_response_init(HTTP_Response_Handle *hHttpResponse)
{
	HTTP_Response_Handle _hHttpResponse = NULL;
	RS_s ret = RS_OK;

	_hHttpResponse = (HTTP_Response_Handle)my_malloc(sizeof(HTTP_Request_s));
	RETURNIF(_hHttpResponse == NULL, RS_ERROR, "");

	ret = http_common_init(&_hHttpResponse->hCommon);
	RETURNIF(ret != RS_OK,  RS_ERROR, "");

	_hHttpResponse->parse = __parse_response;
//	_hHttpResponse->raw = __raw_response;
	_hHttpResponse->set_status_msg = __set_status_msg;

	*hHttpResponse = _hHttpResponse;

	return ret;
}

RS_s http_response_uninit(HTTP_Response_Handle hHttpResponse)
{
	
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////T  E   S   T ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#define my_malloc malloc
int main(int argc, void **argv)
{
	HTTP_Handle hRequest = NULL;
	RS_s ret = RS_OK;
	char raw_data[512];

	char *sub_protocol[5] = 
	{
		"chat0", 
		"chat1", 
		"chat2", 
		"chat3", 
		"chat4", 
	};

	ret = http_init(&hRequest);
	RETURNIF((ret != RS_OK || hRequest == NULL), ret, "");

	do 
	{
		int flag = 0;
		int i = 0;
		int raw_key[4];
		char handshake_key[50];
		
		CHECK_RUN(hRequest->set_method(hRequest, "Get"));
		CHECK_RUN(hRequest->set_uri(hRequest, "/chat"));
		CHECK_RUN(hRequest->set_version(hRequest, "HTTP/1.1"));
		CHECK_RUN(hRequest->add_header(hRequest, "Upgrade", "websocket"));
		CHECK_RUN(hRequest->add_header(hRequest, "Connection", "Upgrade"));
		CHECK_RUN(hRequest->add_header(hRequest, "Sec-WebSocket-Version", "13"));
		CHECK_RUN(hRequest->add_header(hRequest, "Host", "192.168.10.125"));
		CHECK_RUN(hRequest->add_header(hRequest, "Origin", "client"));
		
		while (i < 5)
		{
			if (!flag)
			{
				CHECK_RUN(hRequest->add_header(hRequest, "Sec-WebSocket-Protocol", sub_protocol[i]));
				flag = 1;
			}
			else
			{
				CHECK_RUN(hRequest->add_header(hRequest, "Sec-WebSocket-Protocol", sub_protocol[i]));
			}
			i++;
		}

		srand( (int)time(0) );
		for (i = 0; i < 4; i++)
		{
			raw_key[i] = rand();
		}
		//b64_encode(raw_key, 4*4, handshake_key, 50);
		sprintf(handshake_key, "sjdjf10030j2j002n11n");
		DBG_PRINT("handshake_key--->%s\n", handshake_key);
		CHECK_RUN(hRequest->add_header(hRequest, "Sec-WebSocket-Key", handshake_key));
		CHECK_RUN(hRequest->add_header(hRequest, "User-Agent", "FW-websocket"));
		if (1)
		{
			//CHECK_RUN(socket_send(int socketfd, char * buffer, int len))
		}
		else
		{
			//CHECK_RUN(socket_send(hClient->socketfd, hRequest->raw(), hRequest->raw_data_len));
		}	
		
			
	}while (0);

	//DBG_PRINT("%s\n", hRequest->raw_data);
	memset(raw_data, 0, 512);
	ret = hRequest->raw(hRequest, raw_data, 512);
	DBG_PRINT("\r%s\n", raw_data);

	hRequest->replace_header(hRequest, "Sec-WebSocket-Protocol", "cacacacacaca");
	hRequest->replace_header(hRequest, "Sec-WebSocket-Key", "111111111111111122222222222222222222333333");
	memset(raw_data, 0, 512);
	ret = hRequest->raw(hRequest, raw_data, 512);
	DBG_PRINT("\r%s\n", raw_data);
	return 0;
}

#endif
