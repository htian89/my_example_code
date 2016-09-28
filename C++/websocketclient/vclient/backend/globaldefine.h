#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <stdlib.h>
#include "log.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0501 //in orde to use function GlobalMemoryStatusEx(winbase.h)
#include <windows.h>
#define VCLIENT_THREAD HANDLE
#define VCLIENT_THREAD_CREATE(thread, attr, start, arg) \
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, NULL)
#define VCLIENT_THREAD_RELEASE(m) CloseHandle(m)
#define THREAD_WAIT(m) WaitForSingleObject(m, INFINITE)
#define STOP_WEBSOCKET_EVENT "Global\\stop_WebSocket123456789"

#define PRODUCTNAME "vClient"
#define VCLIENT_EXE_NAME "vClient.exe"
#define VCLIENT_UUID_IN_SYS_ENV "vClientUUID_usedForIdentifyHost"
#define USERPATH_STORFILE "userpath"
#define CONFIG_FILE_NAME "config.xml"
const std::string userPath = std::string(getenv("APPDATA")) + std::string("\\vclient\\");
const std::string uuidFile = userPath + std::string("uuid");
const std::string networkPath = userPath + std::string("config.xml");
#else
#include <pthread.h>
#define VCLIENT_MUTEX pthread_mutex_t
#define VCLIENT_THREAD_CREATE(thread, attr, start, arg) pthread_create(&thread, attr, start, arg)
#define THREAD_DETACH(m) pthread_detach(m)
#define THREAD_WAIT(m) pthread_join(m, NULL)
#define VCLIENT_THREAD_RELEASE(m) ;
#define VCLIENT_THREAD pthread_t
#define Sleep(_minisecond)  sleep(_minisecond/1000)
#define PRODUCTNAME "Fronview3000"
const std::string userPath = std::string(getenv("HOME")) + std::string("/.vclient/");
const std::string uuidFile = userPath + std::string("uuid");
const std::string versionFile = std::string("/opt/vclient/") + std::string("vclient_version");
const std::string networkPath = userPath + std::string("config.xml");
#endif

//#define ICON_PATH ":image/resource/fronware.ico"
////#define ICON_PATH ":image/resource/vclient.ico"
////#define ICON_PATH ":image/resource/DHC-vclient.ico"

//#define ICON_PATH ":image/resource/icon_fronware.png"
//#define ICON_PATH ":image/resource/icon_oem.png"
#define ICON_PATH ":image/resource/about_vCl_logo.png"
//#define ICON_PATH ":image/resource/DHC-vclient.png"
int const MAXSIZE = 512;

struct NetWorkInfo{
    char ipAddress[MAXSIZE];
    int port;
};
typedef NetWorkInfo NETWORKINFO;


#endif // COMMON_H
