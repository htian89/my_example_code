#include "common.h"
#include <sys/timeb.h> 
#include <time.h> 

static int s_debug_enable = 0;

inline void set_debug_enable()
{
	s_debug_enable = 1;
}

inline int debug_enable()
{
	return s_debug_enable;
}

char * my_malloc(int len)
{
	char *p = NULL;

	p = (char *)malloc(len);
	
	if (p)
	{
		memset(p, 0, len);
	}
	
	return p;
}

void my_free(char *p)
{
	if (p)
	{
		free(p);
	}
}

RS_s __check_malloc(char **p, int len, MALLOCK_FLAG_s flag)
{
	RETURNIF(p == NULL, RS_ERROR, "");

	if (*p == NULL)
	{
		*p = my_malloc(len);
		RETURNIF(*p == NULL, RS_FAIL, "");
	}
	else if (flag == MALLOC_EXTEND)
	{
		*p = (char *)realloc(*p, len+strlen(*p)+1);
		RETURNIF(*p == NULL, RS_FAIL, "");
	}
	else if (flag == MALLOC_REMALLOC)
	{
		free(*p);
		*p = my_malloc(len);
		RETURNIF(*p == NULL, RS_FAIL, "");
	}
	
	return RS_OK;
}


static long long __getSystemTime(void) 
{ 
	struct timeb t;
	
	ftime(&t); 
	
	return 1000 * t.time + t.millitm; 
} 

int my_rand(unsigned int rand_max)
 {
       static int _srand = 0;

	if (!_srand)
	{
	     srand( (unsigned int)__getSystemTime());                                                                                                                                                                                      
	     _srand = 0;
	}
	 
	return (rand() % rand_max);
}   


