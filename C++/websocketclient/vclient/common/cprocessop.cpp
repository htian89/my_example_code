#include "cprocessop.h"
#include "../common/log.h"
#include <stdlib.h>
#include <QDebug>
#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "../common/ds_launchapp.h"

#define KEY 0x383838
#define SIZE 128

CProcessOp::CProcessOp(const std::string appName, const std::string cmdParam):
    m_strAppName(appName), m_strCmdParam(cmdParam)
{
    signal(SIGINT, SIG_DFL);
    m_usbId = 0;
    m_iExitCode = 0;
#ifdef _WIN32
    ZeroMemory( &m_procInfo, sizeof(m_procInfo) );
#endif
    LOG_INFO("appName:%s\tcmdParam:%s", appName.c_str(), cmdParam.c_str());
}

CProcessOp::~CProcessOp()
{
#ifdef _WIN32
    if(NULL != m_procInfo.hProcess)
    {
        CloseHandle(m_procInfo.hProcess);
        CloseHandle(m_procInfo.hThread);
        m_procInfo.hProcess = NULL;
    }
#endif
}

int CProcessOp::create(char *pWorkdir /*=NULL*/, int iShowWindow /*= 1*/)
{
#ifdef _WIN32
    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &m_procInfo, sizeof(m_procInfo) );
    if(0 == iShowWindow)
        si.dwFlags = STARTF_USESHOWWINDOW;
    char* pchAppName = NULL, *pchCmdParam = NULL;
    if(m_strAppName.length()>0)
        pchAppName = (char*)m_strAppName.c_str();
    if(m_strCmdParam.length()>0)
        pchCmdParam = (char*)m_strCmdParam.c_str();
    wchar_t wchAppName[MAX_PATH*8], wchCmdParam[MAX_PATH*8], wchWorkingDir[MAX_PATH*8];
    memset(wchAppName, 0, MAX_PATH*8);
    memset(wchCmdParam, 0, MAX_PATH*8);
    memset(wchWorkingDir, 0, MAX_PATH*8);
    //transcoding the appname, parameters to utf-8
    if(NULL != pchAppName)
    {
        int iRet = MultiByteToWideChar(CP_UTF8, 0, pchAppName, strlen(pchAppName), wchAppName, MAX_PATH*8);
        if(iRet <= 0)
            LOG_ERR("MultiByteToWideChar failed. reason:%d", GetLastError());
    }
    if(NULL != pchCmdParam)
    {//MultiByteToWideChar
        int iRet = MultiByteToWideChar(CP_UTF8, 0, pchCmdParam, strlen(pchCmdParam), wchCmdParam, MAX_PATH*8);
        if(iRet <= 0)
            LOG_ERR("MultiByteToWideChar failed. reason:%d", GetLastError());
    }
    if(NULL != pWorkdir)
    {//MultiByteToWideChar
        int iRet = MultiByteToWideChar(CP_UTF8, 0, pWorkdir, strlen(pWorkdir), wchWorkingDir, MAX_PATH*8);
        if(iRet <= 0)
            LOG_ERR("MultiByteToWideChar failed. reason:%d", GetLastError());
    }
    wchar_t* pwchAppName = NULL, *pwchCmdParam = NULL, *pwchWorkingDir = NULL;
    if(wcslen(wchAppName)>0)
        pwchAppName = wchAppName;
    if(wcslen(wchCmdParam)>0)
        pwchCmdParam = wchCmdParam;
    if(wcslen(wchWorkingDir)>0)
        pwchWorkingDir = wchWorkingDir;
    //run app in process
    if( !CreateProcessW(pwchAppName, pwchCmdParam, NULL, NULL, FALSE, 0, NULL, pwchWorkingDir, &si, &m_procInfo) )
    {
        LOG_ERR("create process failed. reason:%d", GetLastError());
        return -1;
    }
    return 0;
#else
    const char* pchAppName ;
    const char* pchCmdParam;
    //    char **switchCommand = (char **)calloc(sizeof(char *), 64);
    //    const char *p = NULL;
    if(m_strAppName.length()>0)
        pchAppName = m_strAppName.c_str();

    //    qDebug() << "Launch usb params: ------------------------------";
    if(m_strCmdParam.length()>0)
    {
        pchCmdParam = m_strCmdParam.c_str();

        /************
        for(int i=0; i<64; ++i)
        {
            p = strstr(pchCmdParam, " ");
            if(p!=NULL)
            {
                switchCommand[i] = (char *)calloc(1, 128);
                strncpy(switchCommand[i], pchCmdParam, p-pchCmdParam);
                pchCmdParam = ++p;
            }
            else
            {
                switchCommand[i] = (char *)calloc(1, 128);
                strncpy(switchCommand[i], pchCmdParam, strlen(pchCmdParam));
                switchCommand[++i] = NULL;
                break;
            }
        }
********************/
    }


    if( strstr(m_strAppName.c_str(), "tcp-usbip") != NULL )
    {
        m_shmid = shmget(KEY, SIZE, IPC_CREAT|0600);
    }
    m_processId = fork();
    if(m_processId < 0)
    {
        shmctl(m_shmid, IPC_RMID, NULL);
        LOG_ERR("%s, errorCode =%d", "Create process error", errno);
        return -1;
    }
    else if(m_processId == 0)
    {
        if( strstr(m_strAppName.c_str(), "tcp-usbip") != NULL )
        {
            m_usbId = vfork();
            if(m_usbId < 0)
            {
                LOG_ERR("%s, errorCode =%d", "Create process error", errno);
                return -1;
            }
            else if(0 == m_usbId)
            {
                if(execl("/bin/sh", "sh", "-c", pchCmdParam, (char *) 0) < 0)
                {
                    exit(1);
                }
            }
            else
            {
                char *shmaddr = (char *)shmat( m_shmid, NULL, 0 ) ;
                if ( (int)shmaddr == -1 )
                {
                    perror("shmat addr error child") ;
                }
                sprintf(shmaddr, "%d", m_usbId);
                shmdt( shmaddr ) ;
                exit(0);
            }
        }
        else
        {
            if(execl("/bin/sh", "sh", "-c", pchCmdParam, (char *) 0) < 0)
            {
                exit(1);
            }
        }
    }
    else
    {
        LOG_INFO("process++ : %s, pid : %d", m_strAppName.c_str(), m_processId);
        if( strstr(m_strAppName.c_str(), "tcp-usbip") != NULL )
        {
            waitpid(m_processId, NULL, 0);
            char *shmaddr = (char *) shmat(m_shmid, NULL, 0 ) ;
            if ( (int)shmaddr == -1 )
            {
                perror("shmat addr error parent") ;
            }
            sscanf(shmaddr, "%d", &m_usbId);
            LOG_INFO("usb process pid: %d", m_usbId);
            shmdt( shmaddr );
            shmctl(m_shmid, IPC_RMID, NULL);
        }
    }

    //    for(int i=0; i<64; ++i)
    //    {
    //        if(switchCommand[i]!=NULL)
    //            free(switchCommand[i]);
    //    }
    //    free(switchCommand);

    return 0;
#endif
}

int CProcessOp::wait()
{
#ifdef _WIN32
    DWORD iRet = 0;
    LOG_INFO("going to call WaitForSingleObject:procHandle:%d",(int)m_procInfo.hProcess);
    iRet = WaitForSingleObject(m_procInfo.hProcess, INFINITE );
    GetExitCodeProcess(m_procInfo.hProcess, &m_iExitCode);
    LOG_INFO("WaitForSingleObject returned:procHandle:%d, return value:%d, exitCode:%d",(int)m_procInfo.hProcess, iRet, m_iExitCode);
    if(NULL != m_procInfo.hProcess)
    {
        CloseHandle(m_procInfo.hProcess);
        CloseHandle(m_procInfo.hThread);
    }
    ZeroMemory( &m_procInfo, sizeof(m_procInfo) );
    return iRet;
#else
    if(m_processId <=0)
        return -1;
    LOG_INFO("going to call waitpid:%d",(int)m_processId);
    int status;
    //    waitpid(m_processId, &status, 0);

    /***
     *
     * Because when connect to the desktop with usb mapping,
     *then only use waitpid once, then the waitpid return -1, and errno is
     *4( interupt system call), so if errno is 4, call the waitpid() again.
     *
     ***/
    if( strstr(m_strAppName.c_str(), "tcp-usbip") != NULL )
    {
        return 0;
    }
    int count  = 0;
    while(waitpid(m_processId, &status, 0)==-1)
    {
        if(errno==4)
        {
            if(1 < count++)
                break;

            continue;
        }
        else
            break;
    }
    if(WIFEXITED(status))
    {
        LOG_INFO("Process quit code = %d", WEXITSTATUS(status));
        m_iExitCode = WEXITSTATUS(status);
    }
    else
    {
        LOG_INFO("Process Error quit code = %d", WEXITSTATUS(status));
        m_iExitCode = -1;
    }
    return m_iExitCode;
#endif
}

int CProcessOp::termate()
{
#ifdef _WIN32
    LOG_INFO("TerminateProcess beging procHandle: %d", int(m_procInfo.hProcess));
    BOOL bRet =  TerminateProcess(m_procInfo.hProcess, 0);
    if(FALSE == bRet)
    {
        LOG_ERR("TerminateProcess failed . procHandle: %d, reason:%d", int(m_procInfo.hProcess),GetLastError());
        return -1;
    }
    else
    {
        LOG_INFO("TerminateProcess finished procHandle: %d", int(m_procInfo.hProcess));
        m_procInfo.hProcess = NULL;
    }
    return 0;
#else
    char cmdlf[MAXLINE]; // path to /proc/<pid>/cmdline
    char procname[MAXLINE]; // argv[0];
    char *end = NULL;
    FILE *fin;


    end = cmdlf;
    end += sprintf(end, "%s", "/proc/");
    if( strstr(m_strAppName.c_str(), "tcp-usbip") != NULL )
    {
        end += sprintf(end, "%ld", (long)m_usbId);
    }
    else
    {
        end += sprintf(end, "%ld", (long)m_processId);
    }
    end += sprintf(end, "%s", "/cmdline");
    end += sprintf(end, "%c", '\0'); // null terminal

    LOG_INFO("cmdlf: %s", cmdlf);

    if ( (fin=fopen(cmdlf, "r")) == NULL) {
        LOG_ERR("can't open cmdline file in /proc");
        goto KILLTERM;
    }
    if ( fgets(procname, MAXLINE, fin) == NULL) {
        LOG_ERR("can't read %s", cmdlf);
        goto KILLTERM;
    }

    LOG_INFO("procname: %s", procname);

    fclose(fin);

    if ( 0 == strcmp(procname, APP_NAME_RDP)) {
        kill(m_processId, SIGTERM);
    } else if ( 0 == strcmp(procname, APP_NAME_FAP)) {
        LOG_INFO("kill(m_processId, SIGINT);");
        kill(m_processId, SIGINT);
    } else if ( 0 == strcmp(procname, APP_NAME_TERMINAL)) {
        kill(m_processId, SIGTERM);
    } else if ( 0 == strcmp(procname, APP_FUSB_TCP_PATH)) {
        kill(m_usbId, SIGINT);
        LOG_INFO("tcp-usbip killed by SIGINT");
    } else {
        goto KILLTERM; //fail safe
    }

    LOG_INFO("TerminateProcess finished procHandle: %d", m_processId);
    return 0;

KILLTERM:
    kill(m_processId, SIGTERM); // in some situations they will ignore signal SIGINT, then vclient will hang up

    LOG_INFO("TerminateProcess finished procHandle: %d", m_processId);
    return 0;
#endif

}

bool CProcessOp::procQuit()
{
#ifdef _WIN32
    if(NULL == m_procInfo.hProcess)
        return false;
    DWORD iRet = 0;
    LOG_INFO("going to call WaitForSingleObject:procHandle:%d",(int)m_procInfo.hProcess);
    iRet = WaitForSingleObject(m_procInfo.hProcess, 0 );
    if(WAIT_OBJECT_0 == iRet)
        return true;
    else
        return false;//the proc is running
#else
    int ret = waitpid(m_processId, NULL, WNOHANG);
    if(ret == 0)
        return true;
    else
        return false;
#endif
}
