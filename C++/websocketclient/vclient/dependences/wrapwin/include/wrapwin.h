//// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
//// ��ı�׼�������� DLL �е������ļ��������������϶���� WRAPWIN_EXPORTS
//// ���ű���ġ���ʹ�ô� DLL ��
//// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
//// WRAPWIN_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
//// ������Ϊ�Ǳ������ġ�
//#ifdef WRAPWIN_EXPORTS
//#define WRAPWIN_API __declspec(dllexport)
//#else
//#define WRAPWIN_API __declspec(dllimport)
//#endif
//
//// �����Ǵ� wrapwin.dll ������
//class WRAPWIN_API Cwrapwin {
//public:
//	Cwrapwin(void);
//	// TODO: �ڴ�������ķ�����
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
