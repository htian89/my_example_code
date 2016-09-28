#include "cfrontendfap.h"
#include "../backend/csession.h"
#include "../backend/tcp.h"
#include "../common/cthread.h"
#include "../common/log.h"
#include <iostream>
#include <stdio.h>
#include "backend/vaccess.h"

#ifdef unix
#include <string.h>
#endif
using namespace std;
extern bool g_isUsbOccupy;
extern bool g_isFlashDiskOccupy ;
extern bool g_isHardDiskOccupy ;
extern bool g_isPrintOccupy ;
extern bool g_isCDRomOccupy ;
extern bool g_isOthersOccupy ;
extern bool g_autoLogin;
extern int role_dy;

CFrontEndFap::CFrontEndFap(const LAUCH_FAP &stLaunchFap, LAUNCH_CALLBACK_INFO pCallInfo):CFrontEndBase(pCallInfo)
{
    m_stLauchFap = stLaunchFap;
    m_bNeedMapUsb = m_stLauchFap.bMapUsb;
    m_bNeedMapFileSystem = m_stLauchFap.bMapFileSystem;
    m_iReloadDisk = RELOAD_VDISK_NO;

#ifdef unix
    m_bNeedMapUsb = false;
    m_bNeedMapFlashDisk = false;
    m_bNeedMapHardDisk = false;
    m_bNeedMapPrint = false;
    m_bNeedMapCDRom = false;
    m_bNeedMapOthers = false;
#endif

}

int CFrontEndFap::Launch()
{
    m_strCmdLine.clear();
    int iRet = 0;
    m_stLauchFap.stAppInfo.displayprotocol = 1;
    iRet = queryVaccess(m_stLauchFap.stAppInfo); //call  launchResource() -----> call VAccessLaunchResource() to get parameter for fap.
    if(iRet < 0)
    {
        LOG_ERR("queryVaccess failed return value:%d", iRet);
        if(NULL != m_pCallBackInfo.pFun)
        {
            m_pCallBackInfo.pFun(TYPE_QUERYVACCESS_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchFap.stAppInfo));
        }
        m_event.clear();
        return iRet;
    }
    iRet = AttachVDisk(m_stLauchFap.stAppInfo, m_stLauchFap.bAttachDisk, m_stLauchFap.launchDisk);
    if(m_bUserRequestQuit)
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, doesnot going to create process.");
        vaccessShutDownResource();
        m_event.clear();
        return 0;
    }
    if(iRet<0 || iRet==2) // attach disk failed.
    {//for desktop pool. now it has two ways. 1. to continue execute. 2. to shutdonw resource and launch again.(connect to a new desktop)
        if(DESKTOPPOOL==m_stLauchFap.stAppInfo.desktopType && 1==m_stLauchFap.stAppInfo.userAssignment)
        {
            LOG_ERR("it's destoppool(%d), attach vdisk failed. return (wait user's selection). return value:%d",(int)m_stLauchFap.stAppInfo.desktopType, iRet);
            return 0;
        }
    }
    return runDesktopInProc(); //call function formCmdParam_unix() pass parameter for fap.
}

int reLaunchApp(CFrontEndFap* pFap)
{
    if(NULL == pFap)
    {
        LOG_ERR("%s","NULL == pFap");
        return 0;
    }

    LOG_INFO("iReloadDisk:%d", pFap->m_iReloadDisk);
    if(pFap->m_iReloadDisk == RELOAD_VDISK_YES)
    {
        pFap->vaccessShutDownResource(1);
        //find the desktopool to see if it has availabe destops
        LIST_USER_RESOURCE_DATA userResListData;
        bool bHasFoundDesktop = false;
        APP_LIST appList;
        if(pFap->vaccessListUserResource(userResListData) >= 0)
        {
            for(unsigned int i = 0; i < userResListData.stAppList.size(); i++)
            {
                if(0 == strcmp(userResListData.stAppList[i].uuid, pFap->m_stLauchFap.stAppInfo.uuid))
                {
                    bHasFoundDesktop = true;
                    appList = userResListData.stAppList[i];
                    break;
                }
            }
        }
        if(!bHasFoundDesktop)
        {//the destkop doesnot exists
            if(NULL != pFap->m_pCallBackInfo.pFun)
            {// TYPE_DESKTOP_NOT_AVAILABLE 0:  thes destktop desnot belong to the user   1. no available desktop
                pFap->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 0, (void*)(&(pFap->m_pCallBackInfo)), (void*)(&(pFap->m_stLauchFap.stAppInfo)));
            }
            pFap->m_event.clear();
            return 0;
        }
        //check wether has availabe desktops
        if(appList.powerOnVmNum > 0 || appList.vmState ==1)
            return pFap->Launch();//actually, we donot need to call m_pSession->getResParam again. but
        else
        {// no availabe desktop
            if(NULL != pFap->m_pCallBackInfo.pFun)
            {
                pFap->m_stLauchFap.stAppInfo.rdpOnVmNum = appList.rdpOnVmNum;
                pFap->m_stLauchFap.stAppInfo.rdpServiceState = appList.rdpServiceState;
                pFap->m_stLauchFap.stAppInfo.vmState = appList.vmState;
                pFap->m_stLauchFap.stAppInfo.powerOnVmNum = appList.powerOnVmNum;
                pFap->m_stLauchFap.stAppInfo.displayprotocol = appList.displayprotocol;
                pFap->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 1, (void*)(&(pFap->m_pCallBackInfo)), (void*)(&(pFap->m_stLauchFap.stAppInfo)));
            }
            pFap->m_event.clear();
            return 0;
        }
        //return pFap->Launch();//actually, we donot need to call m_pSession->getResParam again. but
    }
    else
    {
        return pFap->runDesktopInProc();
    }
}

int CFrontEndFap::reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk)
{
    if(m_bUserRequestQuit || iReloadDisk== ONLY_RETURN)
    {
        LOG_ERR("doesnot going to create process.m_bUserRequestQuit:%d, iReloadDisk:%d", m_bUserRequestQuit, (int)iReloadDisk);
        m_event.clear();
        return 0;
    }
    m_iReloadDisk = iReloadDisk;
    return CThread::createThread(NULL, NULL, (FUN_THREAD)(&reLaunchApp), (void*)this);
}

int CFrontEndFap::runDesktopInProc()
{
    int iRet = 0;
#ifdef WIN32
    iRet = formCmdParam_win();
#else
    iRet = formCmdParam_unix();
#endif
    if(iRet < 0)
    {
        LOG_ERR("formCmdParam failed. return value:%d", iRet);
        if(NULL != m_pCallBackInfo.pFun)
        {
            m_pCallBackInfo.pFun(TYPE_LAUNCH_FORM_CMDPARAM_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchFap.stAppInfo));
        }
        m_event.clear();
        return iRet;
    }
    string cmd(APP_NAME_FAP);
    cmd = cmd + " " + m_strCmdLine.c_str();
    cout<<cmd<<endl;

#ifdef WIN32
    int iExitCode = 0, iExitCodeLastTime = 0, iRunTimes = 0;
    while (true)
    {//try to reconnect when app quit in an unusual way
        if(0 != iRunTimes)
            Sleep(2000);
        if(m_bUserRequestQuit)
            break;
        iExitCodeLastTime = iExitCode;
        iExitCode = runInSubProcAndWait(APP_NAME_FAP, cmd, m_stLauchFap.stAppInfo, m_stLauchFap.bAttachDisk);
        if(0!=iExitCode && 9!=iExitCode) //9 :means fap time-out-quit
        {
            if(iExitCode==iExitCodeLastTime || 0 == iRunTimes)
                iRunTimes++;
            else
                iRunTimes = 0;
        }
        else
        {
            break;
        }
        if(iRunTimes>10 || m_bUserRequestQuit)
            break;
    }
#else
//    int iExitCode = 0;
//    iExitCode = runInSubProcAndWait(APP_NAME_FAP, cmd, m_stLauchFap.stAppInfo, m_stLauchFap.bAttachDisk);
//    if(m_bNeedMapUsb)
//        g_isUsbOccupy = false;
//    if(iExitCode < 0)
//    {
//        LOG_ERR("runInSubProcAndWait failed. return value:%d", iExitCode);
//    }
    int iExitCode = 0, iExitCodeLastTime = 0, iRunTimes = 0;
    while (true)
    {//try to reconnect when app quit in an unusual way
        if(0 != iRunTimes)
            Sleep(2000);
        if(m_bUserRequestQuit)
            break;
        iExitCodeLastTime = iExitCode;
        iExitCode = runInSubProcAndWait(APP_NAME_FAP, cmd, m_stLauchFap.stAppInfo);
        if(0!=iExitCode && 9!=iExitCode) //9 :means fap time-out-quit
        {
            if(iExitCode==iExitCodeLastTime || 0 == iRunTimes)
                iRunTimes++;
            else
                iRunTimes = 0;
        }
        else
        {
            break;
        }
        if(iRunTimes > 3 || m_bUserRequestQuit)
            break;
    }
    if(m_bNeedMapUsb){
            g_isUsbOccupy = false;
    }
    if(m_bNeedMapFlashDisk){
        g_isFlashDiskOccupy = false;
    }
    if(m_bNeedMapHardDisk){
        g_isHardDiskOccupy = false;
    }
    if(m_bNeedMapPrint){
        g_isPrintOccupy = false;
    }
    if(m_bNeedMapCDRom){
        g_isCDRomOccupy = false;
    }
    if(m_bNeedMapOthers){
        g_isOthersOccupy = false;
    }
#endif

    cout << "vaccessShutdownResource" << endl;
    iRet = vaccessShutDownResource();
    if(NULL != m_pCallBackInfo.pFun)
    {
        int iValue = iRet;
        if(210 == iExitCode)
            iValue = ERROR_CODE_LAUNCHPROC_QUIT_210;
        if(9 == iExitCode)
            iValue = ERROR_CODE_LAUNCHPROC_RDP_TIME_OUT_9;
        m_pCallBackInfo.pFun(TYPE_LAUNCH_SHUTDOWN_RES_FINISHED, iValue, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchFap.stAppInfo));
    }
    m_event.clear();
    return iRet;
}

#ifdef WIN32
int CFrontEndFap::formCmdParam_win()
{
    m_strCmdLine = m_strCmdLine + "--full-screen=auto-conf";
    if(strlen(m_stLauchFap.stAppInfo.name) > 0)
        m_strCmdLine = m_strCmdLine + " -t " + m_stLauchFap.stAppInfo.name; //m_strCmdLine = m_strCmdLine + " -t \"" + m_stLauchFap.stAppInfo.name + "\"";

    char fileName[MAX_LEN];
    memset(fileName, 0, MAX_LEN);

    if (FALSE == GetModuleFileNameA(NULL, fileName, MAX_LEN))
    {
        return -1;
    }
    for(size_t i = strlen(fileName); i>0; i--)
    {
        if(fileName[i]=='/' || fileName[i]=='\\')
        {
            fileName[i+1] = '\0';
            break;
        }
    }
    LOG_INFO("GetModuleFileNameA %s", fileName);
    if(strlen(m_resData.stResInfo.SecurtityPort) >0 && strlen(m_resData.stResInfo.SecurtityToken) >0) {
        m_strCmdLine = m_strCmdLine + " --fap-host-subject=\"C=CN, L=Beijing, O=Fronware, CN=my server\" ";
        m_strCmdLine = m_strCmdLine + " --fap-ca-file=\"" + fileName + "ca-cert.pem\"";
        //m_strCmdLine = m_strCmdLine + " --fap-ca-file=/root/.config/spicy/ca-cert.pem";
        m_strCmdLine = m_strCmdLine + " -s " + m_resData.stResInfo.SecurtityPort;
        m_strCmdLine = m_strCmdLine + " -w " + m_resData.stResInfo.SecurtityToken;

    }
    if(HASBAR_STATE == m_stLauchFap.barStatus)
        m_strCmdLine = m_strCmdLine + " --on-float";

    if(strlen(m_resData.stResInfo.port) > 0)
        m_strCmdLine = m_strCmdLine + " -p " + m_resData.stResInfo.port;
    else
        m_strCmdLine = m_strCmdLine + " -p 3389";

    if(strlen(m_resData.stResInfo.ipAddr) > 0)
        m_strCmdLine = m_strCmdLine + " -h " + m_resData.stResInfo.ipAddr;
    else
    {
        CSession* pSession = CSession::GetInstance();
        if(NULL == pSession)
        {
            LOG_ERR("%s","NULL == pSession");
            return -5;
        }
        const NETWORK& stNetwork = pSession->getNetwork();
        m_strCmdLine = m_strCmdLine + " -h " + stNetwork.stPresentServer.serverAddress;//m_strCmdLine = m_strCmdLine + " -h " + stNetwork.presentServer;
    }

    if(m_bNeedMapFileSystem){
        m_strCmdLine = m_strCmdLine + " --spice-shared-dir=";
        m_strCmdLine = m_strCmdLine + m_stLauchFap.stFileInfo.stFirstPath.filePath;
    }
    if(m_resData.stResInfo.iIdleTime>=0)
    {//time out
        char caTime[32];
        memset(caTime, 0, 32);
        sprintf(caTime, "%d", m_resData.stResInfo.iIdleTime);
        m_strCmdLine = m_strCmdLine + " -m " + caTime;
    }

    LOG_INFO("cmd:\t%s", m_strCmdLine.c_str());
    return 0;
}
#endif

extern bool vclass_flag;
int CFrontEndFap::formCmdParam_unix()
{
    //m_strCmdLine = m_strCmdLine + "-f";
            if(strlen(m_stLauchFap.stAppInfo.name) > 0)
            {
        //        m_strCmdLine = m_strCmdLine + " --title=" + m_stLauchFap.stAppInfo.name;
                m_strCmdLine = m_strCmdLine + " --title=\"" + m_stLauchFap.stAppInfo.name + "\"";
            }

            // In Linux, we always has float bar.
//            m_strCmdLine = m_strCmdLine + " --on-float";
            //auto login need floatbar;
            if(vclass_flag)
            {
                if(2 == role_dy)
                {
                    m_strCmdLine = m_strCmdLine + " --student-floatbar ";
                }
                else
                {
                    m_strCmdLine = m_strCmdLine + " --teacher-floatbar ";
                }
            }

            char ip[MAX_LEN] = {0};
            char port[MAX_LEN] = {0};
            if(strlen(m_resData.stResInfo.ipAddr) > 0)
            {
                strcpy(ip, m_resData.stResInfo.ipAddr);
            }
            else
            {
                CSession* pSession = CSession::GetInstance();
                if(NULL == pSession)
                {
                    LOG_ERR("%s","NULL == pSession");
                    return -5;
                }
                const NETWORK& stNetwork = pSession->getNetwork();
                strcpy(ip, stNetwork.stPresentServer.serverAddress);
            }
            if(strlen(m_resData.stResInfo.port) > 0)
            {
                strcpy(port, m_resData.stResInfo.port);
            }
            else
            {
                strcpy(port, "3389");
            }

          // åŠ å¯†
            if(strlen(m_resData.stResInfo.SecurtityPort) >0 && strlen(m_resData.stResInfo.SecurtityToken) >0)
            {
                m_strCmdLine = m_strCmdLine + " fap://" + ip + "?port=" + port +"\\&tls-port=" + m_resData.stResInfo.SecurtityPort +"\\&password=" + m_resData.stResInfo.SecurtityToken;
                //m_strCmdLine = m_strCmdLine + " fap://" + ip + "?port=" + port +"\\&tls-port=" + port +"\\&password=" + m_resData.stResInfo.SecurtityToken;
                m_strCmdLine = m_strCmdLine + " --fap-host-subject=\"C=CN, L=Beijing, O=Fronware, CN=my server\" ";
                m_strCmdLine = m_strCmdLine + " --fap-ca-file=/opt/fap/ca-cert.pem ";

                if (strlen(m_resData.stResInfo.SecurityUuid) > 0 && strlen(m_resData.stResInfo.Uuid) > 0)
                {
                    m_strCmdLine = m_strCmdLine + " --fap-fwproxy-suuid=\"" + m_resData.stResInfo.SecurityUuid + "\"";
                    m_strCmdLine = m_strCmdLine + " --fap-fwproxy-uuid=\"" + m_resData.stResInfo.Uuid + "\"";
                }

            }
            else
            {
                m_strCmdLine = m_strCmdLine + " fap://" + ip + "?port=" + port;

                if (strlen(m_resData.stResInfo.Uuid) > 0)
                {
                    m_strCmdLine = m_strCmdLine + " --fap-fwproxy-uuid=\"" + m_resData.stResInfo.Uuid + "\"";
                }
            }

            if(1 != m_resParam.audio)
            {
                m_strCmdLine = m_strCmdLine + " --fap-disable-audio";
            }
            if(strlen(m_resParam.usb) > 0 && !g_isUsbOccupy)
            {
                g_isUsbOccupy = true;
                m_bNeedMapUsb = true;

                if( strstr(m_resParam.usb, "####:####") != NULL )
                {
                    m_strCmdLine = m_strCmdLine + " --fap-usbredir-redirect-on-connect=\"0x03,-1,-1,-1,0|0x0e,-1,-1,-1,0|-1,-1,-1,-1,1\"";
                }
                else
                {
                    m_strCmdLine = m_strCmdLine + " --fap-usbredir-redirect-on-connect=\"0x03,-1,-1,-1,0|0x0e,-1,-1,-1,0|-1,-1,-1,-1,1\"";
                    string str = m_resParam.usb;
                    string usb_param;
                    int loop = 0;
                    //note the vid:pid styl  "1111:2222 3333:4444";
                    cout << "usb::::" << m_resParam.usb << endl;
                    int len = (str.length()+1)/10 ;
                    while(  len-- )
                    {
                        string node = str.substr(loop, 10);
                        string vid = node.substr(0, 4);
                        string pid = node.substr(5, 4);
                        char value[30];
                        memset(value, 0, sizeof(value));
                        if(len == 0){
                            sprintf(value , "%s:%s",vid.c_str(), pid.c_str() );
                        }else{
                            sprintf(value , "%s:%s|",vid.c_str(), pid.c_str() );
                        }
                        usb_param = usb_param + value;
                        loop = loop + 10;
                    }
                    m_strCmdLine = m_strCmdLine + " --fap-usbredir-white-list=\"" + usb_param +"\"";
                }
            }
            else
                m_strCmdLine = m_strCmdLine + " --fap-disable-usbredir";
// the control to usb type is conflict to usb map
            int count = 0;
            while(strlen(m_resParam.usbType[count]) > 0){
                cout << "control the usb type" << endl;
                if(strcmp(m_resParam.usbType[count], "FlashDisk") == 0 && !g_isFlashDiskOccupy){
                    cout << "FlashDisk" << endl;
                    m_bNeedMapFlashDisk = true;
                    g_isFlashDiskOccupy = true;
                }else if(strcmp(m_resParam.usbType[count], "HardDisk") == 0 && !g_isHardDiskOccupy){
                    cout << "HardDisk" << endl;
                    m_bNeedMapHardDisk = true;
                    g_isHardDiskOccupy = true;
                }else if(strcmp(m_resParam.usbType[count], "Print") == 0 && !g_isPrintOccupy){
                    cout << "Print" << endl;
                    m_bNeedMapPrint = true;
                    g_isPrintOccupy = true;
                }else if(strcmp(m_resParam.usbType[count], "CD-ROM") == 0 && !g_isCDRomOccupy){
                    cout << "CD-ROM" << endl;
                    m_bNeedMapCDRom = true;
                    g_isCDRomOccupy = true;
                }else if(strcmp(m_resParam.usbType[count], "Others") == 0 && !g_isOthersOccupy){
                    cout << "Others" << endl;
                    m_bNeedMapOthers = true;
                    g_isOthersOccupy = true;
                }
                count++;
            }
// the end of control usb type;
            if(m_bNeedMapFileSystem){
                m_strCmdLine = m_strCmdLine + " --spice-shared-dir=\"";
                m_strCmdLine = m_strCmdLine + m_stLauchFap.stFileInfo.stFirstPath.filePath + "\"";
            }
            LOG_INFO("cmd:\t%s", m_strCmdLine.c_str());
            return 0;
}

