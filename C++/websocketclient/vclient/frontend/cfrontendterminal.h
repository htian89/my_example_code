#ifndef CFRONTENDTERMINAL_H
#define CFRONTENDTERMINAL_H

#include <string>
#include "cfrontendbase.h"
#include "../common/ds_launchapp.h"

class CFrontEndTerminal : public CFrontEndBase
{
public:
    CFrontEndTerminal(const LAUNCH_TERMINAL& stLaunchRdp, LAUNCH_CALLBACK_INFO pCallInfo);
    //virtual ~CFrontEndTerminal(){}
    virtual ~CFrontEndTerminal();
    int Launch();
    //when load vdisk failed and the desktop type is desktoppool. we should do
    //based on the user's will. if the user selects connect to a new desktop and
    //attach vdisk again, we will redo most of the work. otherwise....
    int reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk);
    int runDesktopInProc();
    int formCmdParam_win();
    int formCmdParam_unix();
    static int getRunningCount() {return runningCount;}
    int setForeground(LAUNCH_TYPE, const char*);
    virtual LAUNCH_TYPE LaunchType() const{ return LAUNCH_TYPE_TERMINAL; }



    RELOAD_VDISK_TYPE m_iReloadDisk;
    LAUNCH_TERMINAL m_stLaunchTerminal;
private:
    static int runningCount;
    std::string  m_strCmdLine;
};


#endif // CFRONTENDTERMINAL_H
