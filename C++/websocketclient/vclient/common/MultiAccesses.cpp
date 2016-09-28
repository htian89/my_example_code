// Need to add error checking for calloc
#include "MultiAccesses.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <QDebug>
extern CConfigInfo* g_pConfigInfo;


MultiAccesses::MultiAccesses()
{
    m_ifcfg = NULL;
    memset(&m_defaultAccessStruct, 0, sizeof( AccessStruct ));
    initLocalNetInfo();

    strcpy(fileName, getenv("HOME"));
    strcat(fileName, "/.vclient/");
    strcat(fileName, "MultiAccesses.conf");
    qDebug() << "MultiAcesses configuration file path: " << fileName;
    LOG_INFO("MultiAcesses configuration file path:%s",fileName);

    m_AccessStructStackHead = NULL;
    AccessStructStackSize = 0;

}

void MultiAccesses::initLocalNetInfo()
{
    m_ifcfg = new rh_ifcfg;
    SETTINGS_VCLIENT stVclientSetting;
    memset(&stVclientSetting, 0, sizeof(SETTINGS_VCLIENT));
    if(g_pConfigInfo != NULL){
        g_pConfigInfo->getSettings_vclient(stVclientSetting);
        if(strlen(stVclientSetting.m_network.stPresentServer.serverAddress) > 0){
            strcpy(m_defaultAccessStruct.AccessIp, stVclientSetting.m_network.stPresentServer.serverAddress);
        }else if(strlen(stVclientSetting.m_network.stFirstServer.serverAddress) > 0){
            strcpy(m_defaultAccessStruct.AccessIp, stVclientSetting.m_network.stFirstServer.serverAddress);
        }
    }else{
        LOG_ERR("g_pConfigInfo failed");
    }
    QString tmp;

    tmp.clear();
    if(m_ifcfg->get_value("BOOTPROTO", &tmp)){
        if(tmp.length() >0 && 0 == tmp.compare("dhcp")){
            strcpy(m_defaultAccessStruct.ip,m_ifcfg->getIpInfo().ip);
            strcpy(m_defaultAccessStruct.netmask,m_ifcfg->getIpInfo().netmask);
            strcpy(m_defaultAccessStruct.gateway,m_ifcfg->getIpInfo().gateway);
            strcpy(m_defaultAccessStruct.dns1,m_ifcfg->getIpInfo().dns1);
        }else{
            if(m_ifcfg->get_value("IPADDR", &tmp)){
                if(tmp.length() > 0){
                    strcpy(m_defaultAccessStruct.ip, tmp.toUtf8().data());
                }else{
                    strcpy(m_defaultAccessStruct.ip,m_ifcfg->getIpInfo().ip);
                }
            }
            tmp.clear();
            if(m_ifcfg->get_value("NETMASK", &tmp)){
                if(tmp.length() > 0){
                    strcpy(m_defaultAccessStruct.netmask, tmp.toUtf8().data());
                }else{
                    strcpy(m_defaultAccessStruct.netmask,m_ifcfg->getIpInfo().netmask);
                }
            }
            tmp.clear();
            if(m_ifcfg->get_value("GATEWAY", &tmp)){
                if(tmp.length() > 0){
                    strcpy(m_defaultAccessStruct.gateway, tmp.toUtf8().data());
                }else{
                    strcpy(m_defaultAccessStruct.gateway,m_ifcfg->getIpInfo().gateway);
                }
            }
            tmp.clear();
            if(m_ifcfg->get_value("DNS1", &tmp)){
                if(tmp.length() > 0){
                    strcpy(m_defaultAccessStruct.dns1, tmp.toUtf8().data());
                }else{
                    strcpy(m_defaultAccessStruct.dns1, m_ifcfg->getIpInfo().dns1);
                }
            }
        }
    }
}

MultiAccesses::~MultiAccesses()
{
    AccessStructNode *pIterator;
    AccessStructNode *p2Del;
    pIterator = m_AccessStructStackHead;
    while (pIterator != NULL) {
        p2Del = pIterator;
        pIterator = pIterator->next;
        AccessStructStackSize--;
        free(p2Del);
    }
    if(m_ifcfg != NULL){
        delete m_ifcfg;
    }
}

void MultiAccesses::set_default(AccessStruct &defaultAccessStruct)
{
    m_defaultAccessStruct = defaultAccessStruct;
}

AccessStruct MultiAccesses::get_default()
{
    return m_defaultAccessStruct;
}

int MultiAccesses::writeFile()
{
    FILE *pOut;
	int iRet = 0;
    if ( (pOut=fopen(fileName, "w")) == NULL) {
        qDebug() << "writeFile error open file";
        LOG_ERR("%s", "writeFile error open file");
        return -1;
    }
    fputc(AccessStructStackSize, pOut);
    qDebug() << "fpuc: " << AccessStructStackSize;
    AccessStructNode *pIterator;
    pIterator = m_AccessStructStackHead;
    while (pIterator != NULL) {
        if (1 == fwrite(&(pIterator->data), sizeof(AccessStruct), 1, pOut)) {
            qDebug() << "write Data ip: " << pIterator->data.ip;
            pIterator = pIterator->next;
        } else {
            qDebug() << "writeFile fwrite error";
            LOG_ERR("%s", "writeFile fwrite error");
            fclose(pOut);
			iRet =  system("sync");
            return -1;
        }
    }

    fclose(pOut);
    iRet = system("sync");
	LOG_INFO(" height 8: %d", iRet/256);
    return 0;
}

int MultiAccesses::readFile()
{
    FILE *pIn;
    int size = 0;

    if ( (pIn =fopen(fileName, "r")) == NULL) {
        qDebug() << "openFile error open file";
        LOG_ERR("%s", "openFile error open file");
        return -1;
    }
    size = fgetc(pIn);
    qDebug() << "fgetc: " << size;

    int readCount = 0;

    AccessStruct data;

    while (readCount < size) {

        if ( 1 == fread(&data, sizeof(AccessStruct), 1, pIn)) {
            // need to deal with corrupt config file, in that case, data.ip isn't a Qstring, program will crash.
            qDebug() << "read Data ip: " << data.ip;
            push(data);
        } else {
            qDebug() << "readFile fread error";
            LOG_ERR("%s", "readFile fread error");
            fclose(pIn);
            return -1;
        }
        readCount++;
    }
    fclose(pIn);
    return 0;
}

bool MultiAccesses::push(AccessStruct AccessStructData)
{
    AccessStructNode *newAccessStructNode;
    newAccessStructNode = (AccessStructNode *)calloc(1, sizeof(AccessStructNode));
    memset(newAccessStructNode, 0, sizeof(AccessStructNode));
    memcpy(&(newAccessStructNode->data), &AccessStructData, sizeof(AccessStruct));
    newAccessStructNode->next = m_AccessStructStackHead;
    m_AccessStructStackHead = newAccessStructNode;
    AccessStructStackSize++;
    qDebug() << "push NodeData ip: " << m_AccessStructStackHead->data.ip;

    return true;
}

bool MultiAccesses::pop()
{
    if (AccessStructStackSize == 0) {
        return false;
    }
    AccessStructNode *AccessStructNodeToDel;
    AccessStructNodeToDel = m_AccessStructStackHead;
    qDebug() << "pop NodeData ip: " << AccessStructNodeToDel->data.ip;
    m_AccessStructStackHead = m_AccessStructStackHead->next;
    free(AccessStructNodeToDel);
    AccessStructStackSize--;

    return true;
}

AccessStruct MultiAccesses::top()
{
    if (size() <= 0) {
        qDebug() << "top error: size() <= 0";
        exit(-1);
    } else {
        qDebug() << "top succeed.";

        return m_AccessStructStackHead->data;
    }
}

int MultiAccesses::size()
{
    return AccessStructStackSize;
}
