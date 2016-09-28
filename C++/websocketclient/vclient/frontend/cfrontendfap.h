#ifndef CFRONTENDFAP_H
#define CFRONTENDFAP_H
#include <string>
#include "cfrontendbase.h"
#include "../common/ds_settings.h"

class CFrontEndFap : public CFrontEndBase
{
public:
    CFrontEndFap(const LAUCH_FAP& stLaunchFap, LAUNCH_CALLBACK_INFO pCallInfo);
    virtual ~CFrontEndFap(){}
    int Launch();
    int reloadVDiskInNewDesktop(RELOAD_VDISK_TYPE iReloadDisk);
    int runDesktopInProc();
    int formCmdParam_win();
    int formCmdParam_unix();
    virtual LAUNCH_TYPE LaunchType() const{ return LAUNCH_TYPE_FAP; }

    RELOAD_VDISK_TYPE m_iReloadDisk;
    LAUCH_FAP m_stLauchFap;
private:
    std::string  m_strCmdLine;
#ifdef unix
    bool m_bNeedMapUsb;
    bool m_bNeedMapFlashDisk;
    bool m_bNeedMapHardDisk;
    bool m_bNeedMapPrint;
    bool m_bNeedMapCDRom;
    bool m_bNeedMapOthers;
#endif
};

#endif // CFRONTENDFAP_H
