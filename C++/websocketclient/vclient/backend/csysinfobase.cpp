#include "csysinfobase.h"
#include "globaldefine.h"
#include <string.h>
#include <log.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdio.h>
#include <QUuid>
#include <QIODevice>
#include <QTextStream>
#include <QFile>
#ifdef WIN32
#include <windows.h>
#include <winver.h>  //used to get file version
#include <winsock2.h>
#include <iphlpapi.h>
#endif

#ifdef WIN32
//get a file's version info
int getCurrentVersion(const char* fullName, char* pVerInfo)//ST_VERSION& stVer
{
    DWORD dwLen = GetFileVersionInfoSizeA(fullName, NULL);
    if(0 == dwLen)
    {
        LOG_ERR("GetFileVersionInfoSizeA failed. %s, reason:%d", fullName, GetLastError());
        return -1;
    }
    char* lpData =new char[dwLen+1];
    if(FALSE == GetFileVersionInfoA(fullName, 0, dwLen, lpData))
    {
        LOG_ERR("GetFileVersionInfoSizeA failed. reason:%d", GetLastError());
        delete []lpData;
        lpData = NULL;
        return -5;
    }

    //language and code page
    LPVOID lpBuffer = NULL;
    struct LANGANDCODEPAGE
    {
      WORD wLanguage;
      WORD wCodePage;
    };
    UINT uLen = 0;
    if(FALSE == VerQueryValueA(lpData, "\\VarFileInfo\\Translation", (LPVOID*)&lpBuffer, &uLen))
    {
        LOG_ERR("VerQueryValueA(\\VarFileInfo\\Translation) failed. reason:%d", GetLastError());
        delete []lpData;
        lpData = NULL;
        return -10;
    }
    char caTranslation[32];
    memset(caTranslation, 0, 32);
    LANGANDCODEPAGE* lpTranslate = (LANGANDCODEPAGE*)lpBuffer;
    sprintf(caTranslation, "%04x%04x", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
    LOG_INFO("caTranslation:%s", caTranslation);

    std::string strCode;
    strCode = strCode + "\\StringFileInfo\\" + caTranslation + "\\FileVersion";
    LOG_INFO("strCode:%s", strCode.c_str());
    if(FALSE == VerQueryValueA(lpData, strCode.c_str(), &lpBuffer, &uLen))
    {
        LOG_ERR("VerQueryValueA failed. reason:%d", GetLastError());
        delete []lpData;
        lpData = NULL;
        return -15;
    }
    std::string strVerInfo =(char*)lpBuffer;
    LOG_INFO("versionINFO:%s", strVerInfo.c_str());
    delete [] lpData;

    if(NULL != pVerInfo)
    {
        int iMarjor=0, iMini=0, iSmall=0, iLittle=0;
        sscanf(strVerInfo.c_str(), "%d.%d.%d,%d", &iMarjor, &iMini, &iSmall, &iLittle);
        sprintf(pVerInfo, "%d.%d.%dBuild%04d", iMarjor, iMini, iSmall, iLittle);
    }
    else
    {
        LOG_ERR("%s", "NULL == pVerInfo");
        return -20;
    }

    return 0;
}

int GetCpuInfo(char* pCpuName)
{
    if(NULL == pCpuName)
    {
        LOG_ERR("%s", "NULL == pCpuName");
        return -1;
    }
    const char* strPath="HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
    HKEY hkey_cpuInfo;
    LONG lResult=RegOpenKeyExA(HKEY_LOCAL_MACHINE,strPath,0, KEY_READ, &hkey_cpuInfo); //open reg
    if (lResult!=ERROR_SUCCESS)
    {
        LOG_ERR("RegOpenKeyA HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0 failed. reason:%d", lResult);
        return -5;
    }
    char chCPUName[128];
    memset(chCPUName, 0, 128);
    DWORD dw_dataLen = 128, dw_datatype;
    lResult = RegQueryValueExA(hkey_cpuInfo, "ProcessorNameString", 0,
                               &dw_datatype, (BYTE*)chCPUName, &dw_dataLen);
    RegCloseKey(hkey_cpuInfo);
    if(ERROR_SUCCESS == lResult)
    {
        strcpy(pCpuName, chCPUName);        
        return 0;
    }
    else
    {
        LOG_ERR("RegQueryValueExA HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0 failed. reason:%d", lResult);
        return -10;
    }
}

//to get info from system environment.
//if no specified variable name--name, then return -15
int getWindowsVariable(const char *name, char *value)
{
    if(NULL==name || NULL==value)
    {
        LOG_ERR("%s", "NULL==name || NULL==value");
        return -1;
    }
    const char* strPath="System\\CurrentControlSet\\Control\\Session Manager\\Environment";
    HKEY hkey_env;
    LONG lResult=RegOpenKeyExA(HKEY_LOCAL_MACHINE,strPath,0, KEY_QUERY_VALUE, &hkey_env); //open reg
    if (lResult!=ERROR_SUCCESS)
    {
        LOG_ERR("RegOpenKeyA System\\CurrentControlSet\\Control\\Session Manager\\Environment failed. reason:%d", lResult);
        return -5;
    }

    char chEnv[128];
    memset(chEnv, 0, 128);
    DWORD dw_dataLen = 128, dw_datatype;
    lResult = RegQueryValueExA(hkey_env, name, 0,
                               &dw_datatype, (BYTE*)chEnv, &dw_dataLen);
    RegCloseKey(hkey_env);
    if(ERROR_SUCCESS == lResult)
    {
        strcpy(value, chEnv);
        return 0;
    }
    else
    {
        LOG_ERR("RegQueryValueExA System\\CurrentControlSet\\Control\\Session Manager\\Environment failed. reason:%d", lResult);
        if(2==lResult)//no such variable
            return -15;
        return -10;
    }
}
//to set info to system environment NEEDS Administrator privilage;(query doesnot need).
//normally you have to do much more; to broadcast a msg to the system;more info please
//reference to MSDN topic:"Changing Environment Variables (Windows)"
int setWindowsVariable(const char* name, const char* value)
{
    if(NULL==name || NULL==value)
    {
        LOG_ERR("%s", "NULL==name || NULL==value");
        return -1;
    }
    const char* strPath="System\\CurrentControlSet\\Control\\Session Manager\\Environment";
    HKEY hkey_env;
    LONG lResult=RegOpenKeyExA(HKEY_LOCAL_MACHINE,strPath,0, KEY_WRITE, &hkey_env); //open reg
    if (lResult!=ERROR_SUCCESS)
    {
        LOG_ERR("RegOpenKeyA System\\CurrentControlSet\\Control\\Session Manager\\Environment failed. reason:%d", lResult);
        return -5;
    }
    lResult = RegSetValueExA(hkey_env, name, 0,
                            REG_SZ, (BYTE*)value, strlen(value));
    RegCloseKey(hkey_env);
    if(ERROR_SUCCESS == lResult)
    {
        return 0;
    }
    else
    {
        LOG_ERR("RegSetValueExA System\\CurrentControlSet\\Control\\Session Manager\\Environment failed. reason:%d", lResult);
        return -10;
    }
}

int CSysInfoWin::getNetworkinfo()
{
    WSADATA wsaData;
    int iRet = WSAStartup(1, &wsaData);
    if(iRet != 0)
        LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
    else
    {
        char hostName[512];
        memset(hostName, 0, 512);
        iRet = gethostname(hostName, 512);
        if(SOCKET_ERROR == iRet)
            LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
        else
            m_str_hostName = hostName;
        WSACleanup();
    }

    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapter = NULL;
    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
    if(pAdapterInfo == NULL)
    {
        LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
        return -1;
    }
    if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (pAdapterInfo == NULL)
        {
            LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
            return -1;
        }
    }
    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    if(dwRetVal == NO_ERROR)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            while(0==strlen(pAdapter->IpAddressList.IpAddress.String)
                || 0==strcmp(pAdapter->IpAddressList.IpAddress.String, "127.0.0.1")
                || 0==strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0"))
            {
                if(NULL == pAdapter->IpAddressList.Next)
                {
                    pAdapter = pAdapter->Next;
                    if(NULL == pAdapter)
                    {
                        free(pAdapterInfo);
                        LOG_ERR("%s","find failed.");
                        return -1;
                    }
                    continue;
                }
                pAdapter->IpAddressList = *(pAdapter->IpAddressList.Next);
            }
            m_str_ip = pAdapter->IpAddressList.IpAddress.String;
            m_str_netmask = pAdapter->IpAddressList.IpMask.String;

            char macAddr[4];
            for (unsigned int i = 0; i < pAdapter->AddressLength; i++)
            {
                macAddr[0] = '\0';
                if (i == (pAdapter->AddressLength - 1))
                    sprintf(macAddr,"%.2X", (int) pAdapter->Address[i]);
                else
                    sprintf(macAddr, "%.2X:", (int) pAdapter->Address[i]);
                m_str_macAdd = m_str_macAdd + macAddr;
            }
            break;
        }
    }
    else
    {
        LOG_ERR("GetAdaptersInfo failed with error: %d\n", dwRetVal);
        return -1;
    }

    if (pAdapterInfo)
        free(pAdapterInfo);
    return 0;
}

CSysInfoWin::CSysInfoWin()
{
    m_bHasGotNetworkInfo = false;
}

void CSysInfoWin::getProductVersion(char *value)
{
    if(value == NULL)
        return;
//get vClient.exe fullPath
    char fileName[MAX_PATH];
    memset(fileName, 0, MAX_PATH);
    if(GetModuleFileNameA(NULL, fileName, MAX_PATH)<=0)
    {
        LOG_ERR("GetModuleFileNameA failed. reason:%d", GetLastError());
        return ;
    }
    LOG_INFO("module fileName:%s", fileName);
    size_t size = strlen(fileName);
    for(int i = size; i>=0; i--)
    {
        if(fileName[i] == '/' || fileName[i] == '\\')
        {
            fileName[i+1] = '\0';
            break;
        }
    }
    strcat(fileName, VCLIENT_EXE_NAME);

//get file version
    int iRet = getCurrentVersion(fileName, value);
    if(iRet < 0)
    {//get version failed
        LOG_ERR("getCurrentVersion failed. return value:%d", iRet);
        strcpy(value, "Unknown");
    }
}

void CSysInfoWin::getProductName(char *value)
{
    if(value == NULL)
        return;
    strcpy(value, PRODUCTNAME);
}

void CSysInfoWin::getProductUuid(char *value)
{
    if(value == NULL)
        return;

    int iRet = getWindowsVariable(VCLIENT_UUID_IN_SYS_ENV, value);
    if(iRet < 0)//may be because doesnot have the variable
    {
        LOG_ERR("getWindowsVariable failed. return value:%d", iRet);
        if(-15 == iRet)
        {//variable doesnot exist, then try to make one
            QUuid uuid = QUuid::createUuid();
            QString string = uuid.toString();
            strcpy(value, string.toLocal8Bit().data());
            iRet = setWindowsVariable(VCLIENT_UUID_IN_SYS_ENV, string.toLocal8Bit().data());
            if(iRet < 0)
            {
                LOG_ERR("setWindowsVariable failed. return value:%d", iRet);
            }
            else
            {//get the variable
                int iRet = getWindowsVariable(VCLIENT_UUID_IN_SYS_ENV, value);
                if(iRet < 0)
                    LOG_ERR("getWindowsVariable failed. return value:%d", iRet);
            }
        }
    }
}

void CSysInfoWin::getCpuInfo(char *value)
{
    if(value==NULL)
        return;
    int iRet = GetCpuInfo(value);
    if(iRet < 0)
    {
        LOG_ERR("GetCpuInfo failed. return value:%d", iRet);
    }
}

void CSysInfoWin::getHostName(char *value)
{
    if(value==NULL)
        return;
    m_bHasGotNetworkInfo = false;
    getNetworkinfo();
    m_bHasGotNetworkInfo = true;
    if(m_str_hostName.empty())
        value[0] = '\0';
    else
        strcpy(value, m_str_hostName.c_str());
//    char hostName[512];
//    memset(hostName, 0, 512);
//    WSADATA wsaData;
//    int iRet = WSAStartup(1, &wsaData);
//    if(iRet != 0)
//    {
//        LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
//    }
//    else
//    {

//        iRet = gethostname(hostName, 512);
//        if(SOCKET_ERROR == iRet)
//        {
//            LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
//        }
//        else
//        {
//            strcpy(value, hostName);
//        }
//        WSACleanup();
//    }
}

void CSysInfoWin::getIpAddress(char *value)
{
    if(NULL == value)
    {
        LOG_ERR("%s", "NULL == value");
        return;
    }
    if(!m_bHasGotNetworkInfo)
        getNetworkinfo();
    if(m_str_ip.empty())
        value[0] = '\0';
    else
        strcpy(value, m_str_ip.c_str());

//    char hostName[512];
//    memset(hostName, 0, 512);
//    WSADATA wsaData;
//    int iRet = WSAStartup(1, &wsaData);
//    if(iRet != 0)
//    {
//        LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
//    }
//    else
//    {

//        iRet = gethostname(hostName, 512);
//        if(SOCKET_ERROR == iRet)
//        {
//            LOG_ERR("gethostname failed. reason:%d",WSAGetLastError());
//        }
//        else
//        {
//            PHOSTENT pHostinfo = gethostbyname(hostName);
//            if(NULL == pHostinfo)
//            {
//                LOG_ERR("gethostbyname failed. host name:%s   reason:%d", hostName, WSAGetLastError());
//            }
//            else
//            {
//                int i = 0;
//                while(pHostinfo->h_addr_list[i]!= 0)
//                {
//                    struct in_addr addr;
//                    addr.s_addr = *(u_long *)pHostinfo->h_addr_list[i++];
//                    char* ip = inet_ntoa(addr);
//                    if(0 != strcmp(ip, "127.0.0.1"))
//                    {
//                        strcpy(value, ip);
//                        break;
//                    }
//                }
//            }
//        }
//        WSACleanup();
//    }
}

void CSysInfoWin::getMacAddress(char *value)
{
    if(NULL == value)
    {
        LOG_ERR("%s", "NULL == value");
        return;
    }
    if(!m_bHasGotNetworkInfo)
        getNetworkinfo();
    if(m_str_macAdd.empty())
        value[0] = '\0';
    else
        strcpy(value, m_str_macAdd.c_str());

//    value[0]='\0';
//    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapter = NULL;
//    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
//    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
//    if(pAdapterInfo == NULL)
//    {
//        LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
//        return;
//    }
//    if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
//    {
//        free(pAdapterInfo);
//        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
//        if (pAdapterInfo == NULL)
//        {
//            LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
//            return;
//        }
//    }
//    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
//    if(dwRetVal == NO_ERROR)
//    {
//        pAdapter = pAdapterInfo;
//        char macAddr[4];
//        while (pAdapter)
//        {
//            printf("\tAdapter Addr: \t");
//            for (unsigned int i = 0; i < pAdapter->AddressLength; i++)
//            {
//                macAddr[0] = '\0';
//                if (i == (pAdapter->AddressLength - 1))
//                    sprintf(macAddr,"%.2X", (int) pAdapter->Address[i]);
//                else
//                    sprintf(macAddr, "%.2X-", (int) pAdapter->Address[i]);
//                strcat(value, macAddr);
//            }
//            break;//pAdapter = pAdapter->Next;
//        }
//    }
//    else
//    {
//        LOG_ERR("GetAdaptersInfo failed with error: %d\n", dwRetVal);
//        return;
//    }

//    if (pAdapterInfo)
//        free(pAdapterInfo);
//    return;
}

void CSysInfoWin::getNetmask(char *value)
{
    if(NULL == value)
    {
        LOG_ERR("%s", "NULL == value");
        return;
    }

    if(!m_bHasGotNetworkInfo)
        getNetworkinfo();
    if(m_str_netmask.empty())
        value[0] = '\0';
    else
        strcpy(value, m_str_netmask.c_str());

//    PIP_ADAPTER_INFO pAdapterInfo = NULL, pAdapter = NULL;
//    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
//    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
//    if(pAdapterInfo == NULL)
//    {
//        LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
//        return;
//    }
//    if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
//    {
//        free(pAdapterInfo);
//        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
//        if (pAdapterInfo == NULL)
//        {
//            LOG_ERR("%s", "Error allocating memory needed to call GetAdaptersinfo\n");
//            return;
//        }
//    }
//    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
//    if(dwRetVal == NO_ERROR)
//    {
//        pAdapter = pAdapterInfo;
//        while (pAdapter)
//        {
//            strcpy(value, pAdapter->IpAddressList.IpMask.String);
//            break;//pAdapter = pAdapter->Next;
//        }
//    }
//    else
//    {
//        LOG_ERR("GetAdaptersInfo failed with error: %d\n", dwRetVal);
//        return;
//    }

//    if (pAdapterInfo)
//        free(pAdapterInfo);
//    return;
}

void CSysInfoWin::getMemory(char *value)
{
    if(NULL == value)
        return;

    MEMORYSTATUSEX memStatus;
    memset(&memStatus, 0, sizeof(MEMORYSTATUSEX));
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    BOOL bRet = GlobalMemoryStatusEx ( &memStatus );
    if(FALSE == bRet)
    {
        LOG_ERR("GlobalMemoryStatusEx failed. reason:%d", GetLastError());
    }
    else
    {
        DWORDLONG  zt = memStatus.ullTotalPhys;
        sprintf(value, "%uMB",zt/1024/1024);
        LOG_INFO("total memory: %s", value);
    }
    return;
}

void CSysInfoWin::getGraphicsCard(char *value)
{
    //LOG_INFO("%s","getting grpahics card info");
    if(NULL == value)
        return;
    HKEY keyServ, keyGraphicCard, key;
    LONG lResult;
    lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services", 0, KEY_READ, &keyServ);
    if(ERROR_SUCCESS != lResult)
    {
        LOG_ERR("RegOpenKeyExA SYSTEM\\CurrentControlSet\\Services failed. return value:%d", lResult);
        return;
    }
    int count = 0;
    DWORD size = 0, type = 0;
    for(int i = 0;;++i)
    {
        //Sleep(5);
        size = 512;
        char name[512];
        lResult = RegEnumKeyExA(keyServ,i,name,&size,NULL,NULL,NULL,NULL);
        if(lResult == ERROR_NO_MORE_ITEMS)
        {
            LOG_ERR("enum finished, but not find graphic info. count=%d", i);
            break;
        }
        //LOG_INFO("name:%s", name);
        lResult = RegOpenKeyExA(keyServ,name,0,KEY_READ,&key);
        if(lResult != ERROR_SUCCESS)
        {
            RegCloseKey(keyServ);
            LOG_ERR("RegOpenKeyExA failed. name:%s, return value:%d", name, lResult);
            return;
        }
        size = 512;
        lResult = RegQueryValueExA(key,"Group",0,&type,(LPBYTE)name,&size);
        if(lResult!=ERROR_SUCCESS || strcmp(("Video"),name)!=0)
        {
            RegCloseKey(key);
            continue;
        };
    //has found graphic card info
        keyGraphicCard = key;
        lResult = RegOpenKeyExA(keyGraphicCard,"Enum",0,KEY_READ,&key);
        size = sizeof(count);
        lResult = RegQueryValueExA(key,"Count",0,&type,(LPBYTE)&count,&size);//get graphics card count
        RegCloseKey(key);

        char sz[512], devname[64];
        bool bHasFound = false;
        for(int j=0; j <count; ++j)
        {
            memset(sz, 0, 512);
            memset(devname, 0, 64);
            sprintf(devname,"Device%d",j);
            size = sizeof(sz);
            lResult = RegOpenKeyExA(keyGraphicCard, devname, 0, KEY_READ, &key);

            size = sizeof(sz);
            lResult = RegQueryValueExA(key,"Device Description",0,&type,(LPBYTE)sz,&size);
            if(lResult != ERROR_SUCCESS)//lResult==ERROR_FILE_NOT_FOUND ||
            {
                size = sizeof(sz);
                lResult = RegQueryValueExA(key,"DeviceDesc",0,&type,(LPBYTE)sz,&size);
                if(lResult != ERROR_SUCCESS)
                    continue;
            };
            if(strlen(sz)<=0)
                continue;
            //LOG_INFO("graphic card info:%s",sz);
            strcpy(value, sz);
            RegCloseKey(key);
            bHasFound = true;
            break;
        };
        RegCloseKey(keyGraphicCard);
        if(!bHasFound)
        {
            LOG_INFO("%s", "not found .continue");
            continue;
        }
        else
        {
            LOG_INFO("has found:%s",sz);
            break;
        }
    }
    RegCloseKey(keyServ);
}

void CSysInfoWin::getSoundCard(char *value)
{
//    MCIERROR mciErr = mciSendStringA("info 0 waveaudio product", value, 256, NULL);
//    if(0 != mciErr)
//    {
//        char errStr[256];
//        mciGetErrorStringA(mciErr, errStr, 256);
//        LOG_ERR("mciSendStringA failed. reason:%d, %s", mciErr, errStr);
//    }

    if(NULL == value)
        return;
     UINT num = waveOutGetNumDevs();
     if(num <=0)
     {
         LOG_ERR("%s", "no sound device found");
         strcpy(value, "Not Found");
         return;
     }
     LOG_INFO("wave out device num:%d", num);
     WAVEOUTCAPS audioInfo;
     MMRESULT iResult = waveOutGetDevCaps(0, &audioInfo, sizeof(WAVEOUTCAPS));
     if(MMSYSERR_NOERROR == iResult)
     {
         if(0 == ::WideCharToMultiByte(CP_UTF8, 0, audioInfo.szPname, sizeof(audioInfo.szPname), value, 256, 0, 0))
         {
             LOG_ERR("WideCharToMultiByte failed. reason:%d", GetLastError()) ;
         }
         else
         {//because it is only 32 bytes long. the name is truncated by the API
             for(int i = strlen(value); i > 0; i--)
             {
                 if(value[i] == ')')
                 {
                     value[i+1]='\0';
                     break;
                 }
             }
         }

     }
     else
     {
         LOG_ERR("auxGetDevCapsA failed. return value:%d", iResult);
     }
}

void CSysInfoWin::getOsVersion(char *value)
{
    if(NULL == value)
        return;
    OSVERSIONINFOEX os;
    SYSTEM_INFO sysInfo;
    ::ZeroMemory(&os,sizeof(os));
    GetSystemInfo(&sysInfo);
    os.dwOSVersionInfoSize=sizeof(os);
    ::GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&os));//get windows version
    if(os.dwMajorVersion == 5 && os.dwMinorVersion == 0)
        sprintf(value, "Windows 2000 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
    else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 1)
        sprintf(value, "Windows XP (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
    else if(os.dwMajorVersion == 5 && os.dwMinorVersion == 2)
    {
//        if(os.wProductType==VER_NT_WORKSTATION && sysInfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
//            sprintf(value, "Windows XP Professional x64 Edition (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        if(os.wProductType==VER_NT_WORKSTATION)
            sprintf(value, "Windows XP (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else if(GetSystemMetrics(SM_SERVERR2) == 0)
            sprintf(value, "Windows Server 2003 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else if(os.wSuiteMask&0x00008000)//VER_SUITE_WH_SERVER)
            sprintf(value, "Windows Home Server (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else if(GetSystemMetrics(SM_SERVERR2) != 0)
            sprintf(value, "Windows Server 2003 R2 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else
            sprintf(value, "Windows %lu.%lu.%lu", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
    }
    else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 0)
    {
        if(os.wProductType == VER_NT_WORKSTATION)
            sprintf(value, "Windows Vista (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else if(os.wProductType != VER_NT_WORKSTATION)
            sprintf(value, "Windows Server 2008 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else
            sprintf(value, "Windows %lu.%lu.%lu", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
    }
    else if(os.dwMajorVersion == 6 && os.dwMinorVersion == 1)
    {
        if(os.wProductType == VER_NT_WORKSTATION)
            sprintf(value, "Windows 7 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else if(os.wProductType != VER_NT_WORKSTATION)
            sprintf(value, "Windows Server 2008 R2 (%lu.%lu.%lu)", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
        else
            sprintf(value, "Windows %lu.%lu.%lu", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
    }
    else
        sprintf(value, "Windows %lu.%lu.%lu", os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);
}

#else

void CSysInfoUnix::getProductVersion(char *value)
{
    if(value == NULL)
        return;
    FILE *file = NULL;
    file = fopen(versionFile.c_str(), "r");
    if(file!=NULL)
    {
        fgets(value, 256, file);
        value[strlen(value)-1] = '\0';
        fclose(file);
    }
    else
        strcpy(value, "Unknown");
}

void CSysInfoUnix::getProductName(char *value)
{
    if(value == NULL)
        return;
    strcpy(value, PRODUCTNAME);
}

void CSysInfoUnix::getProductUuid(char *value)
{
    if(value == NULL)
        return;
    FILE *file = NULL;
    file = fopen(uuidFile.c_str(), "r");
    if(file == NULL)
    {
        QUuid uuid = QUuid::createUuid();
        QString string = uuid.toString();
        strcpy(value, string.toLocal8Bit().data());
        value = value+1;
        value[strlen(value)-1] = '\0';
        file = fopen(uuidFile.c_str(), "w");
        fprintf(file, "%s", value);
    }
    else
        fgets(value, 256, file);
    fclose(file);
}

void CSysInfoUnix::getCpuInfo(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("cat /proc/cpuinfo | grep name | cut -f2 | uniq | awk -F: '{print$2}'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("Cpu: %s", value);
    }
}

void CSysInfoUnix::getHostName(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("hostname");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("hostname: %s", value);
    }
}

void CSysInfoUnix::getIpAddress(char *value)
{
    if(value!=NULL)
    {
        //char *retValue = excuteSystemCmdInfo("ifconfig | grep -i -1 eth0 | grep -i  inet | awk -F: '{print $2}' | awk '{print $1}'");
        char *retValue = excuteSystemCmdInfo("ifconfig | grep 'inet\b' | grep -v '127.0.0.1' | awk -F' ' '{ print $2 }'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("iP address: %s", value);
    }
}

void CSysInfoUnix::getMacAddress(char *value)
{
    QFile f("/root/.vclient/mac_address");
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_ERR("Open failed.");
        return;
    }
    QTextStream txtInput(&f);
    QString lineStr;
    lineStr = txtInput.readLine();
    QByteArray ba;
    ba = lineStr.toLatin1();
    strcpy(value, ba.data());
    f.close();
}

void CSysInfoUnix::getNetmask(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("ifconfig | grep -1 -i eth0 | grep -i inet | awk -F: '{print $4}'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("netmask: %s", value);
    }
}

void CSysInfoUnix::getMemory(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("free -m | grep Mem | awk '{print $2}'");
        copyValue(value, retValue);
        free(retValue);
        strcat(value, "MB");
        LOG_INFO("total memory: %s", value);
    }
}

void CSysInfoUnix::getGraphicsCard(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("lspci |grep -i 'VGA' | awk -F: '{print $3}'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("graphics card: %s", value);
    }
}

void CSysInfoUnix::getSoundCard(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("lspci |grep -i 'Multimedia' | awk -F: '{print $3}'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("sound cards: %s", value);
    }
}

/*2014-11-5 add for VAccessGetTerminalFunc*/
void CSysInfoUnix::setTerminalFuncSwitch(char *name, int value)
{
    if (strcmp((const char *)"Browser", (const char *)name) == 0)
    {
        if (0 == value)
            excuteSystemCmdInfo("sed -i '/fw-google-chrome.desktop/d' /root/.config/menus/xfce-applications.menu");
        if (1 == value)
            excuteSystemCmdInfo("grep -q 'fw-google-chrome.desktop' /root/.config/menus/xfce-applications.menu || sed -i '/vclient.desktop/a\\t<Filename>fw-google-chrome.desktop</Filename>' /root/.config/menus/xfce-applications.menu");

        LOG_INFO("Terminal Application Function Switch %s:%d", name, value);
    }
}

void CSysInfoUnix::getOsVersion(char *value)
{
    if(value!=NULL)
    {
        char *retValue = excuteSystemCmdInfo("cat /proc/version | awk '{print $1 $3}'");
        copyValue(value, retValue);
        free(retValue);
        LOG_INFO("Os version: %s", value);
    }

}

void CSysInfoUnix::copyValue(char *dest, const char *src)
{
    if(strlen(src)>0)
        strcpy(dest, src);
    else
        strcpy(dest, "Not Found");
}

char *CSysInfoUnix::excuteSystemCmdInfo(const char *cmd)
{
    if(cmd == NULL)
        return NULL;
    char *tRet = (char *)malloc(4096);
    FILE *fpRead;
    int len;

    fpRead = popen(cmd, "r");

    tRet[0] = '\0';
    char str[4096] = {0};
    while (fgets(str, 4096 - 1, fpRead) != NULL)
    {
        strcat(tRet, str);
        memset(str, 0, sizeof(str));
    }

    len = strlen(tRet);
    if (tRet[len - 1] == '\n' || tRet[len - 1] == '\r')
        tRet[len - 1] = '\0';

    if(fpRead != NULL)
        pclose(fpRead);

    return tRet;
}

std::string CSysInfoUnix::excuteSystemCmdInfo2(const char *cmd)
{
    std::string tRet;
    if(cmd == NULL)
        return tRet;


    FILE *fpRead;
    int len;

    fpRead = popen(cmd, "r");

    char str[4096] = {0};
    while (fgets(str, 4096 - 1, fpRead) != NULL)
    {
        tRet = tRet + std::string(str);
        memset(str, 0, sizeof(str));
    }

    len = tRet.size();
    if(len) {
        if (tRet.at(len-1) == '\n' || tRet.at(len-1) == '\r')
            tRet.replace(len-1, 1, "\0");
    }

    if(fpRead != NULL)
        pclose(fpRead);

    return tRet;
}

std::string CSysInfoUnix::getFirstLine(std::string &value)
{
    int len = value.size();
    if(len == 0)
        return value;

    int pos = value.find_first_of("\n");
    if(pos != std::string::npos) {
        value.replace(pos, 1, "\0");
        value.erase(pos , len - pos);
    }
    return value;
}
#endif

