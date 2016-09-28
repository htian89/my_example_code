#include "connection.h"



static RS_s connection(CONN_Handle hConn)
{
	return RS_OK;
}

static RS_s write(CONN_Handle hConn, const unsigned char *payload)
{
	return RS_OK;
}

static RS_s read(CONN_Handle hConn, const unsigned char *payload)
{
	return RS_OK;
}

RS_s new_connection(CONN_Handle *hConn, int socketfd)
{
	RETURNIF(*hConn == NULL, RS_ERROR, "");
	CONN_Handle _hConn = NULL;
	
	_hConn = (CONN_Handle)my_malloc(sizeof(CONNECTION_s));
	_hConn->connection = connection;
	_hConn->read = read;
	_hConn->write = write;
//	_hConn->socketfd = socketfd;

	*hConn = _hConn;

	return RS_OK;
}

RS_s free_connection(CONN_Handle hConn)
{
	RETURNIF(hConn == NULL, RS_ERROR, "");

	free(hConn);

	return RS_OK;
}




