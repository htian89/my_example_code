#ifndef CFRONTENDRDP_H
#define CFRONTENDRDP_H
#include <string>
#include "cfrontendbase.h"
#include "../common/ds_launchapp.h"

class CFrontEndRdp : public CFrontEndBase
{
public:
    CFrontEndRdp(const LAUCH_RDP& stLaunchRdp, LAUNCH_CALLBACK_INFO pCallInfo);
    virtual ~CFrontEndRdp(){}
    int Launch();
    //when load vdisk failed and the desktop type is desktoppool. we should do
    //based on the user's will. if the user selects connect to a new desktop and
    //attach vdisk again, we will redo most of the work. otherwise....
    int reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk);
    int runDesktopInProc();
    int formCmdParam_win();
    int formCmdParam_unix();
    int setForeground(LAUNCH_TYPE, const char*);
    virtual LAUNCH_TYPE LaunchType() const{ return LAUNCH_TYPE_RDP; }

    RELOAD_VDISK_TYPE m_iReloadDisk;
    LAUCH_RDP m_stLauchRdp;
private:    
    std::string  m_strCmdLine;
};

#endif // CFRONTENDRDP_H
