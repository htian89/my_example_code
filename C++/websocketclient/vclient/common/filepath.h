#ifndef FILTPATH_H
#define FILTPATH_H

#include <QByteArray>
#include <stdlib.h>
#include "../config.h"

#ifdef _WIN32
	const QByteArray userPath = qgetenv("APPDATA") + "\\vclient\\";
	const QByteArray iconPath = userPath + "icon\\";
	const QByteArray updatePath = userPath + "update\\";
#else
const QByteArray userPath = QByteArray(getenv("HOME")) + "/.vclient/";
//	const QByteArray userPath = "/root/.vclient/";
const QByteArray iconPath = userPath + "icon/";
const QByteArray updatePath = userPath + "update/";
#endif

const QByteArray networkSettingPath = userPath + "networksettinginfo";
const QByteArray loginSettingPath = userPath + "loginsettinginfo";
const QByteArray configSoftwareFilePath = userPath +"config.ini";
const QByteArray userInfoPath = userPath + "userlogininfo";
const QByteArray positionPath = userPath + "fronware_vclient_position";

#define USERPATH_STORFILE "userpath"
#endif // FILTPATH_H
