#include "cfrontendrdp.h"
#include "../backend/csession.h"
#include "../common/cthread.h"
#include "../common/log.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <QDebug>

extern char str_uuid_haproxy[128];

using namespace std;

CFrontEndRdp::CFrontEndRdp(const LAUCH_RDP& stLaunchRdp, LAUNCH_CALLBACK_INFO pCallInfo):CFrontEndBase(pCallInfo)
{
    m_stLauchRdp = stLaunchRdp;
    m_bNeedMapUsb = m_stLauchRdp.bMapUsb;
    m_bNeedMapFileSystem = m_stLauchRdp.bMapFileSystem;
    m_iReloadDisk = RELOAD_VDISK_NO;
}

int CFrontEndRdp::Launch()
{
    int iRet = 0;
    m_stLauchRdp.stAppInfo.displayprotocol = 0;
    iRet = queryVaccess(m_stLauchRdp.stAppInfo);
    if(iRet < 0)
    {
        LOG_ERR("queryVaccess failed return value:%d", iRet);
        if(NULL != m_pCallBackInfo.pFun)
        {
            m_pCallBackInfo.pFun(TYPE_QUERYVACCESS_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchRdp.stAppInfo));
        }
        m_event.clear();
        return iRet;
    }
    iRet = AttachVDisk(m_stLauchRdp.stAppInfo, m_stLauchRdp.bAttachDisk, m_stLauchRdp.launchDisk);   // launchDisk is non-required;  ==NULL
    if(m_bUserRequestQuit )
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, doesnot going to create process.");
        vaccessShutDownResource();
        m_event.clear();
        return 0;
    }
    if(iRet<0 || iRet==2) // attach disk failed.
    {//for desktop pool. now it has two ways. 1. to continue execute. 2. to shutdonw resource and launch again.(connect to a new desktop)
        if(DESKTOPPOOL==m_stLauchRdp.stAppInfo.desktopType && 1==m_stLauchRdp.stAppInfo.userAssignment)
        {
            LOG_ERR("it's destoppool(%d), attach vdisk failed. return (wait user's selection). return value:%d",(int)m_stLauchRdp.stAppInfo.desktopType, iRet);
            return 0;
        }
    }
    return runDesktopInProc();
}

int reLaunchApp(CFrontEndRdp* pRdp)
{
    if(NULL == pRdp)
    {
        LOG_ERR("%s","NULL == pRdp");
        return 0;
    }
    LOG_INFO("iReloadDisk:%d", pRdp->m_iReloadDisk);
    if(pRdp->m_iReloadDisk == RELOAD_VDISK_YES)
    {
        pRdp->vaccessShutDownResource(1);
        //find the desktopool to see if it has availabe destops
        LIST_USER_RESOURCE_DATA userResListData;
        bool bHasFoundDesktop = false;
        APP_LIST appList;
        if(pRdp->vaccessListUserResource(userResListData) >= 0)
        {
            for(unsigned int i = 0; i < userResListData.stAppList.size(); i++)
            {
                if(0 == strcmp(userResListData.stAppList[i].uuid, pRdp->m_stLauchRdp.stAppInfo.uuid))
                {
                    bHasFoundDesktop = true;
                    appList = userResListData.stAppList[i];
                    break;
                }
            }
        }
        if(!bHasFoundDesktop)
        {//the destkop doesnot exists
            if(NULL != pRdp->m_pCallBackInfo.pFun)
            {// TYPE_DESKTOP_NOT_AVAILABLE 0:  thes destktop desnot belong to the user   1. no available desktop
                pRdp->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 0, (void*)(&(pRdp->m_pCallBackInfo)), (void*)(&(pRdp->m_stLauchRdp.stAppInfo)));
            }
            pRdp->m_event.clear();
            return 0;
        }
        //check wether has availabe desktops
        if(appList.rdpOnVmNum > 0 || appList.rdpServiceState ==1)
            return pRdp->Launch();//actually, we donot need to call m_pSession->getResParam again. but
        else
        {// no availabe desktop
            if(NULL != pRdp->m_pCallBackInfo.pFun)
            {
                pRdp->m_stLauchRdp.stAppInfo.rdpOnVmNum = appList.rdpOnVmNum;
                pRdp->m_stLauchRdp.stAppInfo.rdpServiceState = appList.rdpServiceState;
                pRdp->m_stLauchRdp.stAppInfo.vmState = appList.vmState;
                pRdp->m_stLauchRdp.stAppInfo.powerOnVmNum = appList.powerOnVmNum;
                pRdp->m_stLauchRdp.stAppInfo.displayprotocol = appList.displayprotocol;
                pRdp->m_pCallBackInfo.pFun(TYPE_DESKTOP_NOT_AVAILABLE, 1, (void*)(&(pRdp->m_pCallBackInfo)), (void*)(&(pRdp->m_stLauchRdp.stAppInfo)));
            }
            pRdp->m_event.clear();
            return 0;
        }
    }
    else
    {
        return pRdp->runDesktopInProc();
    }
}

int CFrontEndRdp::reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk)
{
    if(m_bUserRequestQuit || iReloadDisk==ONLY_RETURN)
    {
        LOG_ERR("doesnot going to create process.m_bUserRequestQuit:%d, iReloadDisk:%d", m_bUserRequestQuit, (int)iReloadDisk);
        m_event.clear();
        return 0;
    }
    m_iReloadDisk = iReloadDisk;
    return CThread::createThread(NULL, NULL, (FUN_THREAD)(&reLaunchApp), (void*)this);
}

int CFrontEndRdp::runDesktopInProc()
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
            m_pCallBackInfo.pFun(TYPE_LAUNCH_FORM_CMDPARAM_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchRdp.stAppInfo));
        }
        m_event.clear();
        return iRet;
    }

    string cmd(APP_NAME_RDP);
    cmd = cmd + " " + m_strCmdLine.c_str();

//    int iExitCode = 0, iExitCodeLastTime = 0, iRunTimes = 0;
//    while (true)
//    {//try to reconnect when app quit in an unusual way
//        if(0 != iRunTimes)
//            Sleep(2000);
//        if(m_bUserRequestQuit)
//            break;
//        iExitCodeLastTime = iExitCode;
//        iExitCode = runInSubProcAndWait(APP_NAME_RDP, cmd, m_stLauchRdp.stAppInfo, m_stLauchRdp.bAttachDisk);
//        if(0!=iExitCode && 9!=iExitCode) //9 :means fap time-out-quit
//        {
//            if(iExitCode==iExitCodeLastTime || 0 == iRunTimes)
//                iRunTimes++;
//            else
//                iRunTimes = 0;
//        }
//        else
//        {
//            break;
//        }
//        if(iRunTimes > 3 || m_bUserRequestQuit)
//            break;
//    }


    int iExitCode = runInSubProcAndWait(APP_NAME_RDP, cmd, m_stLauchRdp.stAppInfo);
//    if(exitCode < 0)
//    {
//        LOG_ERR("runInSubProcAndWait failed. return value:%d", exitCode);
//    }

    iRet = vaccessShutDownResource();
    if(NULL != m_pCallBackInfo.pFun)
    {
        int iValue = iRet;
        if(131 == iExitCode)   // the new frdp errno = 131  launch fail;
            iValue = ERROR_CODE_LAUNCHPROC_QUIT_210;
        if(132 == iExitCode) // the new frdp errno = 132  time out;
            iValue = ERROR_CODE_LAUNCHPROC_RDP_TIME_OUT_9;
        m_pCallBackInfo.pFun(TYPE_LAUNCH_SHUTDOWN_RES_FINISHED, iValue, (void*)(&m_pCallBackInfo), (void*)(&m_stLauchRdp.stAppInfo));
    }
    m_event.clear();
    return iRet;
}

int CFrontEndRdp::formCmdParam_win()
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
    if(strlen(m_stLauchRdp.stAppInfo.name) > 0)
    {
        m_strCmdLine = m_strCmdLine + " -T \"" + m_stLauchRdp.stAppInfo.name + "\"";
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
            else if(3 == m_resParam.disk)
            {
                m_strCmdLine = m_strCmdLine + " disk:*:hotplug:wo";
            }
        }
        if(1 == m_resParam.parallelPort)
        {
            //if(0 == m_stLauchRdp.stPort.parallelLocal)
            //{
            //    m_strCmdLine = m_strCmdLine + " parallel:LPT2:LPT1";
            //}
            //else
            //{
                char remote[MIN_LEN], local[MIN_LEN];
                sprintf(remote, "%d", m_stLauchRdp.stPort.parallelRemote+1);
                sprintf(local, "%d", m_stLauchRdp.stPort.parallelLocal+1);
                m_strCmdLine = m_strCmdLine + " parallel:LPT" + remote + ":LPT" + local;
            //}
        }
        if(1 == m_resParam.serialPort)
        {
            //if( 0 == m_stLauchRdp.stPort.serialLocal)
            //{
            //    m_strCmdLine = m_strCmdLine + " serial:COM1:COM1";//??? is all COM1???
            //}
            //else
            //{
                char remote[MIN_LEN], local[MIN_LEN];
                sprintf(remote, "%d", m_stLauchRdp.stPort.serialRemote+1);
                sprintf(local, "%d", m_stLauchRdp.stPort.serialLocal+1);
//                itoa(m_stLauchRdp.stPort.serialRemote, remote, 10);
//                itoa(m_stLauchRdp.stPort.serialLocal, local, 10);
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
        else if(4 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " o";
        else if(5 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " p";
        else if(6 == m_resParam.clipboard)
            m_strCmdLine = m_strCmdLine + " q";
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
    if(HASBAR_STATE == m_stLauchRdp.barStatus)
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

/* rdp_cmd_param:  /usr/bin/frdp   /bpp:32 /u:pk /p:111111 /f /title:"sd-zy-win7" +compression /network:lan -sec-tls -sec-nla /cert-ignore /multimedia:decoder:gstreamer /on-float /port:60511 /v:10.10.46.206 */
int CFrontEndRdp::formCmdParam_unix()
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

    if(m_resData.stResInfo.iIdleTime>=0)
    {//time out
        char caTime[32];
        memset(caTime, 0, 32);
        sprintf(caTime, "%d", m_resData.stResInfo.iIdleTime);
        m_strCmdLine = m_strCmdLine + "/set-timer:" + caTime;
    }

    //m_strCmdLine = m_strCmdLine + "-a 32"; //color deeps
    m_strCmdLine = m_strCmdLine  + " /bpp:32";  // rdp_cmd_param(1)
//add ntAccountInfo to Param
    if(strlen(stAccountInfo.ntUsername)>0)
    {
        //m_strCmdLine = m_strCmdLine + " /u:" + stAccountInfo.ntUsername;  // rdp_cmd_param(2)
        m_strCmdLine = m_strCmdLine + " -u " + stAccountInfo.ntUsername;  // rdp_cmd_param(2)
    }
    if(strlen(stAccountInfo.ntPassword)>0)
    {
        //m_strCmdLine = m_strCmdLine + " /p:" + stAccountInfo.ntPassword; // rdp_cmd_param(3)
        m_strCmdLine = m_strCmdLine + " -p " + stAccountInfo.ntPassword; // rdp_cmd_param(3)
    }
    if(strlen(stAccountInfo.ntDomain)>0)
    {
        char ntDomain[MAX_LEN];
        strcpy(ntDomain, stAccountInfo.ntDomain);
        char* pIndex = strchr(ntDomain, '.');
        if(NULL != pIndex)
        {
            ntDomain[strlen(ntDomain) - strlen(pIndex)] = '\0';
        }
        m_strCmdLine = m_strCmdLine + " /d:\"" + ntDomain + "\"";
        cout << "cmdline ntDomain:" << ntDomain << endl;
    }

    if(strlen(m_resParam.alternateShell) > 0)
    {
        m_strCmdLine = m_strCmdLine + " /shell:\"" + m_resParam.alternateShell + "\"";
    }
    if(strlen(m_resParam.workingDir) > 0)
    {//work dir
        m_strCmdLine = m_strCmdLine + " /shell-dir:\"" + m_resParam.workingDir + "\"";
    }
    //m_strCmdLine = m_strCmdLine + " /f";// rdp_cmd_param(4)--->fullscreen
    m_strCmdLine = m_strCmdLine + "  -f ";// rdp_cmd_param(4)--->fullscreen
    if(strlen(m_stLauchRdp.stAppInfo.name) > 0)
    {
        m_strCmdLine = m_strCmdLine + " /title:\"" + m_stLauchRdp.stAppInfo.name + "\""; //rdp_cmd_param(5)
    }
    if(1 == m_resParam.protocol)
    {
        if(stNetwork.stPresentServer.isHttps)
            m_strCmdLine = m_strCmdLine + " /P:https://" + stNetwork.stPresentServer.serverAddress +":"+stNetwork.stPresentServer.port;
        else
            m_strCmdLine = m_strCmdLine + " /P:http://" + stNetwork.stPresentServer.serverAddress +":"+stNetwork.stPresentServer.port;
//        if(stNetwork.isHttps)
//            m_strCmdLine = m_strCmdLine + " -P https://" + stNetwork.presentServer +":"+stNetwork.port;
//        else
//            m_strCmdLine = m_strCmdLine + " -P http://" + stNetwork.presentServer +":"+stNetwork.port;
    }
    m_strCmdLine = m_strCmdLine + " +compression /network:lan -sec-tls -sec-nla /cert-ignore"; // rdp_cmd_param(6)

    if(m_resParam.disk>0 || 1==m_resParam.smartcard || 1==m_resParam.parallelPort || 1==m_resParam.serialPort)
    {
        LOG_INFO("m_needMapFileSystem: %d", m_bNeedMapFileSystem);
        /*
         *note: the disk map can be multi mapping;
         */
        if (m_resParam.disk>0 && strlen(m_resParam.usb)<=0 /*&& m_bNeedMapFileSystem*/)
        {
            switch(m_resParam.disk)
            {
            case 1:
                 m_strCmdLine = m_strCmdLine + " /drive:hotplug,*,ro";
                break;
            case 2:
                m_strCmdLine = m_strCmdLine + " /drive:hotplug,*,rw";
                break;
            case 3:
                m_strCmdLine = m_strCmdLine + " /drive:hotplug,*,wo";
                break;
            default: break;
            }
        }
        if(1 == m_resParam.smartcard)
        {
            m_strCmdLine = m_strCmdLine + " /smartcard:scard";
        }
        if(1 == m_resParam.parallelPort)
        {
            char *data = getCmdSystem("ls /dev/usb/lp* | grep lp");
            if(data!=NULL && strlen(data) == 0)
                free(data);
            else
            {
//                if(hasdisk == 0 )
//                    m_strCmdLine = m_strCmdLine + " --plugin rdpdr --data";
//                hasp_port = 1;
                char *p, *q, *par;

                q = data;
                par = getCmdSystem("echo $?");
                if (par[0] == '0')
                {
                    if ((p = strstr(q, "\n")) != NULL)
                    {
                        char parallelPort[64] = "/parallel:LPT1,";
                        char tmp[256];
                        strncpy(tmp, q, strlen(q) - strlen(p));
                        tmp[strlen(q) - strlen(p)] = '\0';
                        strcat(parallelPort, tmp);
                        m_strCmdLine = m_strCmdLine + " ";
                        m_strCmdLine = m_strCmdLine.append(parallelPort);
                        q = p + 1;
                    }
                    else {
                        char parallelPort[64] = "/parallel:LPT1,";
                        char tmp[256];
                        strncpy(tmp, q, strlen(q));
                        tmp[strlen(q)] = '\0';
                        strcat(parallelPort, tmp);
                        m_strCmdLine = m_strCmdLine + " ";
                        m_strCmdLine = m_strCmdLine.append(parallelPort);
                        qDebug() << "!!!!!!!!!!!!!!!!!m_strCmdLine: " << m_strCmdLine.c_str();
                    }
                }
                free(par);
                free(data);
            }
        }
        if(1 == m_resParam.serialPort)
        {

            qDebug() << "We got here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            char *data = getCmdSystem("ls /dev/ttyUSB* | grep ttyUSB");
            if(data!=NULL && strlen(data) == 0)
                free(data);
            else
            {
//                if(hasdisk == 0 && hass_port==0)
//                    m_strCmdLine = m_strCmdLine + " --plugin rdpdr --data";
//                hass_port = 1;
                char *p, *q, *par;

                q = data;
                par = getCmdSystem("echo $?");
                qDebug() << "par!!!!!!!!!!!!!!!!!!: " << par;
                if (par[0] == '0')
                {
                    if ((p = strstr(q, "\n")) != NULL)
                    {
                        char serialPort[64] = "/serial:COM1,";
                        char tmp[256];
                        strncpy(tmp, q, strlen(q) - strlen(p));
                        tmp[strlen(q) - strlen(p)] = '\0';
                        strcat(serialPort, tmp);
                        m_strCmdLine = m_strCmdLine + " ";
                        m_strCmdLine = m_strCmdLine.append(serialPort);
                        q = p + 1;
                        qDebug() << "!!!!!!!!!!!!!!!!!m_strCmdLine: " << m_strCmdLine.c_str();
                    }
                    else {
                        char serialPort[64] = "/serial:COM1,";
                        char tmp[256];
                        strncpy(tmp, q, strlen(q));
                        tmp[strlen(q)] = '\0';
                        strcat(serialPort, tmp);
                        m_strCmdLine = m_strCmdLine + " ";
                        m_strCmdLine = m_strCmdLine.append(serialPort);
                        qDebug() << "!!!!!!!!!!!!!!!!!m_strCmdLine: " << m_strCmdLine.c_str();
                    }
                }
                free(par);
                free(data);
            }
        }
//        if(hasdisk==1 || hasp_port==1 || hass_port==1)
//            m_strCmdLine = m_strCmdLine + " --";
    }

    if(1 == m_resParam.audio)
        m_strCmdLine = m_strCmdLine + " /sound:sys:pulse";

    if(1 == m_resParam.audioIn)
    {
        m_strCmdLine = m_strCmdLine + " /microphone:sys:pulse";
//        m_strCmdLine = m_strCmdLine + " --plugin drdynvc --data";
//        if (m_resParam.audioIn == 1)
//        {
//            m_strCmdLine = m_strCmdLine + " audin";
//        }
//        if (m_resParam.audio == 1)
//        {
//            m_strCmdLine = m_strCmdLine + " tsmf:audio:alsa";
//        }
//        m_strCmdLine = m_strCmdLine + " --";
    }

    m_strCmdLine = m_strCmdLine + " /multimedia:decoder:gstreamer";  // rdp_cmd_param(7)

    if( m_resParam.clipboard > 0)
    {
        m_strCmdLine = m_strCmdLine + " +clipboard";
    }
    if(1 == m_resParam.printer)
    {
        m_strCmdLine = m_strCmdLine + " /printer:fprinter";
    }

    if(strlen(m_resParam.rail.applicationName)>0)
    {
        char temp[MAX_LEN];
        strcpy(temp, m_resParam.rail.applicationName);
        for (size_t j = 0; j < strlen(temp); j++)
            if (temp[j] == ':')
                temp[j] = '|';
        m_strCmdLine = m_strCmdLine + " /app:\"" + temp + "\"";
        if (strlen(m_resParam.rail.workingDirectory) > 0)
        {
            strcpy(temp, m_resParam.rail.workingDirectory);
            for (size_t j = 0; j < strlen(temp); j++)
                if (temp[j] == ':')
                    temp[j] = '|';
            m_strCmdLine = m_strCmdLine + " /app-name:\"" + temp + "\"";
        }
//        m_strCmdLine = m_strCmdLine + ":";
        if (strlen(m_resParam.rail.arguments) > 0)
        {
            strcpy(temp, m_resParam.rail.arguments);
            for (size_t j = 0; j < strlen(temp); j++)
                if (temp[j] == ':')
                    temp[j] = '|';
            m_strCmdLine = m_strCmdLine + " /app-cmd:\"" + temp + "\"";
        }
//        m_strCmdLine = m_strCmdLine + " --";
    }

    // In Linux, we always has float bar
    m_strCmdLine = m_strCmdLine +  " /on-float"; // rdp_cmd_param(8)

    //m_strCmdLine = m_strCmdLine + " /port:"; //rdp_cmd_param(9)
    m_strCmdLine = m_strCmdLine + " -port ";
    if (strlen(m_resData.stResInfo.port) > 0)
        m_strCmdLine = m_strCmdLine +  m_resData.stResInfo.port;
     else
        m_strCmdLine = m_strCmdLine + "3389";

    //m_strCmdLine = m_strCmdLine + " /v:";
    m_strCmdLine = m_strCmdLine + " -v ";
    if(0 == strlen(m_resData.stResInfo.ipAddr))
    {
        m_strCmdLine = m_strCmdLine + stNetwork.stPresentServer.serverAddress;//rdp_cmd_param(10) server ip
    }
    else
    {
        m_strCmdLine = m_strCmdLine + m_resData.stResInfo.ipAddr;
    }

    /*haproxy*/
    if(strlen(m_resData.stResInfo.Uuid) > 0 && 1 != m_resParam.protocol)
    {
            m_strCmdLine = m_strCmdLine + " -ha " + m_resData.stResInfo.Uuid;
    }

    cout << m_strCmdLine.c_str() << endl;
    LOG_INFO("cmd:\t%s", m_strCmdLine.c_str());
    return 0;
}


int CFrontEndRdp::setForeground(LAUNCH_TYPE type, const char *titleName )
{
    if( titleName != NULL){
        LOG_INFO("launchtype: %d, titlename: %s", type, titleName);
    }
    if(procStatus()<0)
        return -1;
#ifdef _WIN32
    if(VIRTUALAPP != m_stLauchRdp.stAppInfo.desktopType)
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
