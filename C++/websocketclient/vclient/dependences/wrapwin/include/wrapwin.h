//// 下列 ifdef 块是创建使从 DLL 导出更简单的
//// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 WRAPWIN_EXPORTS
//// 符号编译的。在使用此 DLL 的
//// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
//// WRAPWIN_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
//// 符号视为是被导出的。
//#ifdef WRAPWIN_EXPORTS
//#define WRAPWIN_API __declspec(dllexport)
//#else
//#define WRAPWIN_API __declspec(dllimport)
//#endif
//
//// 此类是从 wrapwin.dll 导出的
//class WRAPWIN_API Cwrapwin {
//public:
//	Cwrapwin(void);
//	// TODO: 在此添加您的方法。
//};
//
//extern WRAPWIN_API int nwrapwin;
//
//WRAPWIN_API int fnwrapwin(void);
//LIBRARY

#ifndef __MY_DLL__
#define __MY_DLL__

#ifndef  _MSC_VER //use vc compile
    #include <windows.h>
	#define IOC_IN					0x80000000      /* copy in parameters */
	#define _WSAIOW(x,y)            (IOC_IN|(x)|(y))
	#define IOC_VENDOR              0x18000000
	#define SIO_KEEPALIVE_VALS		_WSAIOW(IOC_VENDOR,4)

    typedef struct _WTS_PROCESS_INFOA {
        DWORD SessionId;     // session id
        DWORD ProcessId;     // process id
        LPSTR pProcessName;  // name of process
        PSID pUserSid;       // user's SID
    } WTS_PROCESS_INFO, * PWTS_PROCESS_INFO;
#endif

extern "C" DWORD myWTSGetActiveConsoleSessionId();
extern "C" int myhello(int i, char* s);
extern "C" int myWSAIoctl(SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer,
			   LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned,
			   LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

extern "C" BOOL myWTSEnumerateProcesses(HANDLE hServer, DWORD Reserved, DWORD Version, PWTS_PROCESS_INFO * ppProcessInfo,
							OUT DWORD * pCount);

#endif //__MY_DLL__
