#ifndef GLOBALDEFINE_H
#define GLOBALDEFINE_H

#include <QByteArray>
#include <stdlib.h>

const QByteArray userPath = QByteArray(getenv("HOME")) + "/.vclient/";
const QByteArray logPath = userPath + "updatelog";
const QByteArray networkPath = userPath + "config.xml";
const QByteArray versionFile = QByteArray("/opt/vclient/vclient_version");
const int  MAX_LEN = 512;
struct NetWorkInfo{
    char ipAddress[MAX_LEN];
    char port[32];
};
typedef NetWorkInfo NETWORKINFO;


#endif // GLOBALDEFINE_H


