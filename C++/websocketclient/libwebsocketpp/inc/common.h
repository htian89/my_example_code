#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <string.h>
#include <semaphore.h>

#ifndef U8 
#define U8 	unsigned char
#endif

#ifndef U16	
#define U16	unsigned short
#endif

#ifndef U32
#define U32 	unsigned int
#endif

#ifndef S8 
#define S8 	char
#endif

#ifndef S16 
#define S16 	short
#endif

#ifndef S32 
#define S32 	int
#endif

#ifndef U64 
#define U64 	unsigned long long
#endif

#ifndef S64
#define S64 	long long
#endif

inline int debug_enable();
inline void set_debug_enable();

#define DBG_PRINT(...)  \
{	\
	if (debug_enable())\
	{ \
		printf("\033[33m"); \
		printf("[WS-LIB][%s][%d]:   ", __FUNCTION__, __LINE__); \
		printf(__VA_ARGS__);\
		printf("\033[m\n"); \
	}\
}

#define RETURNIF(exp, val, ext_str) \
{	\
	if (exp) \
	{ \
		DBG_PRINT("*** [return "#val" if "#exp"]:%s \n", ext_str); \
		return val; \
	} \
}

#define WARNIF(exp, ext_str) \
{	\
	if (exp) \
	{	\
		DBG_PRINT("***[warning:"#exp"]:%s\n", ext_str); \
	} \
}

#define WARNING(exp) \
{	\
	DBG_PRINT("***[warning:"#exp"]"); \
}


// just for while loop
#define CHECK_RUN(exp) \
{ \
	if (exp != RS_OK) \
	{ \
		DBG_PRINT("may be error happen\n");\
		break; \
	} \
}

#define UNSIGNED(str)  ((unsigned char *)str )
#define SIGNED(str)  ((char *)str)

typedef enum 
{
	RS_ERROR=-3,
	RS_FAIL=-2,
	RS_SOCKETCLOSE = -1,
	RS_OK=0,
}RS_s;

typedef enum
{
	MALLOC_NORMAL, 
	MALLOC_EXTEND,
	MALLOC_REMALLOC
}MALLOCK_FLAG_s;

char * my_malloc(int len);
void my_free(char *p);
RS_s __check_malloc(char **p, int len, MALLOCK_FLAG_s flag);


#endif
