#ifndef _WIN32
#include "cfrontendterminal.h"
#include "../backend/csession.h"
#include "../common/cthread.h"
#include "../common/log.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ui/cmessagebox.h"
#include "ipc/ipcitalc.h"

using namespace std;

int CFrontEndTerminal::runningCount = 0;
extern IpcItalc *g_ipcItalc;

CFrontEndTerminal::CFrontEndTerminal(const LAUNCH_TERMINAL& stLaunchTerminal, LAUNCH_CALLBACK_INFO pCallInfo):CFrontEndBase(pCallInfo)
{
    m_stLaunchTerminal = stLaunchTerminal;
    cout << "new g_ipcitalc " << endl;
    g_ipcItalc = new IpcItalc;
    ++runningCount;

}

CFrontEndTerminal::~CFrontEndTerminal() {
    --runningCount;
    if(g_ipcItalc != NULL)
    {
        cout << " delete ipcItalc" << "##################" << endl;
        delete g_ipcItalc;
        g_ipcItalc = NULL;
    }
}

int CFrontEndTerminal::Launch()
{
    int iRet = 0;
    m_stLaunchTerminal.stAppInfo.displayprotocol = 0;

//    iRet = queryVaccess(m_stLaunchTerminal.stAppInfo);
//    if(iRet < 0)
//    {
//        LOG_ERR("queryVaccess failed return value:%d", iRet);
//        if(NULL != m_pCallBackInfo.pFun)
//        {
//            m_pCallBackInfo.pFun(TYPE_QUERYVACCESS_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLaunchTerminal.stAppInfo));
//        }
//        m_event.clear();
//        return iRet;
//    }

    if(m_bUserRequestQuit )
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, doesnot going to create process.");
        m_event.clear();
        return 0;
    }

   iRet = runDesktopInProc();
    return iRet;
}

int reLaunchApp(CFrontEndTerminal* pTerminal)
{
    if(NULL == pTerminal)
    {
        LOG_ERR("%s","NULL == pTerminal");
        return 0;
    }
    LOG_INFO("iReloadDisk:%d", pTerminal->m_iReloadDisk);
    if(pTerminal->m_iReloadDisk == RELOAD_VDISK_YES)
    {
        pTerminal->vaccessShutDownResource(1);
        //find the desktopool to see if it has availabe destops
        LIST_USER_RESOURCE_DATA userResListData;
        bool bHasFoundDesktop = false;
        APP_LIST appList;
        if(pTerminal->vaccessListUserResource(userResListData) >= 0)
        {
            for(unsigned int i = 0; i < userResListData.stAppList.size(); i++)
            {
                if(0 == strcmp(userResListData.stAppList[i].uuid, pTerminal->m_stLaunchTerminal.stAppInfo.uuid))
                {
                    bHasFoundDesktop = true;
                    appList = userResListData.stAppList[i];
                    break;
                }
            }
        }
        if(!bHasFoundDesktop)
        {//the destkop doesnot exists
            if(NULL != pTerminal->m_pCallBackInfo.pFun)
            {// TYPE_DESKTOP_NOT_AVAILABLE 0:  thes destktop desnot belong to the user   1. no available desktop
                pTerminal->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 0, (void*)(&(pTerminal->m_pCallBackInfo)), (void*)(&(pTerminal->m_stLaunchTerminal.stAppInfo)));
            }
            pTerminal->m_event.clear();
            return 0;
        }
        //check wether has availabe desktops
        if(appList.state > 0)
            return pTerminal->Launch();//actually, we donot need to call m_pSession->getResParam again. but
        else
        {// no availabe desktop
            if(NULL != pTerminal->m_pCallBackInfo.pFun)
            {
                pTerminal->m_stLaunchTerminal.stAppInfo.state = appList.state;
                pTerminal->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 1, (void*)(&(pTerminal->m_pCallBackInfo)), (void*)(&(pTerminal->m_stLaunchTerminal.stAppInfo)));
            }
            pTerminal->m_event.clear();
            return 0;
        }
    }
    else
    {
        return pTerminal->runDesktopInProc();
    }
}

int CFrontEndTerminal::reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk)
{
    if(m_bUserRequestQuit || iReloadDisk==ONLY_RETURN)
    {
        LOG_ERR("does not going to create process.m_bUserRequestQuit:%d, iReloadDisk:%d", m_bUserRequestQuit, (int)iReloadDisk);
        m_event.clear();
        return 0;
    }
    m_iReloadDisk = iReloadDisk;
    return CThread::createThread(NULL, NULL, (FUN_THREAD)(&reLaunchApp), (void*)this);
}

int CFrontEndTerminal::runDesktopInProc()
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
            m_pCallBackInfo.pFun(TYPE_LAUNCH_FORM_CMDPARAM_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLaunchTerminal.stAppInfo));
        }
        m_event.clear();
        return iRet;
    }


    string cmd(APP_NAME_TERMINAL);
    cmd = cmd + " " + m_strCmdLine.c_str();

//    LIST_USER_RESOURCE_DATA userResListData;
//    vaccessListUserResource(userResListData);

    int iExitCode = 0;
    m_stLaunchTerminal.bAttachDisk = false;

    iExitCode = runInSubProcAndWait(APP_NAME_TERMINAL, cmd, m_stLaunchTerminal.stAppInfo);

    if(iExitCode < 0)
    {
        LOG_ERR("runInSubProcAndWait failed. return value:%d", iExitCode);
    }

    iRet = vaccessShutDownResource();

//    if(g_ipcItalc != NULL)
//    {
//        cout << "delete g_ipcitalc" << endl;
//        delete g_ipcItalc;
//        g_ipcItalc = NULL;
//    }
    if(NULL != m_pCallBackInfo.pFun)
    {
        int iValue = iRet;
        if(210 == iExitCode)
            iValue = ERROR_CODE_LAUNCHPROC_QUIT_210;
        if(9 == iExitCode)
            iValue = ERROR_CODE_LAUNCHPROC_RDP_TIME_OUT_9;
        m_pCallBackInfo.pFun(TYPE_LAUNCH_SHUTDOWN_RES_FINISHED, iValue, (void*)(&m_pCallBackInfo), (void*)(&m_stLaunchTerminal.stAppInfo));
    }

    m_event.clear();

    return iRet;
}

#ifdef _WIN32
int CFrontEndTerminal::formCmdParam_win()
{
    m_strCmdLine.clear();
    CSession* pSession = CSession::GetInstance();
    if(NULL == pSession)
    {
        LOG_ERR("%s","NULL == pSession");
        return -5;
    }
    const NT_ACCOUNT_INFO& stAccountInfo = pSession->getNT_ACCOUNT_INFO();
    const NETWORK& stNetwork = pSession->getNetwork();

    m_strCmdLine = m_strCmdLine + " --debug-log "; //set log level
    m_strCmdLine = m_strCmdLine + "-a 32"; //color deeps
    if(m_resData.stResInfo.iIdleTime>=0)
    {//time out
        char caTime[32];
        memset(caTime, 0, 32);
        sprintf(caTime, "%d", m_resData.stResInfo.iIdleTime);
        m_strCmdLine = m_strCmdLine + " --set-timer " + caTime;
    }
//add ntAccountInfo to Param
    if(strlen(stAccountInfo.ntUsername)>0)
        m_strCmdLine = m_strCmdLine + " -u \"" + stAccountInfo.ntUsername + "\"";
    if(strlen(stAccountInfo.ntPassword)>0)
        m_strCmdLine = m_strCmdLine + " -p \"" + stAccountInfo.ntPassword + "\"";
    if(strlen(stAccountInfo.ntDomain)>0)
    {
        char ntDomain[MAX_LEN];
        strcpy(ntDomain, stAccountInfo.ntDomain);
        char* pIndex = strchr(ntDomain, '.');
        if(NULL != pIndex)
        {
            ntDomain[strlen(ntDomain) - strlen(pIndex)] = '\0';
        }
        m_strCmdLine = m_strCmdLine + " -d \"" + ntDomain + "\"";
    }

    if(strlen(m_resParam.alternateShell) > 0)
    {
        m_strCmdLine = m_strCmdLine + " -s \"" + m_resParam.alternateShell + "\"";
    }
    if(strlen(m_resParam.workingDir) > 0)
    {//work dir
        m_strCmdLine = m_strCmdLine + " -c \"" + m_resParam.workingDir + "\"";
    }
    m_strCmdLine = m_strCmdLine + " -f";//fullscreen
    if(strlen(m_stLaunchTerminal.stAppInfo.name) > 0)
    {
        m_strCmdLine = m_strCmdLine + " -T \"" + m_stLaunchTerminal.stAppInfo.name + "\"";
    }
    if(1 == m_resParam.protocol)
    {
        if(stNetwork.stPresentServer.isHttps)//if(stNetwork.isHttps)
            m_strCmdLine = m_strCmdLine + " -P \"https://" + stNetwork.stPresentServer.serverAddress +":"+stNetwork.stPresentServer.port + "\"";//m_strCmdLine = m_strCmdLine + " -P \"https://" + stNetwork.presentServer +":"+stNetwork.port + "\"";
        else
            m_strCmdLine = m_strCmdLine + " -P \"http://" + stNetwork.stPresentServer.serverAddress +":"+stNetwork.stPresentServer.port + "\"";//m_strCmdLine = m_strCmdLine + " -P \"http://" + stNetwork.presentServer +":"+stNetwork.port + "\"";
    }
    m_strCmdLine = m_strCmdLine + " -z -x l --no-tls --no-nla";
    if(m_resParam.disk>0 || 1==m_resParam.smartcard || 1==m_resParam.parallelPort || 1==m_resParam.serialPort)
    {
        m_strCmdLine = m_strCmdLine + " --plugin rdpdr --data";
        if(1 == m_resParam.smartcard)
        {
#ifdef _WIN32
            m_strCmdLine = m_strCmdLine + " scard";
#endif
        }
        if(m_resParam.disk > 0)
        {
            if(1 == m_resParam.disk)
            {
                m_strCmdLine = m_strCmdLine + " disk:*:hotplug:ro";
            }
            else if(2 == m_resParam.disk)
            {
                m_strCmdLine = m_strCmdLine + " disk:*:hotplug:rw";
            }
        }
        if(1 == m_resParam.parallelPort)
        {
            //if(0 == m_stLaunchTerminal.stPort.parallelLocal)
            //{
            //    m_strCmdLine = m_strCmdLine + " parallel:LPT2:LPT1";
            //}
            //else
            //{
                char remote[MIN_LEN], local[MIN_LEN];
                sprintf(remote, "%d", m_stLaunchTerminal.stPort.parallelRemote+1);
                sprintf(local, "%d", m_stLaunchTerminal.stPort.parallelLocal+1);
                m_strCmdLine = m_strCmdLine + " parallel:LPT" + remote + ":LPT" + local;
            //}
        }
        if(1 == m_resParam.serialPort)
        {
            //if( 0 == m_stLaunchTerminal.stPort.serialLocal)
            //{
            //    m_strCmdLine = m_strCmdLine + " serial:COM1:COM1";//??? is all COM1???
            //}
            //else
            //{
                char remote[MIN_LEN], local[MIN_LEN];
                sprintf(remote, "%d", m_stLaunchTerminal.stPort.serialRemote+1);
                sprintf(local, "%d", m_stLaunchTerminal.stPort.serialLocal+1);
//                itoa(m_stLaunchTerminal.stPort.serialRemote, remote, 10);
//                itoa(m_stLaunchTerminal.stPort.serialLocal, local, 10);
                m_strCmdLine = m_strCmdLine + " serial:COM" + remote + ":COM" + local;
            //}
        }
        m_strCmdLine = m_strCmdLine + " --";
    }

    if(1 == m_resParam.audio)
    {
        m_strCmdLine = m_strCmdLine + " --plugin frdpsnd";
    }
    if(1 == m_resParam.audioIn || 1 == m_resParam.audio)
    {
        m_strCmdLine = m_strCmdLine + " --plugin drdynvc --data";
        if (m_resParam.audioIn == 1)
        {
            m_strCmdLine = m_strCmdLine + " audin";
        }
        if (m_resParam.audio == 1)
        {
            m_strCmdLine = m_strCmdLine + " tsmf";
        }
        m_strCmdLine = m_strCmdLine + " --";
    }

    if(1 == m_resParam.audio)
    {
        m_strCmdLine = m_strCmdLine + " --plugin rdpsnd";
    }
    if(1 == m_resParam.audioIn || 1 == m_resParam.audio)
    {
        m_strCmdLine = m_strCmdLine + " --plugin drdynvc --data";
        if (m_resParam.audioIn == 1)
        {
            m_strCmdLine = m_strCmdLine + " audin";
        }
        if (m_resParam.audio == 1)
        {
            m_strCmdLine = m_strCmdLine + " tsmf:audio:alsa";
        }
        m_strCmdLine = m_strCmdLine + " --";
    }
    if( m_resParam.clipboard > 0)
    {
        m_strCmdLine = m_strCmdLine + " --plugin cliprdr --data";
        if(1 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " u";
        else if(2 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " d";
        else if(3 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " b";
        m_strCmdLine = m_strCmdLine + " --";
    }
    if(1 == m_resParam.printer)
    {
        m_strCmdLine = m_strCmdLine + " --plugin fprinter";
    }

    if(strlen(m_resParam.rail.applicationName)>0)
    {
        char temp[MAX_LEN];
        strcpy(temp, m_resParam.rail.applicationName);
        for (size_t j = 0; j < strlen(temp); j++)
            if (temp[j] == ':')
                temp[j] = '|';
        m_strCmdLine = m_strCmdLine + " --plugin rail --data \"" + temp + "\":";
        if (strlen(m_resParam.rail.workingDirectory) > 0)
        {
            strcpy(temp, m_resParam.rail.workingDirectory);
            for (size_t j = 0; j < strlen(temp); j++)
                if (temp[j] == ':')
                    temp[j] = '|';
            m_strCmdLine = m_strCmdLine + "\"" + temp + "\"";
        }
        m_strCmdLine = m_strCmdLine + ":";
        if (strlen(m_resParam.rail.arguments) > 0)
        {
            strcpy(temp, m_resParam.rail.arguments);
            for (size_t j = 0; j < strlen(temp); j++)
                if (temp[j] == ':')
                    temp[j] = '|';
            m_strCmdLine = m_strCmdLine + temp;
        }
        m_strCmdLine = m_strCmdLine + " --";
    }
    if(HASBAR_STATE == m_stLaunchTerminal.barStatus)
    {
        m_strCmdLine = m_strCmdLine +  " --on-float";
    }
    m_strCmdLine = m_strCmdLine + " -t";
    if (strlen(m_resData.stResInfo.port) > 0)
        m_strCmdLine = m_strCmdLine + " " +  m_resData.stResInfo.port;
    else
        m_strCmdLine = m_strCmdLine + " 3389";
    if(0 == strlen(m_resData.stResInfo.ipAddr))
    {
        m_strCmdLine = m_strCmdLine + " " + stNetwork.stPresentServer.serverAddress;//m_strCmdLine = m_strCmdLine + " " + stNetwork.presentServer;
    }
    else
    {
        m_strCmdLine = m_strCmdLine + " " + m_resData.stResInfo.ipAddr;
    }
    LOG_INFO("cmd:\t%s", m_strCmdLine.c_str());
    return 0;
}
#endif

int CFrontEndTerminal::formCmdParam_unix()
{
//    m_strCmdLine = m_strCmdLine + " -rctrl";

//    if(strlen(m_stLaunchTerminal.stAppInfo.TerminalIp) > 0) {
//         m_strCmdLine = m_strCmdLine + " " + m_stLaunchTerminal.stAppInfo.TerminalIp;

//    }
//    else {
//        LOG_ERR("m_stLaunchTerminal.stAppInfo.TerminalIp is NULL");
//    }
    LOG_INFO("cmd:\t%s", m_strCmdLine.c_str());
    return 0;
}


int CFrontEndTerminal::setForeground(LAUNCH_TYPE type, const char *titleName )
{
    if( titleName != NULL){
        LOG_INFO("setforeground  launchtype: %d, titlename: %s", type, titleName);
    }
    if(procStatus()<0)
        return -1;
#ifdef _WIN32
    if(VIRTUALAPP != m_stLaunchTerminal.stAppInfo.desktopType)
    {
        WCHAR wtitle[32] ;
        memset(wtitle, 0, sizeof(wtitle));
        int coverLen = MultiByteToWideChar(CP_UTF8, 0, titleName, -1, wtitle, 32);
        std::cout << "cover_len: " << coverLen << std::endl;
        WCHAR wclass[32] = TEXT("wfreerdp");
        HWND hh= FindWindow(wclass,  wtitle);
        SetForegroundWindow(hh);
        ShowWindow(hh, SW_NORMAL);
        return 0;
    }
    else
    {
        return CFrontEndBase::setForeground(LAUNCH_TYPE_FAP, "");
    }
#else
    return 0;
#endif
}

#endif
