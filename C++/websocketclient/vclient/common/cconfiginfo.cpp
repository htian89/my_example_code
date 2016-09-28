#include "cconfiginfo.h"
#include "log.h"
#include <iostream>
using namespace std;

CConfigInfo::CConfigInfo(const char *path)
{
    if(NULL == path)
    {
        LOG_ERR("%s","parameter(in) error:NULL == path");
    }
    else
    {
        m_path = path;
    }
    memset(&m_settings_login, 0, sizeof(SETTINGS_LOGIN));
    memset(&m_settings_vclient, 0, sizeof(SETTINGS_VCLIENT));
    memset(&m_settings_defaultApp, 0, sizeof(SETTING_DEFAULTAPP));

}

int CConfigInfo::getSettings_vclient(SETTINGS_VCLIENT &settings)
{
    int iRet = loadSettings();
    if(iRet < 0)
        return iRet;
    settings = m_settings_vclient;
    return 0;
}

int CConfigInfo::getAccessIp(char *AccessIp)
{
    SETTINGS_VCLIENT settings;
    int iRet = getSettings_vclient(settings);
    if (iRet < 0)
        return iRet;
    strcpy(AccessIp, settings.m_network.stFirstServer.serverAddress);
    return 0;
}

int CConfigInfo::setSettings_vclient(const SETTINGS_VCLIENT &settings)
{
    loadSettings();
    m_settings_vclient = settings;
    return saveSettings();
}

int CConfigInfo::setAccessIp(const char *AccessIp)
{
    SETTINGS_VCLIENT settings;
    int iRet = getSettings_vclient(settings);
    if (iRet < 0)
        return iRet;
    strcpy(settings.m_network.stFirstServer.serverAddress, AccessIp);
    iRet = setSettings_vclient(settings);
    if (iRet < 0)
        return iRet;
    return 0;
}

int CConfigInfo::getSettings_login(SETTINGS_LOGIN& userInfo)
{
    int iRet = loadSettings();
    if(iRet < 0)
        return iRet;
    userInfo = m_settings_login;
    return 0;
}

int CConfigInfo::setSettings_login(const SETTINGS_LOGIN& userInfo)
{
    loadSettings();
    m_settings_login = userInfo;
    return saveSettings();
}

int CConfigInfo::setSetting_defaultDesktop(const SETTING_DEFAULTAPP &defaultApp)
{
    loadSettings();
    m_settings_defaultApp = defaultApp;
    return saveSettings();
}

int CConfigInfo::getSetting_defaultDesktop(SETTING_DEFAULTAPP &defaultApp)
{
    int iRet = loadSettings();
    if(iRet < 0)
        return iRet;
    defaultApp = m_settings_defaultApp;
    return 0;
}

int CConfigInfo::saveSettings()
{
    TiXmlDocument doc(m_path.c_str());
    TiXmlElement vClientSettings("VClientSettings");
//network node
    char caIsHttps[MIN_LEN];
    TiXmlElement elm_netWorkSettings( "NetworkSettings" );
    //save presentServer info
    TiXmlElement elm_presentServer( "presentServer" );
    TiXmlElement elm_port( "port" );
    TiXmlElement elm_isHttps( "isHttps" );
    elm_presentServer.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stPresentServer.serverAddress));
    elm_netWorkSettings.InsertEndChild(elm_presentServer);
    elm_port.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stPresentServer.port));
    elm_netWorkSettings.InsertEndChild(elm_port);
    memset(caIsHttps, 0, MIN_LEN);
    sprintf(caIsHttps, "%d", m_settings_vclient.m_network.stPresentServer.isHttps);
    elm_isHttps.InsertEndChild(TiXmlText(caIsHttps));
    elm_netWorkSettings.InsertEndChild(elm_isHttps);

    //save firstServer info
    TiXmlElement elm_firstServer( "firstServer" );
    TiXmlElement elm_firstServerPort( "firstServerPort" );
    TiXmlElement elm_firstServerIsHttps( "firstServerIsHttps" );
    elm_firstServer.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stFirstServer.serverAddress));
    elm_netWorkSettings.InsertEndChild(elm_firstServer);
    elm_firstServerPort.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stFirstServer.port));
    elm_netWorkSettings.InsertEndChild(elm_firstServerPort);
    memset(caIsHttps, 0, MIN_LEN);
    sprintf(caIsHttps, "%d", m_settings_vclient.m_network.stFirstServer.isHttps);
    elm_firstServerIsHttps.InsertEndChild(TiXmlText(caIsHttps));
    elm_netWorkSettings.InsertEndChild(elm_firstServerIsHttps);

    //save alternateServer info
    TiXmlElement elm_alternateServer( "alternateServer" );
    TiXmlElement elm_alternatePort( "alternateServerPort" );
    TiXmlElement elm_alternateIsHttps( "alternateServerIsHttps" );
    elm_alternateServer.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stAlternateServer.serverAddress));
    elm_netWorkSettings.InsertEndChild(elm_alternateServer);
    elm_alternatePort.InsertEndChild(TiXmlText(m_settings_vclient.m_network.stAlternateServer.port));
    elm_netWorkSettings.InsertEndChild(elm_alternatePort);
    memset(caIsHttps, 0, MIN_LEN);
    sprintf(caIsHttps, "%d", m_settings_vclient.m_network.stAlternateServer.isHttps);
    elm_alternateIsHttps.InsertEndChild(TiXmlText(caIsHttps));
    elm_netWorkSettings.InsertEndChild(elm_alternateIsHttps);

    vClientSettings.InsertEndChild(elm_netWorkSettings);

//file path map  node
    TiXmlElement elm_fileinfo("MapFileInfo");
    TiXmlElement elm_mapSet("MapSet");
    char mapsetBuf[MIN_LEN];
    memset(mapsetBuf, 0, sizeof(mapsetBuf));
    sprintf(mapsetBuf, "%d", m_settings_vclient.m_mapset);
    elm_mapSet.InsertEndChild(TiXmlText(mapsetBuf));
    elm_fileinfo.InsertEndChild(elm_mapSet);
    TiXmlElement elm_firstFilePath("FirstFilePath");
    TiXmlElement elm_firstFileSize("size");
    TiXmlElement elm_firstFileType("Type");
    elm_firstFilePath.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stFirstPath.filePath));
    elm_fileinfo.InsertEndChild(elm_firstFilePath);
    elm_firstFileSize.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stFirstPath.size));
    elm_fileinfo.InsertEndChild(elm_firstFileSize);
    elm_firstFileType.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stFirstPath.type));
    elm_fileinfo.InsertEndChild(elm_firstFileType);


    TiXmlElement elm_alternateFilePath("AlternateFilePath");
    TiXmlElement elm_alternateFileSize("size");
    TiXmlElement elm_alternateFileType("Type");
    elm_alternateFilePath.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stAlterNatePath.filePath));
    elm_fileinfo.InsertEndChild(elm_alternateFilePath);
    elm_alternateFileSize.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stAlterNatePath.size));
    elm_fileinfo.InsertEndChild(elm_alternateFileSize);
    elm_alternateFileType.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stAlterNatePath.type));
    elm_fileinfo.InsertEndChild(elm_alternateFileType);

    TiXmlElement elm_presentFilePath("PresentFilePath");
    TiXmlElement elm_presentFileSize("size");
    TiXmlElement elm_presentFileType("Type");
    elm_presentFilePath.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stPresentPath.filePath));
    elm_fileinfo.InsertEndChild(elm_presentFilePath);
    elm_presentFileSize.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stPresentPath.size));
    elm_fileinfo.InsertEndChild(elm_presentFileSize);
    elm_presentFileType.InsertEndChild(TiXmlText(m_settings_vclient.m_mapFilePathList.stPresentPath.type));
    elm_fileinfo.InsertEndChild(elm_presentFileType);

    vClientSettings.InsertEndChild(elm_fileinfo);
//has bar node
    TiXmlElement elm_barSet("BarSet");
    TiXmlElement elm_rdpBarSet("RDPBarSet");
    TiXmlElement elm_fapBarSet("FAPBarSet");
    char caRdpBarSet[MIN_LEN];
    sprintf(caRdpBarSet, "%d", m_settings_vclient.m_rdpBar);
//    itoa(m_settings_vclient.m_rdpBar, caRdpBarSet, 10);
    elm_rdpBarSet.InsertEndChild(TiXmlText(caRdpBarSet));
    elm_barSet.InsertEndChild(elm_rdpBarSet);
    char caFapBarSet[MIN_LEN];
    sprintf(caFapBarSet, "%d", m_settings_vclient.m_fapBar);
//    itoa(m_settings_vclient.m_fapBar, caFapBarSet, 10);
    elm_fapBarSet.InsertEndChild(TiXmlText(caFapBarSet));
    elm_barSet.InsertEndChild(elm_fapBarSet);
    vClientSettings.InsertEndChild(elm_barSet);

//update settings
    TiXmlElement elm_updatetype("UpdateType");
    char caUpdateType[MIN_LEN];
    sprintf(caUpdateType, "%d", m_settings_vclient.m_updateSetting);
//    itoa(m_settings_vclient.m_updateSetting, caUpdateType, 10);
    elm_updatetype.InsertEndChild(TiXmlText(caUpdateType));
    vClientSettings.InsertEndChild(elm_updatetype);

//auto connect to vAccess(server) settings
    TiXmlElement elm_autoConnectSetting("AutoConnectToServer");
    char caAutoConnectType[MIN_LEN];
    sprintf(caAutoConnectType, "%d", m_settings_vclient.iAutoConnectToServer);
    elm_autoConnectSetting.InsertEndChild(TiXmlText(caAutoConnectType));
    vClientSettings.InsertEndChild(elm_autoConnectSetting);

//userinfo settings
    TiXmlElement elm_userInfo("UserInfo");
    TiXmlElement elm_username("username");
    TiXmlElement elm_password("password");
    TiXmlElement elm_uuid("uuid");
    TiXmlElement elm_domain("domainCurrentUsed");
    TiXmlElement elm_remeber("remeber");
    TiXmlElement elm_autologin("autologin");
    TiXmlElement elm_attachDisk("attachDisk");
    elm_username.InsertEndChild(TiXmlText(m_settings_login.stUserInfo.username));
    elm_userInfo.InsertEndChild(elm_username);
    elm_password.InsertEndChild(TiXmlText(m_settings_login.stUserInfo.password));
    elm_userInfo.InsertEndChild(elm_password);
    elm_uuid.InsertEndChild(TiXmlText(m_settings_login.stUserInfo.uuid));
    elm_userInfo.InsertEndChild(elm_uuid);
    elm_domain.InsertEndChild(TiXmlText(m_settings_login.stUserInfo.domain));
    elm_userInfo.InsertEndChild(elm_domain);
    char ca_AutoLogin[MIN_LEN];
    sprintf(ca_AutoLogin, "%d", m_settings_login.iAutoLogin);
//    itoa(m_settings_login.iAutoLogin, ca_AutoLogin, 10);
    elm_autologin.InsertEndChild(TiXmlText(ca_AutoLogin));
    elm_userInfo.InsertEndChild(elm_autologin);
    char ca_Remeber[MIN_LEN];
    sprintf(ca_Remeber, "%d", m_settings_login.iRemember);
//    itoa(m_settings_login.iRemember, ca_Remeber, 10);
    elm_remeber.InsertEndChild(TiXmlText(ca_Remeber));
    elm_userInfo.InsertEndChild(elm_remeber);
    char ca_attachDisk[MIN_LEN];
    sprintf(ca_attachDisk, "%d", m_settings_login.iAttachVDisk);
    elm_attachDisk.InsertEndChild(TiXmlText(ca_attachDisk));
    elm_userInfo.InsertEndChild(elm_attachDisk);

    vClientSettings.InsertEndChild(elm_userInfo);

    //Save the default desktop
    TiXmlElement elm_defaultApp("DefaultApp");
    TiXmlElement elm_appUuid("appUuid");
    TiXmlElement elm_appName("appName");
    TiXmlElement elm_userName("userName");
    TiXmlElement elm_serverIp("serverIp");
    //TiXmlElement elm_attachvDisk("attachvDisk");
    TiXmlElement elm_mapUsb("mapUsb");
    TiXmlElement elm_connectProtocal("connectProtocal");
    TiXmlElement elm_isAutoConnect("isAutoConnect");
    elm_appUuid.InsertEndChild(TiXmlText(m_settings_defaultApp.uuid));
    elm_defaultApp.InsertEndChild(elm_appUuid);
    elm_appName.InsertEndChild(TiXmlText(m_settings_defaultApp.appName));
    elm_defaultApp.InsertEndChild(elm_appName);
    elm_userName.InsertEndChild(TiXmlText(m_settings_defaultApp.userName));
    elm_defaultApp.InsertEndChild(elm_userName);
    elm_serverIp.InsertEndChild(TiXmlText(m_settings_defaultApp.serverIp));
    elm_defaultApp.InsertEndChild(elm_serverIp);
    char intToChar[MIN_LEN];
    /*sprintf(intToChar, "%d", m_settings_defaultApp.isLoadvDisk);
//    itoa(m_settings_defaultApp.isLoadvDisk, intToChar, 10);
    elm_attachvDisk.InsertEndChild(TiXmlText(intToChar));
    elm_defaultApp.InsertEndChild(elm_attachvDisk);*/
    sprintf(intToChar, "%d", m_settings_defaultApp.isMapUsb);
//    itoa(m_settings_defaultApp.isMapUsb, intToChar, 10);
    elm_mapUsb.InsertEndChild(TiXmlText(intToChar));
    elm_defaultApp.InsertEndChild(elm_mapUsb);
    sprintf(intToChar, "%d", m_settings_defaultApp.connectProtocal);
//    itoa(m_settings_defaultApp.connectProtocal, intToChar, 10);
    elm_connectProtocal.InsertEndChild(TiXmlText(intToChar));
    elm_defaultApp.InsertEndChild(elm_connectProtocal);
    sprintf(intToChar, "%d", m_settings_defaultApp.isAutoConnect);
//    itoa(m_settings_defaultApp.isAutoLogin, intToChar, 10);
    elm_isAutoConnect.InsertEndChild(TiXmlText(intToChar));
    elm_defaultApp.InsertEndChild(elm_isAutoConnect);
    vClientSettings.InsertEndChild(elm_defaultApp);

    doc.InsertEndChild(vClientSettings);
    bool b_ret = doc.SaveFile();
    cout<<"save result:"<<b_ret<<endl;
    if(!b_ret)
    {
        LOG_ERR("%s","save to file failed");
        return -1;
    }
    return 0;
}

int CConfigInfo::loadSettings()
{
    TiXmlDocument doc(m_path.c_str());
    bool b_Ret = doc.LoadFile();
    if(!b_Ret)
    {
        LOG_ERR("%s", "load config file failed.");
        return -5;
    }
//
    TiXmlNode* pVClientSettings =  doc.FirstChild("VClientSettings");
    if(NULL == pVClientSettings)
    {
        LOG_ERR("%s", "no vclientsettings in config file");
        return -10;
    }
    memset(&(m_settings_vclient.m_network), 0, sizeof(NETWORK));
//get networkconfig
    TiXmlElement* pNetworkSettings = pVClientSettings->FirstChildElement("NetworkSettings");
    if(NULL == pNetworkSettings)
    {
        LOG_ERR("%s", "network not configed");
    }
    else
    {//should in compatable with the old format...  the old format is :
        /*
    <NetworkSettings>
        <firstServer>10.10.20.153</firstServer>
        <alternateServer></alternateServer>
        <presentServer>10.10.20.153</presentServer>
        <port>80</port>
        <isHttps>0</isHttps>
    </NetworkSettings>*/
        TiXmlElement* pPresentServer = pNetworkSettings->FirstChildElement("presentServer");
        if(NULL != pPresentServer)
        {
            const char* pData = pPresentServer->GetText();
            if(NULL != pData)
                strcpy(m_settings_vclient.m_network.stPresentServer.serverAddress, pData);//strcpy(m_settings_vclient.m_network.presentServer, pData);
        }
        TiXmlElement* pPort = pNetworkSettings->FirstChildElement("port");
        if(NULL != pPort)
        {
            const char* pData = pPort->GetText();
            if(NULL != pData)
            {
                strcpy(m_settings_vclient.m_network.stAlternateServer.port, pData);//strcpy(m_settings_vclient.m_network.port, pData);
                strcpy(m_settings_vclient.m_network.stFirstServer.port, pData);
                strcpy(m_settings_vclient.m_network.stPresentServer.port, pData);
            }
        }
        TiXmlElement* pIsHttps = pNetworkSettings->FirstChildElement("isHttps");
        if(NULL != pIsHttps)
        {
            const char* pData = pIsHttps->GetText();
            if(NULL != pData)
            {
                m_settings_vclient.m_network.stFirstServer.isHttps = atoi(pData);//m_settings_vclient.m_network.isHttps = atoi(pData);
                m_settings_vclient.m_network.stAlternateServer.isHttps = m_settings_vclient.m_network.stFirstServer.isHttps;
                m_settings_vclient.m_network.stPresentServer.isHttps = m_settings_vclient.m_network.stFirstServer.isHttps;
            }
        }
    //load first server info
        TiXmlElement* pFirstServer = pNetworkSettings->FirstChildElement("firstServer");
        if(NULL != pFirstServer)
        {
            const char* pData = pFirstServer->GetText();
            if(NULL != pData)
                strcpy(m_settings_vclient.m_network.stFirstServer.serverAddress, pData);
        }
        TiXmlElement* pFirstServerPort = pNetworkSettings->FirstChildElement("firstServerPort");
        if(NULL != pFirstServerPort)
        {
            const char* pData = pFirstServerPort->GetText();
            if(NULL != pData)
                strcpy(m_settings_vclient.m_network.stFirstServer.port, pData);
        }
        TiXmlElement* pFirstServerIsHttps = pNetworkSettings->FirstChildElement("firstServerIsHttps");
        if(NULL != pFirstServerIsHttps)
        {
            const char* pData = pFirstServerIsHttps->GetText();
            if(NULL != pData)
                m_settings_vclient.m_network.stFirstServer.isHttps = atoi(pData);
        }
    //load alternate server info
        TiXmlElement* pAlternateServer = pNetworkSettings->FirstChildElement("alternateServer");
        if(NULL != pAlternateServer)
        {
            const char* pData = pAlternateServer->GetText();
            if(NULL != pData)
                strcpy(m_settings_vclient.m_network.stAlternateServer.serverAddress, pData);//strcpy(m_settings_vclient.m_network.alternateServer, pData);
        }
        TiXmlElement* pAlternateServerPort = pNetworkSettings->FirstChildElement("alternateServerPort");
        if(NULL != pAlternateServerPort)
        {
            const char* pData = pAlternateServerPort->GetText();
            if(NULL != pData)
                strcpy(m_settings_vclient.m_network.stAlternateServer.port, pData);
        }
        TiXmlElement* pAlternateIsHttps = pNetworkSettings->FirstChildElement("alternateServerIsHttps");
        if(NULL != pAlternateIsHttps)
        {
            const char* pData = pAlternateIsHttps->GetText();
            if(NULL != pData)
                m_settings_vclient.m_network.stAlternateServer.isHttps = atoi(pData);
        }
    }
//get mapfilepath set
    memset(&(m_settings_vclient.m_mapFilePathList), 0, sizeof(m_settings_vclient.m_mapFilePathList));
    TiXmlElement *pMapFileInfo = pVClientSettings->FirstChildElement("MapFileInfo");
    if(NULL == pMapFileInfo){
        LOG_ERR("%s", "no map file path info in config file");
    }else{
        TiXmlElement *pMapfile = pMapFileInfo->FirstChildElement("MapSet");
        if(NULL != pMapfile){
            m_settings_vclient.m_mapset = UNKNOWN_MAP_STATUS;
            const char *pData = pMapfile->GetText();
            if(NULL != pData){
                m_settings_vclient.m_mapset = (DISK_MAP_STATUS)atoi(pData);
            }
        }
        TiXmlElement *pFirstFilepath = pMapFileInfo->FirstChildElement("FirstFilePath");
        if(NULL != pFirstFilepath){
            const char *pData = pFirstFilepath->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stFirstPath.filePath, pData);
            }
        }
        TiXmlElement *pFirstFileSize = pMapFileInfo->FirstChildElement("size");
        if(NULL != pFirstFileSize){
            const char *pData = pFirstFileSize->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stFirstPath.size, pData);
            }
        }
        TiXmlElement *pFirstFileType = pMapFileInfo->FirstChildElement("type");
        if(NULL != pFirstFileType){
            const char* pData = pFirstFileType->GetText();
            if( NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stFirstPath.type, pData);
            }
        }
        TiXmlElement *pAlternateFilepath = pMapFileInfo->FirstChildElement("AlternateFilePath");
        if(NULL != pAlternateFilepath){
            const char *pData = pAlternateFilepath->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stAlterNatePath.filePath, pData);
            }
        }
        TiXmlElement *pAlternateFileSize = pMapFileInfo->FirstChildElement("size");
        if(NULL != pAlternateFileSize){
            const char *pData = pAlternateFileSize->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stAlterNatePath.size, pData);
            }
        }
        TiXmlElement *pAlternateFileType = pMapFileInfo->FirstChildElement("type");
        if(NULL != pAlternateFileType){
            const char* pData = pAlternateFileType->GetText();
            if( NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stAlterNatePath.type, pData);
            }
        }
        TiXmlElement *pPresentFilepath = pMapFileInfo->FirstChildElement("PresentFilePath");
        if(NULL != pPresentFilepath){
            const char *pData = pPresentFilepath->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stPresentPath.filePath, pData);
            }
        }
        TiXmlElement *pPresentFileSize = pMapFileInfo->FirstChildElement("size");
        if(NULL != pPresentFileSize){
            const char *pData = pPresentFileSize->GetText();
            if(NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stPresentPath.size, pData);
            }
        }
        TiXmlElement *pPresentFileType = pMapFileInfo->FirstChildElement("type");
        if(NULL != pPresentFileType){
            const char* pData = pPresentFileType->GetText();
            if( NULL != pData){
                strcpy(m_settings_vclient.m_mapFilePathList.stPresentPath.type, pData);
            }
        }
    }
//get bar config
    TiXmlElement* pBarSet = pVClientSettings->FirstChildElement("BarSet");
    if(NULL == pBarSet)
    {
        LOG_ERR("%s", "no BARset info in config file");
    }
    else
    {
        m_settings_vclient.m_rdpBar = UNKNOWN_BAR_STATUS;
        TiXmlElement* pRDPBarStatus = pBarSet->FirstChildElement("RDPBarSet");
        if(NULL != pRDPBarStatus)
        {
            const char* pData = pRDPBarStatus->GetText();
            if(NULL != pData)
                m_settings_vclient.m_rdpBar = (RDP_HAS_BAR)atoi(pData);
        }
        m_settings_vclient.m_fapBar = UNKNOWN_BAR_STATUS;
        TiXmlElement* pFAPBarStatus = pBarSet->FirstChildElement("FAPBarSet");
        if(NULL != pFAPBarStatus)
        {
            const char* pData = pFAPBarStatus->GetText();
            if(NULL != pData)
                m_settings_vclient.m_fapBar = (FAP_HAS_BAR)atoi(pData);
        }
    }
//get update config
    m_settings_vclient.m_updateSetting = UNKNOWN_UPDATING_STATUS;
    TiXmlElement* pUpdateSet = pVClientSettings->FirstChildElement("UpdateType");
    if(NULL != pUpdateSet)
    {
        const char *pData = pUpdateSet->GetText();
        if(NULL != pData)
            m_settings_vclient.m_updateSetting = (UPDATE_SETTINGS)atoi(pData);
    }

//get update config
    m_settings_vclient.iAutoConnectToServer = 0;
    TiXmlElement* pAutoConnectSet = pVClientSettings->FirstChildElement("AutoConnectToServer");
    if(NULL != pAutoConnectSet)
    {
        const char *pData = pAutoConnectSet->GetText();
        if(NULL != pData)
            m_settings_vclient.iAutoConnectToServer = atoi(pData);
    }
//get UserInfo
    TiXmlElement* pUserInfo = pVClientSettings->FirstChildElement("UserInfo");
    if(NULL == pUserInfo)
    {
        LOG_ERR("%s", "no USERINFO info in config file");
        memset(&m_settings_login, 0, sizeof(SETTINGS_LOGIN));
    }
    else
    {
        m_settings_login.stUserInfo.username[0] = '\0';
        TiXmlElement* pUserName = pUserInfo->FirstChildElement("username");
        if(NULL != pUserName)
        {
            const char *pData = pUserName->GetText();
            if(NULL != pData)
                strcpy(m_settings_login.stUserInfo.username, pData);
        }
        m_settings_login.stUserInfo.password[0] = '\0';
        TiXmlElement* pPasswd = pUserInfo->FirstChildElement("password");
        if(NULL != pPasswd)
        {
            const char *pData = pPasswd->GetText();
            if(NULL != pData)
                strcpy(m_settings_login.stUserInfo.password, pData);
        }
        m_settings_login.stUserInfo.uuid[0] ='\0';
        TiXmlElement* pUuid = pUserInfo->FirstChildElement("uuid");
        if(NULL != pUuid){
            const char* pData = pUuid->GetText();
            if( NULL != pData){
                strcpy(m_settings_login.stUserInfo.uuid, pData);
            }
        }
        m_settings_login.stUserInfo.domain[0] = '\0';
        TiXmlElement* pDomain = pUserInfo->FirstChildElement("domainCurrentUsed");
        if(NULL != pDomain)
        {
            const char *pData = pDomain->GetText();
            if(NULL != pData)
                strcpy(m_settings_login.stUserInfo.domain, pData);
        }
        m_settings_login.iAutoLogin = 0;
        TiXmlElement* pAutoLogin = pUserInfo->FirstChildElement("autologin");
        if(NULL != pAutoLogin)
        {
            const char *pData = pAutoLogin->GetText();
            if(NULL != pData)
                m_settings_login.iAutoLogin = atoi(pData);
        }
        m_settings_login.iRemember = 0;
        TiXmlElement* pRemeber = pUserInfo->FirstChildElement("remeber");
        if(NULL != pRemeber)
        {
            const char *pData = pRemeber->GetText();
            if(NULL != pData)
                m_settings_login.iRemember = atoi(pData);
        }
        m_settings_login.iAttachVDisk = 0;
        TiXmlElement* pAttachVDisk = pUserInfo->FirstChildElement("attachDisk");
        if(NULL != pAttachVDisk)
        {
            const char *pData = pAttachVDisk->GetText();
            if(NULL != pData)
                m_settings_login.iAttachVDisk = atoi(pData);
        }
    }
 // Get default setting
    TiXmlElement* pDefaultApp = pVClientSettings->FirstChildElement("DefaultApp");
    if(NULL == pDefaultApp)
    {
        LOG_ERR("%s", "no USERINFO info in config file");// m_settings_defaultApp.isLoadvDisk
        m_settings_defaultApp.isMapUsb = m_settings_defaultApp.connectProtocal = m_settings_defaultApp.isAutoConnect = 0;

    }
    else
    {
        TiXmlElement* pUuid = pDefaultApp->FirstChildElement("appUuid");
        if(NULL != pUuid)
        {
            const char *pData = pUuid->GetText();
            if(NULL != pData)
                strcpy(m_settings_defaultApp.uuid, pData);
        }
        TiXmlElement* pAppName = pDefaultApp->FirstChildElement("appName");
        if(NULL != pAppName)
        {
            const char *pData = pAppName->GetText();
            if(NULL != pData)
                strcpy(m_settings_defaultApp.appName ,pData);
        }
        TiXmlElement* pUserName = pDefaultApp->FirstChildElement("userName");
        if(NULL != pUserName)
        {
            const char *pData = pUserName->GetText();
            if(NULL != pData)
                strcpy(m_settings_defaultApp.userName, pData);
        }
        TiXmlElement* pServerIp = pDefaultApp->FirstChildElement("serverIp");
        if(NULL != pServerIp)
        {
            const char *pData = pServerIp->GetText();
            if(NULL != pData)
                strcpy(m_settings_defaultApp.serverIp, pData);
        }
        m_settings_defaultApp.isAutoConnect= 0;
        TiXmlElement* pAutoLogin = pDefaultApp->FirstChildElement("isAutoConnect");
        if(NULL != pAutoLogin)
        {
            const char *pData = pAutoLogin->GetText();
            if(NULL != pData)
                m_settings_defaultApp.isAutoConnect = atoi(pData);
        }
        TiXmlElement* pcp = pDefaultApp->FirstChildElement("connectProtocal");
        if(NULL != pcp)
        {
            const char *pData = pcp->GetText();
            if(NULL != pData)
                m_settings_defaultApp.connectProtocal = atoi(pData);
        }
        /*TiXmlElement* pattachDisk = pDefaultApp->FirstChildElement("attachDisk");
        if(NULL != pattachDisk)
        {
            const char *pData = pServerIp->GetText();
            if(NULL != pData)
                m_settings_defaultApp.isLoadvDisk = atoi(pData);
        }*/
        TiXmlElement* pMapUsb = pDefaultApp->FirstChildElement("mapUsb");
        if(NULL != pMapUsb)
        {
            const char *pData = pMapUsb->GetText();
            if(NULL != pData)
                m_settings_defaultApp.isMapUsb = atoi(pData);
        }
    }

    return 0;
}
