#include "uri.h"

static RS_s __common_set(char **dest, const char *src)
{
	RETURNIF(src == NULL, RS_ERROR, "");
	int len = strlen(SIGNED(src)) + 1;
	
	if (*dest)
	{
		free(*dest);
	}

	*dest = (char *)my_malloc(len);
	RETURNIF(*dest == NULL, RS_ERROR, "");

	strncpy(*dest, src, len);

	return RS_OK;
}

static RS_s __set_host(URI_Handle hUri, const char *host)
{
	RETURNIF(hUri == NULL, RS_ERROR, "");
	return __common_set(&hUri->host, host);
}

static RS_s __set_resource(URI_Handle hUri, const char *resource)
{
	RETURNIF(hUri == NULL, RS_ERROR, "");
	return __common_set(&hUri->resource, resource);
}

RS_s uri_init(URI_Handle *hUri)
{
	URI_Handle _hUri = NULL;

//	DBG_PRINT("start...\n");
	_hUri = (URI_Handle)my_malloc(sizeof(URI_s));
	RETURNIF(_hUri == NULL, RS_ERROR, "");

	_hUri->set_host = __set_host;
	_hUri->set_resource = __set_resource;
	*hUri = _hUri;
//	DBG_PRINT("end...\n");
	return RS_OK;
}

RS_s uri_uninit(URI_Handle hUri)
{
	if (hUri)
	{
		my_free((char *)hUri);
	}
	return RS_OK;
}