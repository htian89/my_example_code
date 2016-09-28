#ifndef __LOG__
#define __LOG__

#define LOG_LEVEL_DEBUG

#ifdef _MSC_VER //use vc compile
	#ifdef LOG_LEVEL_DEBUG
        #define LOG_INFO(format, ...) trace_debug(1, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
        #define LOG_DEBUG(format, ...) trace_debug(2, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
        #define LOG_ERR(format, ...) trace_debug(3, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
        #define LOG_FATAL(format, ...) trace_debug(4, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
	#endif //LOG_LEVEL_DEBUG

	#ifdef  LOG_LEVEL_ERROR
		#define LOG_INFO(format, ...) ;
		#define LOG_DEBUG(format, ...) ;
        #define LOG_ERR(format, ...) trace_debug(3, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
        #define LOG_FATAL(format, ...) trace_debug(4, "%s\t%s(%d)\t"##format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
	#endif //LOG_LEVEL_ERROR

	#ifdef LOG_LEVEL_NOOUTPUT
		#define LOG_INFO(format, ...) ;
		#define LOG_DEBUG(format, ...) ;
		#define LOG_ERR(format, ...) ;
		#define LOG_FATAL(format, ...) ;
	#endif //!(LOG_LEVEL_DEBUG && LOG_LEVEL_ERROR)
#else
	#ifdef LOG_LEVEL_DEBUG
        #define LOG_DEBUG(format, ...) trace_debug(1, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__ )
        #define LOG_INFO(format, ...) trace_debug(2, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__ )
        #define LOG_ERR(format, ...) trace_debug(3, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__ )
        #define LOG_FATAL(format, ...) trace_debug(4, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__ )
	#endif //LOG_LEVEL_DEBUG

	#ifdef  LOG_LEVEL_ERROR
		#define LOG_INFO(format, ...) ;
		#define LOG_DEBUG(format, ...) ;
        #define LOG_ERR(format, ...) trace_debug(3, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
        #define LOG_FATAL(format, ...) trace_debug(4, "%s\t%s(%d)\t"format, __FILE__, __FUNCTION__,__LINE__, __VA_ARGS__ )
	#endif //LOG_LEVEL_ERROR

	#ifdef LOG_LEVEL_NOOUTPUT
		#define LOG_INFO(format, ...) ;
		#define LOG_DEBUG(format, ...) ;
		#define LOG_ERR(format, ...) ;
		#define LOG_FATAL(format, ...) ;
	#endif //!(LOG_LEVEL_DEBUG && LOG_LEVEL_ERROR)
#endif

//*****************************************************************
//function Name:trace_debug
//parameter:
//	format(const char*):	format string (same as printf fuction)
//	...		:				the variant need to write to log	
//return value:
//	>= 0		succeed
//	< 0			failed(-1:		format==NULL	 
//					   -10:		failed when open file
//					  -20,-21:	failed when write data to file)
//*****************************************************************
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

    int trace_debug(int level, const char* format, ...);
    int close_debug();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif 
