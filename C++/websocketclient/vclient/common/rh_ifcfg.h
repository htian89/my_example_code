#ifndef RH_IFCFG_H
#define RH_IFCFG_H
#include <QWidget>
#include <QString>
//#include "../common/MultiAccesses.h"
typedef struct {
    char name[20];
    char ip[20];
    char netmask[20];
    char gateway[20];
    char dns1[20];
} IpInfo;
class rh_ifcfg
{

public:
    explicit rh_ifcfg();
    bool get_value(QString key, QString *value);
    bool set_value(QString key, QString value);
    void initIfcfgPath();
    int get_if_info(int fd);

    ~rh_ifcfg();
    IpInfo getIpInfo(){return m_ipInfo;}
public:
//    AccessStruct m_stNetInfo;
    char m_netPortName[16];
private:
    QString m_ifcfg_filepath;
    IpInfo m_ipInfo;
};


#endif // RH_IFCFG_H
