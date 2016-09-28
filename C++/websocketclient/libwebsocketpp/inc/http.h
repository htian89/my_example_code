#ifndef __HTTP_H__
#define __HTTP_H__
#include "common.h"

#define HTTP_REQUEST_HANDLE struct _HTTP_Rquest_Handle * 
#define HTTP_RESPONSE_HANDLE struct _HTTP_Response_Handle * 
#define HTTP_COMMON_HANDLE  struct _HTTP_Common_s *

/**/
#define MAX_HEADER_COUNT   15 

typedef struct 
{
	char *key;
	char *value;
}HTTP_Header_s;

typedef char * CHAR_;

typedef struct _HTTP_Common_s
{
	char *version;
	HTTP_Header_s header[MAX_HEADER_COUNT];

	RS_s (*set_version)(HTTP_COMMON_HANDLE, const char *);
	
	RS_s (*add_header)(HTTP_COMMON_HANDLE, const char *, const char *);
	RS_s (*replace_header)(HTTP_COMMON_HANDLE, const char *, const char *);
	RS_s (*remove_header)(HTTP_COMMON_HANDLE, const char *);
	char * (*get_header)(HTTP_COMMON_HANDLE);
}HTTP_Common_s, *HTTP_Common_Handle;


typedef struct _HTTP_Rquest_Handle
{
	char *method;
	char *uri;
	HTTP_Common_Handle hCommon;

	RS_s (*set_method)(HTTP_REQUEST_HANDLE, const char * );
	RS_s (*set_uri)(HTTP_REQUEST_HANDLE, const  char *);
	RS_s (*raw)(HTTP_REQUEST_HANDLE, char *, int);
}HTTP_Request_s, *HTTP_Request_Handle;

typedef struct _HTTP_Response_Handle
{
	int status_code;
	char *status_msg;
	HTTP_Common_Handle hCommon;
	
	RS_s (*set_status_msg)(HTTP_RESPONSE_HANDLE, const char *);
	//RS_s (*raw)(HTTP_RESPONSE_HANDLE, char *, int);
	RS_s (*parse)(HTTP_RESPONSE_HANDLE, char *, int);
}HTTP_Response_s, *HTTP_Response_Handle; 


RS_s http_request_init(HTTP_Request_Handle *hHttpRequest);
RS_s http_request_uninit(HTTP_Request_Handle hHttpRequest);

RS_s http_response_init(HTTP_Response_Handle *hHttpResponse);
RS_s http_response_uninit(HTTP_Response_Handle hHttpResponse);

#endif
