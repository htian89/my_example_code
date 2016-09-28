#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "log.h"

#ifndef FORMAT_PRE
#define FORMAT_PRE "\0"
#endif

extern char g_log_file_path[512];		//you'd better define it in the main.c or main.cpp
extern int g_log_writeFileLevel;
static FILE* fp = NULL;

int trace_debug(int level, const char* format, ...)
{
    if(level < g_log_writeFileLevel)
    {//do not write to log
        return 0;
    }
	int numWritten = 0;
    FILE* fp = NULL;
	va_list arg_ptr; 
	time_t now; // 
	if(NULL == format)
	{
		printf("write log to file failed(NULL==format file:log.c int trace_debug(const char* format, ...))!\n");
		return -1;
	} 

    va_start(arg_ptr, format);
    if(NULL == fp)
        fp = fopen(g_log_file_path,"a+");
    if(NULL == fp)
    {
        printf("write log to file failed(NULL==fp  file:log.c int trace_debug(const char* format, ...))!\n");
        return -10;
    }    
 //write log info to file   
    numWritten = vfprintf(fp, format, arg_ptr);
	//int  vsprintf(char *buffer,const char *format,va_list argptr) 
	va_end(arg_ptr);
	if (numWritten < 0)
    {
        printf("write log info to file failed(errorNum:%d  file:log.c int trace_debug(const char* format, ...))\n", numWritten);
		fclose(fp);
        return -20;
    }
//write current time to file
	time(&now); //get current time
	numWritten = fprintf(fp, "\t time:%s\n",ctime(&now));
    fflush(fp);

    fclose(fp);
    if (numWritten < 0)
    {
        printf("write time to file failed(errorNum:%d  file:log.c int trace_debug(const char* format, ...))\n", numWritten);
        return -21;
    }
    return 0;
}

int close_debug()
{
    if(NULL != fp)
        fclose(fp);
    return 0;
}
