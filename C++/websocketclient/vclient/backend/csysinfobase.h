#ifndef CSYSINFOBASE_H
#define CSYSINFOBASE_H
#include <string>
class CSysInfoBase
{
public:
    virtual void getProductVersion(char *value) = 0;
    virtual void getProductName(char *value) = 0;
    virtual void getProductUuid(char *value) = 0;
    virtual void getHostName(char *value) = 0;
    virtual void getIpAddress(char *value) = 0;
    virtual void getNetmask(char *value) = 0;
    virtual void getMacAddress(char *value) = 0;
    virtual void getOsVersion(char *value) =0 ;
    virtual void getCpuInfo(char *value) = 0;
    virtual void getMemory(char *value) = 0;
    virtual void getGraphicsCard(char *value) = 0;
    virtual void getSoundCard(char *value) = 0;
    virtual void setTerminalFuncSwitch(char *name, int value) = 0;
};

#ifdef WIN32
class CSysInfoWin : public CSysInfoBase
{
public:
    CSysInfoWin();
    void getProductVersion(char *value);
    void getProductName(char *value);
    void getProductUuid(char *value);
    virtual void getHostName(char *value);
    virtual void getIpAddress(char *value);
    virtual void getNetmask(char *value);
    virtual void getMacAddress(char *value);
    virtual void getOsVersion(char *value);
    virtual void getCpuInfo(char *value);
    virtual void getMemory(char *value);
    virtual void getGraphicsCard(char *value);
    virtual void getSoundCard(char *value);
private:
    int getNetworkinfo();
    std::string m_str_hostName, m_str_ip, m_str_netmask, m_str_macAdd;
    bool m_bHasGotNetworkInfo;
};
#else

class CSysInfoUnix : public CSysInfoBase
{
public:
    void getProductVersion(char *value);
    void getProductName(char *value);
    void getProductUuid(char *value);
    virtual void getHostName(char *value);
    virtual void getIpAddress(char *value);
    virtual void getNetmask(char *value);
    virtual void getMacAddress(char *value);
    virtual void getOsVersion(char *value);
    virtual void getCpuInfo(char *value);
    virtual void getMemory(char *value);
    virtual void getGraphicsCard(char *value);
    virtual void getSoundCard(char *value);
    virtual void setTerminalFuncSwitch(char *name, int value);

private:
    char *excuteSystemCmdInfo(const char *cmd);
    std::string excuteSystemCmdInfo2(const char *cmd);
    std::string getFirstLine(std::string &value);
    void copyValue(char *dest, const char *src);
};
#endif

#endif // CSYSINFOBASE_H
