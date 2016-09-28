#ifndef CPROCESSOP_H
#define CPROCESSOP_H
///***********************************************************
//* this classes provides process mange functions:create, wait,
//* termiate, get process exit code ....
//***********************************************************/
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <errno.h>
#define MAXLINE 100
#endif
typedef unsigned long DWORD;


class CProcessOp
{
public:
    /*
     *parameter:
     *  appName(std::string)[in]
     *      the application name need to run in a process
     *  cmdParam(std::string)[in]
     *      the parameter passed to the application
     **/
    CProcessOp(const std::string appName, const std::string cmdParam);
    ~CProcessOp();
    /*
     *functions:
     *      to run the application in a process
     *parameter:
     *  pWorkdir(char*)[in]
     *      work dir
     *  iShowWindow(int)[in]
     *      whether show window or not. 0 not show >0 show
     *return value:
     *      >=0 succeed
     *      < 0 failed
     */
    int create(char* pWorkdir = NULL, int iShowWindow = 1);
    /*
     *funcions:
     *      blocked until the process execute finished(returned)
     */
    int wait();
    /*
     *function:
     *      get the application return value. the value is valid
     *  only if the application has finished execution.
     **/
    int getExitCode(){  return (int)m_iExitCode; }
    /*
     *function:
     *      trys to terminate the applications.
     *return value:
     *      >=0 succeed
     *      < 0 failed
     **/
    int termate();

    /*
     *function:
     *      check if the process has quit
     *return value:
     *      true: the process has quit
     *     fasle  the process is running
     **/
    bool procQuit();
#ifdef WIN32
    DWORD getProcId() { return m_procInfo.dwProcessId;  }
#else
    pid_t getProcId(){return m_processId; }
#endif
private:
    std::string m_strAppName;
    std::string m_strCmdParam;
    DWORD m_iExitCode;
#ifdef _WIN32
    PROCESS_INFORMATION m_procInfo;
#else
    pid_t m_processId;
    pid_t m_usbId;
    int m_shmid;
#endif

};

#endif // CPROCESSOP_H
