#ifndef CCONFIGINFO_H
#define CCONFIGINFO_H
#include <string>
#include "tinyxml/tinyxml.h"
#include "ds_settings.h"

class CConfigInfo
{
public:
    CConfigInfo(const char *path);

    int getSettings_vclient(SETTINGS_VCLIENT& settings);
    int setSettings_vclient(const SETTINGS_VCLIENT& settings);
    int getSettings_login(SETTINGS_LOGIN& userInfo);
    int setSettings_login(const SETTINGS_LOGIN& userInfo);
    int setSetting_defaultDesktop(const SETTING_DEFAULTAPP &defaultApp);
    int getSetting_defaultDesktop(SETTING_DEFAULTAPP &defaultApp);

    int getAccessIp(char *AccessIp);
    int setAccessIp(const char *AccessIp);

//    int getResourceSettings();
//    int setResourceSettings();

    int saveSettings();

    int loadSettings();

    //int

private:
    SETTINGS_VCLIENT m_settings_vclient;
    SETTINGS_LOGIN m_settings_login;
    SETTING_DEFAULTAPP m_settings_defaultApp;
    std::string m_path;

};

#endif // CCONFIGINFO_H
