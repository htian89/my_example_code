#include "ipcitalc.h"
#include "../backend/cthreadpool.h"
#include "../ui/desktoplistdialog.h"
#include <string.h>
#include <stdio.h>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>

const int PORT_ITALC =  9001;
const char *ipAddress_italc = "127.0.0.1";

CSession *IpcItalc::m_pSession;
IpcItalc* IpcItalc::m_ipcItalc;
bool IpcItalc::m_isConnected;
/*static int XMLFindIntElement(mxml_node_t *node, mxml_node_t *top, const char *name)
{
    mxml_node_t *mnt;
    int value;

    mnt = mxmlFindElement(node, top, name, NULL, NULL, MXML_DESCEND);

    if (mnt != NULL && mnt->child != NULL)
    {
        value = atoi(mnt->child->value.text.string);
        return value;
    }

    return -1;
}

static int XMLFindStrElement(mxml_node_t *node, mxml_node_t *top, const char *name,
                     char *dest, int length)
{
    mxml_node_t *mnt;

    mnt = mxmlFindElement(node, top, name, NULL, NULL, MXML_DESCEND);
    if(mnt == NULL)
        return -1;
    if (dest != NULL && mnt != NULL && mnt->child != NULL)
    {
        cout << "XMLFindStrElement: " << mnt->child->value.text.string << endl;
        strncpy(dest, mnt->child->value.text.string, length);
    }

    return 0;
}

static int XMLFindBase64Element(mxml_node_t *node, mxml_node_t *top, const char *name,
                        char *dest, int length)
{
    mxml_node_t *mnt;

    mnt = mxmlFindElement(node, top, name, NULL, NULL, MXML_DESCEND);

    if (dest != NULL && mnt != NULL && mnt->child != NULL)
        strncpy(dest, base64_decode(mnt->child->value.text.string), length);

    return -1;
}

static int XMLNewElementText(mxml_node_t *node, const char *name, const char *string)
{
    mxml_node_t *mnt;

    mnt = mxmlNewElement(node, name);
    if (string != NULL)
        mxmlNewText(mnt, 0, string);
    else
        mxmlNewText(mnt, 0, "");

    return -1;
}*/

//CSession* IpcItalc::m_pSession;
//LIST_MONITORS_INFO* IpcItalc::m_listMonitorsInfo;
IpcItalc::IpcItalc()
{
    m_pSession = CSession::GetInstance();
    //    connect(this, SIGNAL(on_signal_addTerminalList(std::vector<APP_LIST>)), this, SLOT(sendAddTerminalList(std::vector<APP_LIST>)));
    //    connect(this, SIGNAL(on_signal_deleteTerminalList(std::vector<APP_LIST>)), this, SLOT(sendDeleteTerminalList(std::vector<APP_LIST>)));
    _initialize();
}
IpcItalc::~IpcItalc()
{
    ipcClose();
}

void IpcItalc::_initialize()
{
    m_isConnected = false;
    memset(&m_addr, 0, sizeof(m_addr));
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( m_sockfd <0 )
    {
        LOG_ERR("create ipc client socket error");
        return ;
    }
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(PORT_ITALC);
    m_addr.sin_addr.s_addr = inet_addr(ipAddress_italc);

    if(m_addr.sin_addr.s_addr == INADDR_NONE)
    {
        LOG_ERR("Ipc server ip is wrong");
        return ;
    }
    m_ipcItalc = this;
    pthread_t thread;
    VCLIENT_THREAD_CREATE(thread, NULL, ipcItalcProcessMsg, this);
    THREAD_DETACH(thread);

}
void *IpcItalc::ipcItalcHello(void *arg)
{
    IpcItalc *ipcItalc = static_cast<IpcItalc *>(arg);
    if(ipcItalc == NULL)
    {
        return NULL;
    }
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(ipcItalc->ipcSendMsg("hello" , strlen("hello")) > 0)
    {
        cout << "hello+++++++++++" << endl;
        return NULL;
    }
    else
    {
        LOG_ERR("send message error");
        return NULL;
    }
}

void* IpcItalc::ipcItalcRequestMonitorsInfo(void * arg)
{
    IpcItalc *ipcItalc = static_cast<IpcItalc *>(arg);
    if(ipcItalc == NULL)
    {
        return NULL;
    }
    LIST_MONITORS_INFO listMonitorsInfo;
    while(1)
    {
        if(!m_isConnected || m_ipcItalc==NULL)
        {
            return 0;
        }
        cout << "request monitors info" << endl;
        taskUUID taskUuid = TASK_UUID_NULL;
        CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
        if(NULL == m_pSession)
            m_pSession =CSession::GetInstance();
        if(NULL == pCall_param)
        {
            LOG_ERR("%s", "new failed! NULL == pCall_param");
            return 0;
        }
        pCall_param->pUi = NULL;
        pCall_param->uiType = 0;
        PARAM_SESSION_IN param;
        param.callbackFun = NULL;
        param.callback_param = pCall_param;
        param.isBlock = BLOCKED;
        param.taskUuid = taskUuid;
        memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
        memset(&listMonitorsInfo, 0, sizeof(listMonitorsInfo));
        m_pSession->getMonitorsInfo(param, &listMonitorsInfo);
        if(listMonitorsInfo.errorcode < 0 || listMonitorsInfo.monitorNum <= 0)
        {
            break;
        }else{
            ipcItalc->sendMonitorsInfo(listMonitorsInfo);
            sleep(120);
        }
    }
    return NULL;
}
void* IpcItalc::ipcItalcListUserReource(void *arg)
{
    IpcItalc *ipcItalc = static_cast<IpcItalc *>(arg);
    if(ipcItalc == NULL)
    {
        return NULL;
    }
    LIST_USER_RESOURCE_DATA stListApp;
    std::vector<APP_LIST> m_addlist, m_deletelist, m_listbak, m_listtemp, stAppBakList;
    while(1)
    {
        if(!m_isConnected || m_ipcItalc==NULL)
        {
            return NULL;
        }
        cout << "request listuserReousrce info" << endl;
        taskUUID taskUuid = TASK_UUID_NULL;
        CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
        if(NULL == m_pSession)
            m_pSession =CSession::GetInstance();
        if(NULL == pCall_param)
        {
            LOG_ERR("%s", "new failed! NULL == pCall_param");
            return 0;
        }
        pCall_param->pUi = NULL;
        pCall_param->uiType = 0;
        PARAM_SESSION_IN param;
        param.callbackFun = NULL;
        param.callback_param = pCall_param;
        param.isBlock = BLOCKED;
        param.taskUuid = taskUuid;
        memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
        m_pSession->listUserResource(param, false, &stListApp);
        if(param.callback_param->errorCode < 0)
        {
            sleep(10);
            continue;
        }
        stAppBakList = stListApp.stAppBakList;
        m_addlist.clear();
        m_deletelist.clear();
        m_listbak.clear();
        m_listtemp.clear();
        if(!m_isConnected || m_ipcItalc==NULL || ipcItalc == NULL)
        {
            return NULL;
        }
        for(std::vector<APP_LIST>::size_type k = 0; k < ipcItalc->m_appbaklist.size(); k++)
        {
            if(ipcItalc->m_appbaklist.at(k).desktopType == TERMINAL)
            {
                m_listtemp.push_back(ipcItalc->m_appbaklist.at(k));
            }
        }
        if(!ipcItalc->m_appbaklist.empty())
        {
            ipcItalc->m_appbaklist.clear();
        }
        for(std::vector<APP_LIST>::size_type j = 0; j < stAppBakList.size();  j++)
        {
            ipcItalc->m_appbaklist.push_back(stAppBakList.at(j));
            if(stAppBakList.at(j).desktopType == TERMINAL)
            {
                m_listbak.push_back(stAppBakList.at(j));
            }
        }
        for(std::vector<APP_LIST>::size_type h = 0; h< m_listtemp.size(); h++)
        {
            for(std::vector<APP_LIST>::size_type q = 0; q < m_listbak.size(); q++)
            {
                if(strcmp(m_listtemp.at(h).uuid, m_listbak.at(q).uuid) == 0)
                {
                    break;
                }
                if( q == (m_listbak.size()-1))
                {
                    m_deletelist.push_back(m_listtemp.at(h));
                }
            }
        }
        for(std::vector<APP_LIST>::size_type l = 0; l < m_listbak.size(); l++)
        {
            for(std::vector<APP_LIST>::size_type p = 0; p < m_listtemp.size(); p++)
            {
                if(strcmp(m_listbak.at(l).uuid,m_listtemp.at(p).uuid) == 0)
                    break;
                if(p == (m_listtemp.size()-1))
                {
                    m_addlist.push_back(m_listbak.at(l));
                }
            }
        }
        if(m_addlist.size() > 0)
        {
            //           emit ipcItalc->on_signal_addTerminalList(m_addlist);
            ipcItalc->sendAddTerminalList(m_addlist);
        }
        if(m_deletelist.size() > 0)
        {
            //           emit ipcItalc->on_signal_deleteTerminalList(m_deletelist);
            ipcItalc->sendDeleteTerminalList(m_deletelist);
        }
        sleep(120);
    }
}

void* IpcItalc::ipcItalcProcessMsg(void *arg)
{
    IpcItalc *ipcItalc = static_cast<IpcItalc*>(arg);
    if(ipcItalc == NULL)
    {
        return NULL;
    }
    while(1)
    {
        if(m_ipcItalc==NULL)
        {
            return NULL;
        }
        if(ipcItalc->getSockfd() < 0 )
        {
            return NULL;
        }
        sockaddr_in addr_in = ipcItalc->getSockAddr();
        if( ::connect(ipcItalc->getSockfd(), (sockaddr*)&addr_in, sizeof(sockaddr)) < 0 )
        {
            if(errno == EINTR)
            {
                LOG_ERR("System interupt when connect to ipc server ");
                return NULL;
            }
            sleep(3);
        }
        else
        {
            break;
        }
    }
    ipcItalc->setConnected(true);
    pthread_t thread_hello;
    VCLIENT_THREAD_CREATE(thread_hello, NULL, ipcItalcHello, arg);
    THREAD_DETACH(thread_hello);
    pthread_t thread;
    VCLIENT_THREAD_CREATE(thread, NULL, ipcItalcRequestMonitorsInfo, arg);
    THREAD_DETACH(thread);
    //    pthread_t thread_listApp;
    //    VCLIENT_THREAD_CREATE(thread_listApp, NULL, ipcItalcListUserReource, arg);
    //    THREAD_DETACH(thread_listApp);

    while(1)
    {
        if(!m_isConnected || m_ipcItalc==NULL)
        {
            return NULL;
        }
        char text[20000];
        memset(text, 20000, 0);
        int len = ipcItalc->ipcRecvMsg(text);
        //        QByteArray temp = QByteArray::fromBase64(QString(text).toUtf8().data());
        cout << "receive messgae : " <<text << endl;
        if( len <=0 )
        {
            break;
        }
        cout << "operate the receive message" << endl;

        QDomDocument doc("mydocument");
        QString cheatDoc;
        //        if (!doc.setContent(QString(temp.data()),&cheatDoc,NULL,NULL)) {
        if (!doc.setContent(QString(text),&cheatDoc,NULL,NULL)) {
            cerr << "doc.setContent error" << endl;
            cerr << cheatDoc.toUtf8().data() << endl;
            continue;
        }
        QDomElement docElem = doc.documentElement();
        if( !docElem.isNull())
        {
            if( docElem.tagName() == "ConnectMonitor" )
            {
                CONNECT_MONITOR *connectMonitor = new CONNECT_MONITOR;
                memset(connectMonitor, 0, sizeof(CONNECT_MONITOR));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "Mode"){
                            sscanf(subElement.text().toUtf8().data(), "%d", &(connectMonitor->mode));
                        }else if(subElement.tagName() == "Monitor"){
                            sscanf(subElement.text().toUtf8().data(), "%d", &(connectMonitor->monitor));
                        }else if(subElement.tagName() == "ToMonitorIp"){
                            strcpy(connectMonitor->toMonitorIp, subElement.text().toUtf8().constData());
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_connect;
                VCLIENT_THREAD_CREATE(thread_connect, NULL, on_slot_connect_monitor,connectMonitor);
                THREAD_DETACH(thread_connect);
            }
            if(docElem.tagName() == "DisconnectMonitor" )
            {
                DISCONNECT_MONITOR *disconnectMonitor = new DISCONNECT_MONITOR;
                memset(disconnectMonitor, 0, sizeof(DISCONNECT_MONITOR));

                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "Monitor"){
                            sscanf(subElement.text().toUtf8().data(), "%d", &(disconnectMonitor->monitor));
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_disconnect;
                VCLIENT_THREAD_CREATE(thread_disconnect, NULL, on_slot_disconnect_monitor,disconnectMonitor);
                THREAD_DETACH(thread_disconnect);
            }
            if(docElem.tagName() == "GetUserOrganizations" )
            {
                pthread_t thread_getUserOrganizations;
                VCLIENT_THREAD_CREATE(thread_getUserOrganizations, NULL, getuserOrganizations, arg);
                THREAD_DETACH(thread_getUserOrganizations);
            }
            if(docElem.tagName() == "AddOrganization" )
            {
                ADD_ORGANIZATION *add_organization = new ADD_ORGANIZATION;
                memset(add_organization, 0, sizeof(ADD_ORGANIZATION));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "ParentUniqueName"){
                            strcpy(add_organization->parentUniqueName ,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Name"){
                            strcpy(add_organization->name,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Description"){
                            strcpy(add_organization->description, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_addOrganization;
                VCLIENT_THREAD_CREATE(thread_addOrganization, NULL, addOrganization, add_organization);
                THREAD_DETACH(thread_addOrganization);
            }
            if( docElem.tagName() == "DeleteOrganization" )
            {
                DELETE_ORGANIZATION *delete_organization = new DELETE_ORGANIZATION;
                memset(delete_organization, 0, sizeof(DELETE_ORGANIZATION));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UniqueName"){
                            strcpy(delete_organization->uniqueName ,subElement.text().toUtf8().constData());
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_deleteOrganization;
                VCLIENT_THREAD_CREATE(thread_deleteOrganization, NULL, deleteOrganization, delete_organization);
                THREAD_DETACH(thread_deleteOrganization);
            }
            if( docElem.tagName() == "UpdateOrganization" )
            {
                UPDATE_ORGANIZATION *update_organization = new UPDATE_ORGANIZATION;
                memset(update_organization, 0, sizeof(UPDATE_ORGANIZATION));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UniqueName"){
                            strcpy(update_organization->uniqueName ,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "ParentUniqueName"){
                            strcpy(update_organization->parentUniqueName,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Name"){
                            strcpy(update_organization->name, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Descreption"){
                            strcpy(update_organization->description, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_updateOrganization;
                VCLIENT_THREAD_CREATE(thread_updateOrganization, NULL, updateOrganization, update_organization);
                THREAD_DETACH(thread_updateOrganization);
            }
            if( docElem.tagName() == "MoveOrganizationUsers" )
            {
                MOVE_ORGANIZATION_USERS *move_organizationUsers = new MOVE_ORGANIZATION_USERS;
                memset(move_organizationUsers, 0, sizeof(MOVE_ORGANIZATION_USERS ));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "DeleteOld"){
                            strcpy(move_organizationUsers->deleteOld ,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "NewUniquename"){
                            strcpy(move_organizationUsers->newUniqueName,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "OldUniqueName"){
                            strcpy(move_organizationUsers->oldUniqueName, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Users"){
                            for (QDomNode Users = subElement.firstChild();  !Users.isNull();  Users=Users.nextSibling()){
                                Userinfo user;
                                memset(&user, 0, sizeof(Userinfo));
                                QDomElement attribute_e = Users.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "UserName") {
                                        strcpy(user.username , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Domain") {
                                        strcpy(user.domain , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Ip") {
                                        strcpy(user.ip , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                move_organizationUsers->users.push_back(user);
                            }
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_moveorganizationUsers;
                VCLIENT_THREAD_CREATE(thread_moveorganizationUsers, NULL, moveOrganizationUsers, move_organizationUsers);
                THREAD_DETACH(thread_moveorganizationUsers);
            }
            if( docElem.tagName() == "AddOrganizationUsers" )
            {
                ADD_ORGANIZATION_USERS *add_organizationUsers = new ADD_ORGANIZATION_USERS;
                memset(add_organizationUsers, 0, sizeof(ADD_ORGANIZATION_USERS));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UniqueName"){
                            strcpy(add_organizationUsers->uniqueName,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Users"){
                            for (QDomNode Users = subElement.firstChild();  !Users.isNull();  Users=Users.nextSibling()){
                                Userinfo user;
                                memset(&user, 0, sizeof(Userinfo));
                                QDomElement attribute_e = Users.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "UserName") {
                                        strcpy(user.username , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Domain") {
                                        strcpy(user.domain , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Ip") {
                                        strcpy(user.ip , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                add_organizationUsers->users.push_back(user);
                            }
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_addorganizationUsers;
                VCLIENT_THREAD_CREATE(thread_addorganizationUsers, NULL, addOrganizationUsers, add_organizationUsers);
                THREAD_DETACH(thread_addorganizationUsers);
            }
            if( docElem.tagName() == "DeleteOrganizationUsers" )
            {
                DELETE_ORGANIZATION_USERS *delete_organizationUsers = new DELETE_ORGANIZATION_USERS;
                memset(delete_organizationUsers, 0, sizeof(DELETE_ORGANIZATION_USERS));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UniqueName"){
                            strcpy(delete_organizationUsers->uniqueName,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "DeleteOld"){
                            strcpy(delete_organizationUsers->deleteOld,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Users"){
                            for (QDomNode Users = subElement.firstChild();  !Users.isNull();  Users=Users.nextSibling()){
                                Userinfo user;
                                memset(&user, 0, sizeof(Userinfo));
                                QDomElement attribute_e = Users.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "UserName") {
                                        strcpy(user.username , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Domain") {
                                        strcpy(user.domain , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Ip") {
                                        strcpy(user.ip , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                delete_organizationUsers->users.push_back(user);
                            }
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_deleteorganizationUsers;
                VCLIENT_THREAD_CREATE(thread_deleteorganizationUsers, NULL, deleteOrganizationUsers, delete_organizationUsers);
                THREAD_DETACH(thread_deleteorganizationUsers);
            }
            if( docElem.tagName() == "GetOrganizationDetail" )
            {
                GET_ORGANIZATION_DETAIL *organization_detail = new GET_ORGANIZATION_DETAIL;
                memset(organization_detail, 0, sizeof(GET_ORGANIZATION_DETAIL));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UniqueName"){
                            strcpy(organization_detail->uniqueName ,subElement.text().toUtf8().constData());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t   thread_getorganizationdetail;
                VCLIENT_THREAD_CREATE(thread_getorganizationdetail, NULL, getOrganizationDetail, organization_detail);
                THREAD_DETACH(thread_getorganizationdetail);
            }
            if(  docElem.tagName() == "GetRoles" )
            {
                pthread_t thread_getroles;
                VCLIENT_THREAD_CREATE(thread_getroles, NULL, getRoles, arg);
                THREAD_DETACH(thread_getroles);
            }
            if(  docElem.tagName() == "AddRole" )
            {
                ADD_ROLE *addrole = new ADD_ROLE;
                memset(addrole, 0, sizeof(ADD_ROLE));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "RoleName"){
                            strcpy(addrole->roleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Weight"){
                            strcpy(addrole->weight,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Descreption"){
                            strcpy(addrole->description, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_addrole;
                VCLIENT_THREAD_CREATE(thread_addrole, NULL, addRole, addrole);
                THREAD_DETACH(thread_addrole);
            }
            if( docElem.tagName() == "DeleteRole" )
            {
                DELETE_ROLE *deleterole = new DELETE_ROLE;
                memset(deleterole, 0, sizeof(DELETE_ROLE));

                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "RoleName"){
                            strcpy(deleterole->roleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }
                    }
                    node = node.nextSibling();
                }
                pthread_t thread_deleterole;
                VCLIENT_THREAD_CREATE(thread_deleterole, NULL, deleteRole, deleterole);
                THREAD_DETACH(thread_deleterole);
            }
            if(  docElem.tagName() == "UpdateRole" )
            {
                UPDATE_ROLE *updaterole = new UPDATE_ROLE;
                memset(updaterole, 0, sizeof(UPDATE_ROLE));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "RoleName"){
                            strcpy(updaterole->roleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "NewRoleName"){
                            strcpy(updaterole->newRoleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Weight"){
                            strcpy(updaterole->weight,subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Descreption"){
                            strcpy(updaterole->description, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_updaterole;
                VCLIENT_THREAD_CREATE(thread_updaterole, NULL, updateRole, updaterole);
                THREAD_DETACH(thread_updaterole);
            }
            if( docElem.tagName() == "AddRoleToUsers" )
            {
                ADDROLE_TOUSERS *addrole_tousers = new ADDROLE_TOUSERS;
                memset(addrole_tousers, 0, sizeof(ADDROLE_TOUSERS));

                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "RoleName"){
                            strcpy(addrole_tousers->roleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Users"){
                            for (QDomNode Users = subElement.firstChild();  !Users.isNull();  Users=Users.nextSibling()){
                                Userinfo user;
                                memset(&user, 0, sizeof(Userinfo));
                                QDomElement attribute_e = Users.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "UserName") {
                                        strcpy(user.username , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Domain") {
                                        strcpy(user.domain , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Ip") {
                                        strcpy(user.ip , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                addrole_tousers->users.push_back(user);
                            }
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_addrole_tousers;
                VCLIENT_THREAD_CREATE(thread_addrole_tousers, NULL, addroleTousers, addrole_tousers);
                THREAD_DETACH(thread_addrole_tousers);
            }
            if( docElem.tagName() == "DeleteRoleFromUsers" )
            {
                DELETEROLE_FROMUSERS *deleterole_fromusers = new DELETEROLE_FROMUSERS;
                memset(deleterole_fromusers, 0, sizeof(DELETEROLE_FROMUSERS));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "RoleName"){
                            strcpy(deleterole_fromusers->roleName ,QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Users"){
                            for (QDomNode Users = subElement.firstChild();  !Users.isNull();  Users=Users.nextSibling()){
                                Userinfo user;
                                memset(&user, 0, sizeof(Userinfo));
                                QDomElement attribute_e = Users.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "UserName") {
                                        strcpy(user.username , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Domain") {
                                        strcpy(user.domain , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()).data());
                                    } else if (attr.toElement().tagName() == "Ip") {
                                        strcpy(user.ip , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                deleterole_fromusers->users.push_back(user);
                            }
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_deleterole_fromusers;
                VCLIENT_THREAD_CREATE(thread_deleterole_fromusers, NULL, deleteroleFromUsers, deleterole_fromusers);
                THREAD_DETACH(thread_deleterole_fromusers);
            }
            if(  docElem.tagName() == "GetUserPrivileges" )
            {
                GET_USERPRIVILEGES_PARAM * get_userprivileges_param = new GET_USERPRIVILEGES_PARAM;
                memset(get_userprivileges_param, 0, sizeof(GET_USERPRIVILEGES_PARAM));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "UserName"){
                            strcpy(get_userprivileges_param->userName, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Domain"){
                            strcpy(get_userprivileges_param->domain, QByteArray::fromBase64(subElement.text().toUtf8().constData()).data());
                        }else if(subElement.tagName() == "Target"){
                            strcpy(get_userprivileges_param->target, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "TargetId"){
                            strcpy(get_userprivileges_param->targetId, subElement.text().toUtf8().constData());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_getuserprivileges;
                VCLIENT_THREAD_CREATE(thread_getuserprivileges, NULL, getuserprivileges, get_userprivileges_param);
                THREAD_DETACH(thread_getuserprivileges);
            }
            if(  docElem.tagName() == "GetPrivileges" )
            {
                GET_PRIVILEGES_PARAM *get_privileges_param = new GET_PRIVILEGES_PARAM;
                memset(get_privileges_param, 0, sizeof(GET_PRIVILEGES_PARAM));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "Owner"){
                            strcpy(get_privileges_param->owner, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "OwnerId"){
                            strcpy(get_privileges_param->ownerId, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Target"){
                            strcpy(get_privileges_param->target, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "TargetId"){
                            strcpy(get_privileges_param->targetId, subElement.text().toUtf8().constData());
                        }
                    }
                    node = node.nextSibling();
                }

                pthread_t thread_getprivileges;
                VCLIENT_THREAD_CREATE(thread_getprivileges, NULL, getprivileges, get_privileges_param);
                THREAD_DETACH(thread_getprivileges);
            }
            if( docElem.tagName() == "AddPrivileges" )
            {
                ADD_PRIVILEGES *addprivileges = new ADD_PRIVILEGES;
                memset(addprivileges, 0, sizeof(ADD_PRIVILEGES));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "Owner"){
                            strcpy(addprivileges->owner, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "OwnerId"){
                            strcpy(addprivileges->ownerId, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Target"){
                            strcpy(addprivileges->target, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "TargetId"){
                            strcpy(addprivileges->targetId, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Privileges"){
                            for (QDomNode Privileges = subElement.firstChild();  !Privileges.isNull();  Privileges=Privileges.nextSibling()){
                                USERPRIVILEGESDATA privilege;
                                memset(&privilege, 0, sizeof(USERPRIVILEGESDATA));
                                QDomElement attribute_e = Privileges.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "Action") {
                                        strcpy(privilege.action , attr.toElement().text().toUtf8().constData());
                                    } else if (attr.toElement().tagName() == "Transmission") {
                                        strcpy(privilege.transmission , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                addprivileges->privileges.push_back(privilege);
                            }
                        }
                        node = node.nextSibling();
                    }
                }
                pthread_t thread_addprivileges;
                VCLIENT_THREAD_CREATE(thread_addprivileges, NULL, addPrivileges, addprivileges);
                THREAD_DETACH(thread_addprivileges);
            }
            if( docElem.tagName() == "DeletePrivileges" )
            {
                DELETE_PRIVILEGES *deleteprivileges = new DELETE_PRIVILEGES;
                memset(deleteprivileges, 0, sizeof(DELETE_PRIVILEGES));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "Owner"){
                            strcpy(deleteprivileges->owner, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "OwnerId"){
                            strcpy(deleteprivileges->ownerId, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Target"){
                            strcpy(deleteprivileges->target, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "TargetId"){
                            strcpy(deleteprivileges->targetId, subElement.text().toUtf8().constData());
                        }else if(subElement.tagName() == "Privileges"){
                            for (QDomNode Privileges = subElement.firstChild();  !Privileges.isNull();  Privileges=Privileges.nextSibling()){
                                USERPRIVILEGESDATA privilege;
                                memset(&privilege, 0, sizeof(USERPRIVILEGESDATA));
                                QDomElement attribute_e = Privileges.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "Action") {
                                        strcpy(privilege.action , attr.toElement().text().toUtf8().constData());
                                    }
                                }
                                deleteprivileges->privileges.push_back(privilege);
                            }
                        }
                        node = node.nextSibling();
                    }
                }
                pthread_t thread_deleteprivileges;
                VCLIENT_THREAD_CREATE(thread_deleteprivileges, NULL, deletePrivileges, deleteprivileges);
                THREAD_DETACH(thread_deleteprivileges);
            }
            if(docElem.tagName() == "SetSeatNumber")
            {
                SEATNUMBERS *seatnumbers = new SEATNUMBERS;
                memset(seatnumbers, 0, sizeof(SEATNUMBERS));
                QDomNode node = docElem.firstChild();
                while(!node.isNull())
                {
                    QDomElement subElement = node.toElement();
                    if(!subElement.isNull())
                    {
                        if(subElement.tagName() == "SeatNumbers"){
                            for (QDomNode SeatList = subElement.firstChild();  !SeatList.isNull();  SeatList =SeatList.nextSibling()){
                                SEATNUMBER seatnumber;
                                memset(&seatnumber, 0, sizeof(SEATNUMBER));
                                QDomElement attribute_e = SeatList.toElement();
                                for (QDomNode attr = attribute_e.firstChild(); !attr.isNull(); attr = attr.nextSibling()) {
                                    if (attr.toElement().tagName() == "DesktopName") {
                                        strcpy(seatnumber.desktopname , QByteArray::fromBase64(attr.toElement().text().toUtf8().constData()));
                                    }else if(attr.toElement().tagName() == "Number"){
                                        strcpy(seatnumber.number, attr.toElement().text().toUtf8().data());
                                    }
                                }
                                seatnumbers->seatlist.push_back(seatnumber);
                            }
                        }
                        node = node.nextSibling();
                    }
                }
                pthread_t thread_setSeatNumbers;
                VCLIENT_THREAD_CREATE(thread_setSeatNumbers, NULL, setSeatNumbers, seatnumbers);
                THREAD_DETACH(thread_setSeatNumbers);
            }
        }
    }
    ipcItalc->setConnected(false);
    return NULL;

}

int IpcItalc::monitorinfoChange()
{
    LIST_MONITORS_INFO listMonitorsInfo;
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return 0;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    memset(&listMonitorsInfo, 0, sizeof(listMonitorsInfo));
    m_pSession->getMonitorsInfo(param, &listMonitorsInfo);
    //     if(listMonitorsInfo)
    //         return -1;
    return sendMonitorsInfo(listMonitorsInfo);
}

void* IpcItalc::on_slot_connect_monitor(void* arg)
{
    CONNECT_MONITOR *connectMonitor = static_cast<CONNECT_MONITOR *>(arg);
    if(connectMonitor == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->connectMonitor(param,connectMonitor);


    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("connectMonitor");
    QDomElement ConnectMonitorElement = doc.createElement("ConnectMonitor");
    doc.appendChild(ConnectMonitorElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    ConnectMonitorElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().data());


    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("SendConnectMonitor: %s", str);
        m_ipcItalc->monitorinfoChange();
    }
    else
    {
        LOG_ERR("SendConnectMonitor fail");
    }
    return NULL;
}
void* IpcItalc::on_slot_disconnect_monitor(void* arg)
{
    DISCONNECT_MONITOR *disconnectMonitor = static_cast<DISCONNECT_MONITOR *> (arg);
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->disconnectMonitor(param, disconnectMonitor);


    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("disconnectMonitor");
    QDomElement disconnectMonitorElement = doc.createElement("DisConnectMonitor");
    doc.appendChild(disconnectMonitorElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    disconnectMonitorElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().data());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("SendDisconnectMonitor: %s", str);
        m_ipcItalc->monitorinfoChange();
    }
    else
    {
        LOG_ERR("SendDisconnectMonitor fail");
    }
    return NULL;
}

int IpcItalc::sendMonitorsInfo(LIST_MONITORS_INFO &pListMonitorsInfo)
{
    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("monitorsInfoList");
    QDomElement MonitorsInfoListElement = doc.createElement("MonitorsInfoList");
    doc.appendChild(MonitorsInfoListElement);
    char s_monitorNum[5] = {0};
    char s_errorcode [5] = {0};
    sprintf(s_monitorNum, "%d", (pListMonitorsInfo.monitorNum));
    sprintf(s_errorcode, "%d", pListMonitorsInfo.errorcode);
    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    MonitorsInfoListElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement MonitorNumElement = doc.createElement("MonitorNum");
    MonitorsInfoListElement.appendChild(MonitorNumElement);
    QDomText MonitorNumText = doc.createTextNode(s_monitorNum);
    MonitorNumElement.appendChild(MonitorNumText);

    if(pListMonitorsInfo.monitorNum > 0)
    {
        QDomElement MonitorsInfoElement = doc.createElement("MonitorsInfo");
        MonitorsInfoListElement.appendChild(MonitorsInfoElement);
        for(unsigned int loop = 0; loop < pListMonitorsInfo.stMonitorInfoList.size(); loop++)
        {
            QDomElement MonitorInfoElement = doc.createElement("MonitorInfo");
            MonitorsInfoElement.appendChild(MonitorInfoElement);
            char s_monitor[5] = {0};
            char s_status[5]= {0};
            char s_mode[5]= {0};
            sprintf(s_monitor, "%d", pListMonitorsInfo.stMonitorInfoList.at(loop).monitor);
            sprintf(s_status, "%d", pListMonitorsInfo.stMonitorInfoList.at(loop).status);
            sprintf(s_mode, "%d", pListMonitorsInfo.stMonitorInfoList.at(loop).mode);

            QDomElement MonitorElement = doc.createElement("Monitor");
            MonitorInfoElement.appendChild(MonitorElement);
            QDomText MonitorText = doc.createTextNode(s_monitor);
            MonitorElement.appendChild(MonitorText);

            QDomElement StatusElement = doc.createElement("Status");
            MonitorInfoElement.appendChild(StatusElement);
            QDomText StatusText = doc.createTextNode(s_status);
            StatusElement.appendChild(StatusText);

            QDomElement ModeElement = doc.createElement("Mode");
            MonitorInfoElement.appendChild(ModeElement);
            QDomText ModeText = doc.createTextNode(s_mode);
            ModeElement.appendChild(ModeText);

            QDomElement UsernameElement = doc.createElement("Username");
            MonitorInfoElement.appendChild(UsernameElement);
            QByteArray temp(pListMonitorsInfo.stMonitorInfoList.at(loop).username);
            QDomText UsernameText = doc.createTextNode(temp.toBase64().data());
            UsernameElement.appendChild(UsernameText);

            QDomElement DomainElement = doc.createElement("Domain");
            MonitorInfoElement.appendChild(DomainElement);
            QByteArray temp1(pListMonitorsInfo.stMonitorInfoList.at(loop).domain);
            QDomText DomainText = doc.createTextNode(temp1.toBase64().data());
            DomainElement.appendChild(DomainText);
        }
    }
    strcpy(str, doc.toString().toUtf8().data());

    //    if( strcmp(m_strMonitorsInfo.c_str(), str) == 0)
    //    {
    //        return;
    //    }
    //     m_strMonitorsInfo.clear();
    //     m_strMonitorsInfo.copy(str, strlen(str), 0);
    if(m_ipcItalc == NULL || !m_isConnected)
        return -1;
    if(ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("sendMonitorInfo: %s", str);
        cout << "sendMonitorInfo    :" <<  str <<endl;
    }
    else
    {
        LOG_ERR("send message error");
        return -1;
    }
    return 0;
}

void IpcItalc::sendAddTerminalList(std::vector<APP_LIST> terminalList )
{
    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("addTerminalsList");
    QDomElement domElement = doc.createElement("AddTerminalsList");
    doc.appendChild(domElement);
    for(unsigned int loop = 0; loop < terminalList.size(); loop++)
    {
        char s_state[4];
        sprintf(s_state, "%d", terminalList.at(loop).state);
        QDomElement nodeElement = doc.createElement("Terminal");
        domElement.appendChild(nodeElement);

        QDomElement IpElement = doc.createElement("Ip");
        nodeElement.appendChild(IpElement);
        QDomText IpText = doc.createTextNode( terminalList.at(loop).TerminalIp);
        IpElement.appendChild(IpText);

        QDomElement UuidElement = doc.createElement("Uuid");
        nodeElement.appendChild(UuidElement);
        QDomText UuidText = doc.createTextNode(terminalList.at(loop).uuid);
        UuidElement.appendChild(UuidText);

        QDomElement NameElement = doc.createElement("Name");
        nodeElement.appendChild(NameElement);
        QByteArray temp(terminalList.at(loop).TerminalName);
        QDomText NameText = doc.createTextNode( temp.toBase64().data());
        NameElement.appendChild(NameText);

        QDomElement StateElement = doc.createElement("State");
        nodeElement.appendChild(StateElement);
        QDomText StateText = doc.createTextNode(s_state);
        StateElement.appendChild(StateText);
    }
    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return;
    if(ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("SendAddTerminalList: %s", str);
    }
    else
    {
        LOG_ERR("send addterminallist message error" );
    }

}

void IpcItalc::sendDeleteTerminalList(std::vector<APP_LIST> terminalList)
{
    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("deleteTerminalsList");
    QDomElement domElement = doc.createElement("DeleteTerminalsList");
    doc.appendChild(domElement);
    for(unsigned int loop = 0; loop < terminalList.size(); loop++)
    {
        char s_state[4];
        sprintf(s_state, "%d", terminalList.at(loop).state);
        QDomElement nodeElement = doc.createElement("Terminal");
        domElement.appendChild(nodeElement);

        QDomElement IpElement = doc.createElement("Ip");
        nodeElement.appendChild(IpElement);
        QDomText IpText = doc.createTextNode( terminalList.at(loop).TerminalIp);
        IpElement.appendChild(IpText);

        QDomElement UuidElement = doc.createElement("Uuid");
        nodeElement.appendChild(UuidElement);
        QDomText UuidText = doc.createTextNode(terminalList.at(loop).uuid);
        UuidElement.appendChild(UuidText);

        QDomElement NameElement = doc.createElement("Name");
        nodeElement.appendChild(NameElement);
        QByteArray temp(terminalList.at(loop).TerminalName);
        QDomText NameText = doc.createTextNode( temp.toBase64().data());
        NameElement.appendChild(NameText);

        QDomElement StateElement = doc.createElement("State");
        nodeElement.appendChild(StateElement);
        QDomText StateText = doc.createTextNode(s_state);
        StateElement.appendChild(StateText);
    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return;
    if(ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("SendDeleteTerminalList: %s", str);
    }
    else
    {
        LOG_ERR("send deleteterminallist message error");
    }
}

void* IpcItalc:: getuserOrganizations(void *arg)
{
    IpcItalc *ipcItalc = static_cast<IpcItalc *>(arg);
    if(ipcItalc == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    USERORGANIZATIONS userOrganizations;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->getUserOrganizations(param, &userOrganizations);
    ipcItalc->sendUserOrganizations(userOrganizations);
    return NULL;
}
int IpcItalc::sendUserOrganizations(USERORGANIZATIONS &userorganizations)
{
    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("getUserOrganizations");
    QDomElement domElement = doc.createElement("GetUserOrganizations");
    doc.appendChild(domElement);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", userorganizations.errorcode);
    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement UserNameElement = doc.createElement("UserName");
    domElement.appendChild(UserNameElement);
    QByteArray temp0(userorganizations.username);
    QDomText UserNameText = doc.createTextNode(temp0.toBase64().data());
    UserNameElement.appendChild(UserNameText);

    QDomElement OrganizationsElement = doc.createElement("Organizations");
    domElement.appendChild(OrganizationsElement);
    for(unsigned int loop = 0; loop < userorganizations.organizations.size(); loop++)
    {
        QDomElement OrganizationElement = doc.createElement("Organization");
        OrganizationsElement.appendChild(OrganizationElement);

        QDomElement IdElement = doc.createElement("Id");
        OrganizationElement.appendChild(IdElement);
        QDomText IdText = doc.createTextNode(userorganizations.organizations[loop].id);
        IdElement.appendChild(IdText);

        QDomElement NameElement = doc.createElement("Name");
        OrganizationElement.appendChild(NameElement);
        QByteArray temp(userorganizations.organizations[loop].name);
        QDomText NameText = doc.createTextNode(temp.toBase64().data());
        NameElement.appendChild(NameText);


        QDomElement UniqueNameElement = doc.createElement("UniqueName");
        OrganizationElement.appendChild(UniqueNameElement);
        QDomText UniqueNameText = doc.createTextNode(userorganizations.organizations[loop].uniqueName);
        UniqueNameElement.appendChild(UniqueNameText);

        QDomElement DescriptionElement = doc.createElement("Description");
        OrganizationElement.appendChild(DescriptionElement);
        QByteArray temp1(userorganizations.organizations[loop].description);
        QDomText DescriptionText = doc.createTextNode(temp1.toBase64().data());
        DescriptionElement.appendChild(DescriptionText);

    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return -1;
    if(ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("sendUserorganizations: %s", str);
    }
    else
    {
        LOG_ERR("SendUserorganizations fail");
        return -1;
    }
    return 0;
}
void * IpcItalc::addOrganization(void * arg)
{
    ADD_ORGANIZATION *add_organization = static_cast<ADD_ORGANIZATION *>(arg);
    if(add_organization == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    ADD_ORGANIZATION_DATA *add_organization_data = new ADD_ORGANIZATION_DATA;
    memset(add_organization_data, 0, sizeof(ADD_ORGANIZATION_DATA));
    m_pSession->addOrganization(param, add_organization, add_organization_data);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("addOrganization");
    QDomElement domElement = doc.createElement("AddOrganization");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement IdElement = doc.createElement("Id");
    domElement.appendChild(IdElement);
    QDomText IdText  = doc.createTextNode(add_organization_data->id);
    IdElement.appendChild(IdText);

    QDomElement UniqueNameElement = doc.createElement("UniqueName");
    domElement.appendChild(UniqueNameElement);
    QDomText UniqueNameText  = doc.createTextNode(add_organization_data->uniquename);
    UniqueNameElement.appendChild(UniqueNameText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("addorganizations: %s", str);
    }
    else
    {
        LOG_ERR("Sendaddorganizations fail");
    }
    return NULL;
}

void *IpcItalc::deleteOrganization(void *arg)
{
    DELETE_ORGANIZATION *delete_organization = static_cast<DELETE_ORGANIZATION *>(arg);
    if(delete_organization == NULL )
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->deleteOrganization(param, delete_organization);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("deleteOrganization");
    QDomElement domElement = doc.createElement("DeleteOrganization");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("deleteorganizations: %s", str);
    }
    else
    {
        LOG_ERR("SendDeleteorganizations fail");
    }
    return NULL;
}

void *IpcItalc::updateOrganization(void *arg)
{
    UPDATE_ORGANIZATION *update_organization = static_cast<UPDATE_ORGANIZATION *>(arg);
    if(update_organization == NULL )
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->updateOrganization(param, update_organization);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("updateOrganization");
    QDomElement domElement = doc.createElement("UpdateOrganization");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("updateorganizations: %s", str);
    }
    else
    {
        LOG_ERR("SendUpdateorganizations fail");
    }
    return NULL;
}

void *IpcItalc::moveOrganizationUsers(void *arg)
{
    MOVE_ORGANIZATION_USERS *move_organizationsUsers = static_cast<MOVE_ORGANIZATION_USERS *>(arg);
    if(move_organizationsUsers == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->moveOrganizationUsers(param, move_organizationsUsers);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("moveOragnizationUsers");
    QDomElement domElement = doc.createElement("MoveOragnizationUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("moveorganizationusers: %s", str);
    }
    else
    {
        LOG_ERR("Sendmoveorganizationsusers fail");
    }
    return NULL;
}

void *IpcItalc::addOrganizationUsers(void *arg)
{
    ADD_ORGANIZATION_USERS *add_organizationsUsers = static_cast<ADD_ORGANIZATION_USERS *>(arg);
    if(add_organizationsUsers == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->addOrganizationUsers(param, add_organizationsUsers);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("addOragnizationUsers");
    QDomElement domElement = doc.createElement("AddOragnizationUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("addorganizationusers: %s", str);
    }
    else
    {
        LOG_ERR("Sendaddorganizationusers fail");
    }
    return NULL;
}

void *IpcItalc::deleteOrganizationUsers(void *arg)
{
    DELETE_ORGANIZATION_USERS *delete_organizationsUsers = static_cast<DELETE_ORGANIZATION_USERS *>(arg);
    if(delete_organizationsUsers == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->deleteOrganizationUsers(param, delete_organizationsUsers);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("deleteOragnizationUsers");
    QDomElement domElement = doc.createElement("DeleteOragnizationUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("deleteorganizationusers: %s", str);
    }
    else
    {
        LOG_ERR("SendDeleteOrganizationusers fail");
    }
    return NULL;
}

void *IpcItalc::getOrganizationDetail(void *arg)
{
    GET_ORGANIZATION_DETAIL *getorganizationdetail = static_cast<GET_ORGANIZATION_DETAIL *>(arg);
    if(getorganizationdetail == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    ORGANIZATION_DETAIL organizationdetail;
    memset(&organizationdetail, 0, sizeof(ORGANIZATION_DETAIL));
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->getOrganizationDetail(param, getorganizationdetail, &organizationdetail);


    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("getOrganizationDetail");
    QDomElement domElement = doc.createElement("GetOrganizationDetail");
    doc.appendChild(domElement);

    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", organizationdetail.errorcode);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement OrganizationsElement = doc.createElement("Organizations");
    domElement.appendChild(OrganizationsElement);
    for(unsigned int loop = 0; loop < organizationdetail.organizationDetail.size(); loop++)
    {
        QDomElement OrganizationElement = doc.createElement("Organization");
        OrganizationsElement.appendChild(OrganizationElement);

        QDomElement IdElement = doc.createElement("Id");
        OrganizationElement.appendChild(IdElement);
        QDomText IdText = doc.createTextNode(organizationdetail.organizationDetail[loop].id);
        IdElement.appendChild(IdText);

        QDomElement NameElement = doc.createElement("Name");
        OrganizationElement.appendChild(NameElement);
        QByteArray temp(organizationdetail.organizationDetail[loop].name);
        QDomText NameText = doc.createTextNode(temp.toBase64().data());
        NameElement.appendChild(NameText);

        QDomElement UniqueNameElement = doc.createElement("UniqueName");
        OrganizationElement.appendChild(UniqueNameElement);
        QDomText UniqueNameText = doc.createTextNode(organizationdetail.organizationDetail[loop].uniqueName);
        UniqueNameElement.appendChild(UniqueNameText);

        QDomElement DescriptionElement = doc.createElement("Description");
        OrganizationElement.appendChild(DescriptionElement);
        QByteArray temp1(organizationdetail.organizationDetail[loop].description);
        QDomText DescriptionText = doc.createTextNode(temp1.toBase64().data());
        DescriptionElement.appendChild(DescriptionText);

        QDomElement UsersElement = doc.createElement("Users");
        OrganizationElement.appendChild(UsersElement);
        for(unsigned int hook = 0; hook < organizationdetail.organizationDetail[loop].users.size(); hook++)
        {
            QDomElement UserElement = doc.createElement("User");
            UsersElement.appendChild(UserElement);
            QDomElement UserNameElement = doc.createElement("UserName");
            UserElement.appendChild(UserNameElement);
            QByteArray temp(organizationdetail.organizationDetail[loop].users[hook].username);
            QDomText UserNameText = doc.createTextNode( temp.toBase64().data());
            UserNameElement.appendChild(UserNameText);

            QDomElement DomainElement = doc.createElement("Domain");
            UserElement.appendChild(DomainElement);
            QByteArray temp1(organizationdetail.organizationDetail[loop].users[hook].domain);
            QDomText DomainText = doc.createTextNode(temp1.toBase64().data());
            DomainElement.appendChild(DomainText);

            QDomElement IpElement = doc.createElement("Ip");
            UserElement.appendChild(IpElement);

            QDomText IpText = doc.createTextNode(organizationdetail.organizationDetail[loop].users[hook].ip);
            IpElement.appendChild(IpText);

            QDomElement RoleElement = doc.createElement("Role");
            UserElement.appendChild(RoleElement);
            QByteArray temp2(organizationdetail.organizationDetail[loop].users[hook].role);
            QDomText RoleText = doc.createTextNode(temp2.toBase64().data());
            RoleElement.appendChild(RoleText);

            QDomElement SeatNumberElement = doc.createElement("SeatNumber");
            UserElement.appendChild(SeatNumberElement);

            QDomText SeatNumberText = doc.createTextNode(organizationdetail.organizationDetail[loop].users[hook].seatnumber);
            SeatNumberElement.appendChild(SeatNumberText);

        }
    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("sendGetorganizationdetail: %s", str);
    }
    else
    {
        LOG_ERR("SendGet organizationdetail fail");
    }
    return NULL;
}

void *IpcItalc::getRoles(void *arg)
{
    IpcItalc *ipcitalc = static_cast<IpcItalc *>(arg);
    if(ipcitalc == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    GET_ROLES roles;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->getRoles(param, &roles);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    QDomDocument doc("getRoles");
    QDomElement domElement = doc.createElement("GetRoles");
    doc.appendChild(domElement);

    char s_errorcode[10] = {0};
    sprintf(s_errorcode, "%d", roles.errorcode);

    QDomElement ErrorcodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorcodeElement);
    QDomText ErrorcodeText = doc.createTextNode(s_errorcode);
    ErrorcodeElement.appendChild(ErrorcodeText);

    QDomElement RolesElement = doc.createElement("Roles");
    domElement.appendChild(RolesElement);

    for(unsigned int loop = 0; loop < roles.roles.size(); loop++)
    {
        QDomElement RoleElement = doc.createElement("Role");
        RolesElement.appendChild(RoleElement);


        QDomElement IdElement = doc.createElement("Id");
        RoleElement.appendChild(IdElement);
        QDomText IdText = doc.createTextNode( roles.roles[loop].id);
        IdElement.appendChild(IdText);
        QDomElement NameElement = doc.createElement("Name");
        RoleElement.appendChild(NameElement);
        QByteArray temp( roles.roles[loop].name);
        QDomText NameText = doc.createTextNode(temp.toBase64().data());
        NameElement.appendChild(NameText);
        QDomElement WeightElement = doc.createElement("Weight");
        RoleElement.appendChild(WeightElement);
        QDomText WeightText = doc.createTextNode( roles.roles[loop].weight);
        WeightElement.appendChild(WeightText);

        QDomElement DescriptionElement = doc.createElement("Description");
        RoleElement.appendChild(DescriptionElement);
        QByteArray temp1(roles.roles[loop].description);
        QDomText DescriptionText = doc.createTextNode( temp1.toBase64().data());
        DescriptionElement.appendChild(DescriptionText);
    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("sendGetRoles: %s", str);
    }
    else
    {
        LOG_ERR("SendGetRoles fail");
    }
    return NULL;
}

void *IpcItalc::addRole(void * arg)
{
    ADD_ROLE *addrole= static_cast<ADD_ROLE *>(arg);
    if(addrole == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    ADD_ROLE_DATA *addroledata = new ADD_ROLE_DATA;
    memset(addroledata, 0, sizeof(ADD_ROLE_DATA));
    m_pSession->addRole(param, addrole, addroledata);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("addRole");
    QDomElement domElement = doc.createElement("AddRole");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement IdElement = doc.createElement("Id");
    domElement.appendChild(IdElement);
    QDomText IdText  = doc.createTextNode(addroledata->id);
    IdElement.appendChild(IdText);

    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("addrole: %s", str);
    }
    else
    {
        LOG_ERR("Sendaddrole fail");
    }
    return NULL;
}

void *IpcItalc::deleteRole(void *arg)
{
    DELETE_ROLE *deleterole= static_cast<DELETE_ROLE *>(arg);
    if(deleterole == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->deleteRole(param, deleterole);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("deleteRole");
    QDomElement domElement = doc.createElement("DeleteRole");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("deleterole: %s", str);
    }
    else
    {
        LOG_ERR("Senddeleterole fail");
    }
    return NULL;
}

void *IpcItalc::updateRole(void *arg)
{
    UPDATE_ROLE *updaterole= static_cast<UPDATE_ROLE *>(arg);
    if(updaterole == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->updateRole(param, updaterole);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("updateRole");
    QDomElement domElement = doc.createElement("UpdateRole");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("updaterole: %s", str);
    }
    else
    {
        LOG_ERR("Senddeleterole fail");
    }
    return NULL;
}

void *IpcItalc::addroleTousers(void *arg)
{
    ADDROLE_TOUSERS *addrole_tousers= static_cast<ADDROLE_TOUSERS *>(arg);
    if(addrole_tousers == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->addroletousers(param, addrole_tousers);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("addRoleToUsers");
    QDomElement domElement = doc.createElement("AddRoleToUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());

    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("addrole_tousers: %s", str);
    }
    else
    {
        LOG_ERR("Sendaddrolerousers fail");
    }
    return NULL;
}

void *IpcItalc::deleteroleFromUsers(void *arg )
{
    DELETEROLE_FROMUSERS *deleterole_fromusers= static_cast<DELETEROLE_FROMUSERS  *>(arg);
    if(deleterole_fromusers == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->deleterolefromusers(param,deleterole_fromusers);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("deleteRoleFromUsers");
    QDomElement domElement = doc.createElement("DeleteRoleFromUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("deleterole_fromusers: %s", str);
    }
    else
    {
        LOG_ERR("Senddeleterolefromusers fail");
    }
    return NULL;
}

void *IpcItalc::getuserprivileges(void *arg)
{
    GET_USERPRIVILEGES_PARAM *getuserprivileges= static_cast<GET_USERPRIVILEGES_PARAM  *>(arg);
    if(getuserprivileges == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    GET_USERPRIVILEGES_DATA * getuserprivilegesData = new GET_USERPRIVILEGES_DATA;
    memset(getuserprivilegesData, 0, sizeof(GET_USERPRIVILEGES_DATA));
    m_pSession->getuserprivileges(param,getuserprivileges, getuserprivilegesData);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("getUserPrivileges");
    QDomElement domElement = doc.createElement("GetUserPrivileges");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement PrivilegesElement = doc.createElement("Privileges");
    domElement.appendChild(PrivilegesElement);
    for(unsigned int loop = 0; loop < getuserprivilegesData->privileges.size(); loop++)
    {
        QDomElement PrivilegeElement = doc.createElement("Privilege");
        PrivilegesElement.appendChild(PrivilegeElement);

        QDomElement ActionElement = doc.createElement("Action");
        PrivilegeElement.appendChild(ActionElement);
        QDomText ActionText  = doc.createTextNode(getuserprivilegesData->privileges[loop].action);
        ActionElement.appendChild(ActionText);
        QDomElement TransmissionElement = doc.createElement("Transmission");
        PrivilegeElement.appendChild(TransmissionElement);
        QDomText TransmissionText  = doc.createTextNode( getuserprivilegesData->privileges[loop].transmission);
        TransmissionElement.appendChild(TransmissionText);
    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("getuserprivileges: %s", str);
    }
    else
    {
        LOG_ERR("Sendgetuserprivileges fail");
    }
    return NULL;
}

void *IpcItalc::getprivileges(void *arg)
{
    GET_PRIVILEGES_PARAM *privilegesparam = static_cast<GET_PRIVILEGES_PARAM *>(arg);
    if(privilegesparam == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    GET_PRIVILEGES_DATA * getprivilegesData = new GET_PRIVILEGES_DATA;
    memset(getprivilegesData, 0, sizeof(GET_USERPRIVILEGES_DATA));
    m_pSession->getprivileges(param,privilegesparam, getprivilegesData);

    char str[BUFFERLENGTH];
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);
    QDomDocument doc("getPrivileges");
    QDomElement domElement = doc.createElement("GetPrivileges");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    QDomElement PrivilegesElement = doc.createElement("Privileges");
    domElement.appendChild(PrivilegesElement);

    for(unsigned int loop = 0; loop < getprivilegesData->privileges.size(); loop++)
    {

        QDomElement PrivilegeElement = doc.createElement("Privilege");
        PrivilegesElement.appendChild(PrivilegeElement);

        QDomElement ActionElement = doc.createElement("Action");
        PrivilegeElement.appendChild(ActionElement);
        QDomText ActionText  = doc.createTextNode(getprivilegesData->privileges[loop].action);
        ActionElement.appendChild(ActionText);
        QDomElement TransmissionElement = doc.createElement("Transmission");
        PrivilegeElement.appendChild(TransmissionElement);
        QDomText TransmissionText  = doc.createTextNode( getprivilegesData->privileges[loop].transmission);
        TransmissionElement.appendChild(TransmissionText);
    }
    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("getprivileges: %s", str);
    }
    else
    {
        LOG_ERR("Sendgetprivileges fail");
    }
    return NULL;
}

void *IpcItalc::addPrivileges(void *arg)
{
    ADD_PRIVILEGES *addprivileges = static_cast<ADD_PRIVILEGES *>(arg);
    if(NULL == addprivileges)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));

    m_pSession->addprivileges(param,addprivileges);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("addPrivileges");
    QDomElement domElement = doc.createElement("AddPrivileges");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("addprivileges: %s", str);
    }
    else
    {
        LOG_ERR("Sendaddprivileges fail");
    }
    return NULL;
}

void *IpcItalc::deletePrivileges(void *arg)
{
    DELETE_PRIVILEGES *deleteprivileges = static_cast<DELETE_PRIVILEGES *>(arg);
    if(NULL == deleteprivileges)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));

    m_pSession->deleteprivileges(param,deleteprivileges);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("deleteRoleToUsers");
    QDomElement domElement = doc.createElement("DeleteRoleToUsers");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("deleteprivileges: %s", str);
    }
    else
    {
        LOG_ERR("Senddelegeprivileges fail");
    }
    return NULL;
}

void *IpcItalc::setSeatNumbers(void * arg)
{
    SEATNUMBERS *seatList = static_cast<SEATNUMBERS *>(arg);
    if(seatList == NULL)
    {
        return NULL;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return NULL;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    m_pSession->setSeatNumbers(param, seatList);

    char str[BUFFERLENGTH];
    memset(str, 0, BUFFERLENGTH);
    char s_errorcode[20] = {0};
    sprintf(s_errorcode, "%d", param.callback_param->errorCode);

    QDomDocument doc("setSeatNumber");
    QDomElement domElement = doc.createElement("SetSeatNumber");
    doc.appendChild(domElement);

    QDomElement ErrorCodeElement = doc.createElement("ErrorCode");
    domElement.appendChild(ErrorCodeElement);
    QDomText ErrorCodeText  = doc.createTextNode(s_errorcode);
    ErrorCodeElement.appendChild(ErrorCodeText);

    strcpy(str, doc.toString().toUtf8().constData());
    if(m_ipcItalc == NULL || !m_isConnected)
        return NULL;
    if(m_ipcItalc->ipcSendMsg(str , strlen(str)) > 0)
    {
        LOG_INFO("setSeatNumber: %s", str);
    }
    else
    {
        LOG_ERR("SendSetSeatNumbre fail");
    }
    return NULL;
}

int IpcItalc::ipcSendMsg(const char *msg, const int dataLen)
{
    if(m_ipcItalc == NULL)
        return -1;
    if( msg == NULL ||  !m_isConnected)
    {
        return -1;
    }
    cout << "ipcsendms 666666666666" << msg << endl;
    //    QByteArray text(msg);
    cout << "send before#############" << endl;
    int iRet = send(m_sockfd,  msg, dataLen, 0);
    cout << "send(m_sockfd, msg, datalen, 0) : " << iRet << endl;
    return iRet;
}

int IpcItalc::ipcRecvMsg(char *msg)
{
    if( msg==NULL || !m_isConnected)
    {
        return -1;
    }

    return recv(m_sockfd, msg, 20000, 0);
}

void IpcItalc::ipcClose()
{
    if(m_sockfd > 0)
    {
        close(m_sockfd);
    }
    m_sockfd = 0;
}


