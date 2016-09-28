#ifndef __URI_H__
#define __URI_H__

#include "common.h"

#define URI_MAX_LEN  (256)
#define HOST_LEN        (16)
#define URI_HANDLE struct _URI_s *

typedef struct _URI_s
{
	int secure;
	char *host;
	int port;
	char *resource;

	RS_s (*set_host)(URI_HANDLE, const char *);
	RS_s (*set_port)(URI_HANDLE, int );
	RS_s (*set_resource)(URI_HANDLE, const char *);
}URI_s, *URI_Handle;

typedef URI_s * URI_H ;


RS_s uri_init(URI_Handle *hUri);
RS_s uri_unit(URI_Handle hUri);

#endif