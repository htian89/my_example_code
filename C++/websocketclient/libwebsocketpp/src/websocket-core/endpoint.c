#include "endpoint.h"
#include "common.h"

#include <string.h>


static int s_endpoint_init_flag = 0;
static sem_t s_ep_sem;

#define EP_LOCK    {do {sem_wait(&s_ep_sem);}while (0);}
#define EP_UNLOCK    {do {sem_post(&s_ep_sem);}while (0);}

EP_Connection_Pool_s *connection_pool = NULL;
static int __find_free_location(void)
{
	int i = 0;
	EP_LOCK;
	for (i = 0; i < MAX_CONNECTION_COUNT; i ++)
	{
		if (!connection_pool[i].used)
		{
			return i;
		}
	}
	EP_UNLOCK;
	return -1;
}

static int __remove_connection(CONN_Handle hConn)
{
	int i = 0;
	EP_LOCK;
	for (i = 0; i < MAX_CONNECTION_COUNT; i++)
	{
		if (connection_pool[i].used)
		{
			if (connection_pool[i].hConn == hConn)
			{
				connection_pool[i].hConn == 0;
				connection_pool[i].used = 0;
				return 1;
			}
		}
	}
	EP_UNLOCK;
	return 0;
}

RS_s endpoint_init(ENDPOINT_Handle *hEp)
{ 
//	DBG_PRINT("start...\n");
	#if 1	
	ENDPOINT_Handle _hEp =NULL;

	RETURNIF(sem_init(&s_ep_sem, 0, 1) != 0, RS_ERROR, "");

	_hEp = (ENDPOINT_Handle)my_malloc(sizeof(ENDPOINT_s));
	RETURNIF(_hEp == NULL, RS_ERROR, "");
	#endif
//	DBG_PRINT("end...\n");
	return RS_OK;
}

RS_s endpoint_uninit(ENDPOINT_Handle hEp)
{
	RETURNIF(!s_endpoint_init_flag, RS_ERROR, "");

	sem_destroy(&s_ep_sem);

	connection_pool = NULL;

	return RS_OK;
}

RS_s create_connection(ENDPOINT_Handle hEp, CONN_Handle *hConn)
{
	#if 0
	RS_s rs = RS_FAIL;
	int pool_index = 0;
	CONN_Handle _hConn = NULL;

	pool_index = __find_free_location();
	RETURNIF(pool_index == -1, RS_ERROR, "connection maybe full");
	
	rs = new_connection(_hConn, hEp->hClient->socketfd);
	RETURNIF(rs != RS_OK, RS_FAIL, "new connection fail");

	connection_pool[pool_index].hConn= _hConn;
	connection_pool[pool_index].used = 1;
	*hConn = _hConn;
#endif
	return RS_OK;
}

RS_s remove_connection(CONN_Handle hConn)
{
	RETURNIF(hConn == NULL, RS_ERROR, "");

	RETURNIF(!__remove_connection(hConn), RS_FAIL, "hConn maybe invalid");

	RETURNIF(free_connection(hConn) != RS_OK, RS_FAIL, "");

	return RS_OK;
}



