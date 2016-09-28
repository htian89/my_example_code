#include <string.h>
#include <stdlib.h>
#include "mxml.h"
#include "base64.h"
#include "vaccess.h"
#include "http.h"
#include "../common/errorcode.h"
#include "../common/log.h"
#include "csysinfobase.h"
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QString>
#include <cthreadpool.h>
#include "tcp.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "tcp.h"
#include "../ui/autologindialog.h"
#include "../frontend/cusbmap.h"

//Debug
#include <iostream>
using std::endl;
using std::cerr;

#define SERV_PORT 15001
#define BROADCAST_PORT 15002
bool has_written_italc_conf = false;
int nTerminals = 0;
extern bool g_autoLogin ;
ST_ACCESS_LOGIN_IN  param_fap; //2014-11-22
extern int role_dy;

void * FapMsgtoControl(void *arg);

static char *MakeUrl(const NETWORK *network, const char *path)
{
    char *url = (char *)calloc(1, MAX_LEN);

    if (network->stPresentServer.isHttps)//if (network->isHttps)
		strcat(url, "https://");
	else
		strcat(url, "http://");

    strcat(url, network->stPresentServer.serverAddress);//strcat(url, network->presentServer);
	strcat(url, ":");
    strcat(url, network->stPresentServer.port);//strcat(url, network->port);
	strcat(url, path);

	return url;
}

static int XMLFindIntElement(mxml_node_t *node, mxml_node_t *top, const char *name)
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
}

static int GetOnlyErrorCode(int httpStatus, const char *buffer)
{
    if (httpStatus < 0)
        return httpStatus;

    mxml_node_t *tree = NULL;	
    LOG_INFO("buffer=\t%s", buffer);
	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
    int iErrCode = XMLFindIntElement(tree, tree, "ErrorCode");
    if(NULL != tree)
        mxmlDelete(tree);
    return iErrCode;
}

static int PostCommon(const NETWORK *network, const char *logonTicket,
			   const char *path, const char *name,
			   const char *markname, const char *mark, char *buffer)
{
	int len, httpStatus;
    mxml_node_t *node = NULL;
    char  *url;//*str = NULL,
    char str[4096];
    memset(str, 0, 4096);

	url = MakeUrl(network, path);
	node = mxmlNewElement(MXML_NO_PARENT, name);
	XMLNewElementText(node, "LogonTicket", logonTicket);
    if (markname != NULL && mark != NULL)  //markname = DesktopUuid
		XMLNewElementText(node, markname, mark);
    //str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);
#ifdef DEBUG_MODE
	printf("str = \n%s\n", str);
#endif
	httpStatus = HttpPost(url, str, buffer, &len);
    free(url);
    mxmlDelete(node);
    node = NULL;

	return httpStatus;
}

static int PostCommonWithErrorCode(const NETWORK *network, const char *logonTicket,
								   const char *path, const char *name,
								   const char *markname, const char *mark)
{
	char buffer[BUFFERLENGTH];
	int httpStatus;

	httpStatus = PostCommon(network, logonTicket, path, name, markname, mark, buffer);
	return GetOnlyErrorCode(httpStatus, buffer);
}

int VAccessGetIcon(ST_ACCESS_GETICON_IN* param)
{
    int len = 0, httpStatus = 0;
    char *buffer = NULL, *url = NULL;

    if(NULL == param ||(NULL!=param && param->strUrlPath.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param or param->strUrlPath.size()<=0");
        httpStatus = -1;
    }
    else
    {
        NETWORK* network = &(param->stParamCommon.network);
        param->stParamCommon.type = TYPE_GETICON;
        url = MakeUrl(network, param->strUrlPath.c_str());
        buffer = (char *)malloc(BUFFERLENGTH);
        LOG_INFO("getICON:url=%s", url);
        httpStatus = HttpGet(url, NULL, buffer, &len);
        free(url);

        if (httpStatus < 0)
        {
            param->stGetIcon.iLen = 0;
            param->stGetIcon.data = NULL;
            free(buffer);
            LOG_ERR("httpGet failed. return value:%d", httpStatus);
        }
        else
        {
            param->stGetIcon.iLen = len;
            param->stGetIcon.data = buffer;
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = httpStatus;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

    return httpStatus;
}

int VAccessGetDomainList(ST_ACCESS_GETDOMAIN_IN *param)
{
    int errorCode =0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    NETWORK* network = &(param->stParamCommon.network);
    param->stParamCommon.type = TYPE_GETDOMAIN;
    char buffer[BUFFERLENGTH];
    int len, httpStatus;
    mxml_node_t *tree = NULL, *node;
    char *url = NULL;
    char domain[MAX_LEN];
    int i = 0;
//send http request
    strcpy(network->stPresentServer.serverAddress, network->stFirstServer.serverAddress);//strcpy(network->presentServer, network->firstServer);
    strcpy(network->stPresentServer.port, network->stFirstServer.port);
    network->stPresentServer.isHttps = network->stFirstServer.isHttps;
    url = MakeUrl(network, REQUEST_DOMAIN_LIST);
//    LOG_INFO("url:%s",url);
    httpStatus = HttpGet(url, NULL, buffer, &len);
    if(httpStatus<0 && strlen(network->stAlternateServer.serverAddress)>0 && strlen(network->stAlternateServer.port)>0)//if(httpStatus<0 && strlen(network->alternateServer)>0)
    {
        free(url);
        url = NULL;
        strcpy(network->stPresentServer.serverAddress, network->stAlternateServer.serverAddress);//strcpy(network->presentServer, network->alternateServer);
        strcpy(network->stPresentServer.port, network->stAlternateServer.port);
        network->stPresentServer.isHttps = network->stAlternateServer.isHttps;
        url = MakeUrl(network, REQUEST_DOMAIN_LIST);
        httpStatus = HttpGet(url, NULL, buffer, &len);
        LOG_INFO("url:%s",url);
    }
    while(httpStatus < 0&&g_autoLogin ){
        Sleep(3000);
        httpStatus = HttpGet(url, NULL, buffer, &len);
    }
    free(url);
    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {//parse result
        LOG_INFO("domainlist return value:%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        if (errorCode >= 0)
        {
            for (node = mxmlFindElement(tree, tree, "Domain", NULL, NULL, MXML_DESCEND);
                    node != NULL;
                    node = mxmlFindElement(node, tree, "Domain", NULL, NULL, MXML_DESCEND))
            {
                memset(domain, 0, sizeof(domain));
                if (node->child != NULL)
                    strcpy(domain, base64_decode(node->child->value.text.string));
                //strcpy(domainlists[i], domain);
                param->stDomainData.vstrDomainlists.push_back(domain);
                i++;
            }
        }
        mxmlDelete(tree);
    }

    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
    return errorCode;
}



int VAccessQueryClientVersion(ST_ACCESS_QUERY_CLIENT_VERSION* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_QUERY_CLIENT_VERSION;
        char buffer[BUFFERLENGTH], type[5], str[BUFFERLENGTH];
        int len = 0, httpStatus = 0;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        mxml_node_t *tree = NULL, *node = NULL;

        node = mxmlNewElement(MXML_NO_PARENT, "GetClientVersion");
        sprintf(type, "%d", param->iClientType);
        XMLNewElementText(node, "ClientType", type);
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);////str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("queryclientversion:\t%s", str);

        NETWORK network = param->stParamCommon.network;
        if(0==strlen(network.stPresentServer.serverAddress) && 0==strlen(network.stPresentServer.port))//if(0 == strlen(network.presentServer))
        {
            strcpy(network.stPresentServer.serverAddress, network.stFirstServer.serverAddress);//strcpy(network->presentServer, network->firstServer);
            strcpy(network.stPresentServer.port, network.stFirstServer.port);
            network.stPresentServer.isHttps = network.stFirstServer.isHttps;
        }
        url = MakeUrl(&network, REQUEST_GET_CLIENT_VER);

        httpStatus = HttpPost(url, str, buffer, &len);
        if (httpStatus < 0)
        {
            if(strlen(network.stAlternateServer.serverAddress)>0 && strlen(network.stAlternateServer.port)>0)//if(strlen(network.alternateServer)>0)
            {
                free(url);
                url = NULL;
                url = MakeUrl(&network, REQUEST_GET_CLIENT_VER);
                strcpy(network.stPresentServer.serverAddress, network.stAlternateServer.serverAddress);//strcpy(network->presentServer, network->alternateServer);
                strcpy(network.stPresentServer.port, network.stAlternateServer.port);
                network.stPresentServer.isHttps = network.stAlternateServer.isHttps;
                httpStatus = HttpPost(url, str, buffer, &len);
            }
        }
        free(url);
        mxmlDelete(node);
        node = NULL;

        if (httpStatus < 0)
        {
            LOG_ERR("httpget failed return value:%d", httpStatus);
            errorCode =  httpStatus;
        }
        else
        {
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char chVersion[MAX_LEN];
                XMLFindStrElement(tree, tree, "ClientVersion", chVersion, MAX_LEN);
                param->stQueryVersion.str_ClientVersion = chVersion;
            }
            mxmlDelete(tree);
        }
    }
    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
	return errorCode;
}
int VAccessGetUserName(ST_ACCESS_GET_USERNAME *param){
    int errorCode =0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    NETWORK* network = &(param->stParamCommon.network);
    param->stParamCommon.type = TYPE_GET_USERNAME;
    char buffer[BUFFERLENGTH];
    memset(buffer, 0, BUFFERLENGTH);
    char str[4096];
    memset(str, 0, 4096);
    int len, httpStatus;
    mxml_node_t *tree = NULL, *node;
    char *url = NULL;
//send http request

    strcpy(network->stPresentServer.serverAddress, network->stFirstServer.serverAddress);//strcpy(network->presentServer, network->firstServer);
    strcpy(network->stPresentServer.port, network->stFirstServer.port);
    network->stPresentServer.isHttps = network->stFirstServer.isHttps;
    url = MakeUrl(network, REQUEST_GET_USERNAME);
    node = mxmlNewElement(MXML_NO_PARENT, "OccupyDesktop");
    XMLNewElementText(node, "Uuid", param->uuid);
    mxmlSaveString(node, str, BUFFERLENGTH, MXML_NO_CALLBACK);
    LOG_INFO("url------> %s\nstr-----> %s", url, str);
    httpStatus = HttpPost(url, str, buffer, &len);
    if(httpStatus<0 && strlen(network->stAlternateServer.serverAddress)>0 && strlen(network->stAlternateServer.port)>0)//if(httpStatus<0 && strlen(network->alternateServer)>0)
    {
        free(url);
        url = NULL;
        strcpy(network->stPresentServer.serverAddress, network->stAlternateServer.serverAddress);//strcpy(network->presentServer, network->alternateServer);
        strcpy(network->stPresentServer.port, network->stAlternateServer.port);
        network->stPresentServer.isHttps = network->stAlternateServer.isHttps;
        url = MakeUrl(network, REQUEST_GET_USERNAME);
        httpStatus = HttpPost(url, str, buffer, &len);
        LOG_INFO("url:%s",url);
    }
    if (httpStatus < 0)
    {
        LOG_ERR("httppost failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {//parse result
        LOG_INFO("get username return value:%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        cout << "buffer : " << buffer << endl;
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        if (errorCode >= 0)
        {
            XMLFindBase64Element(tree, tree, "Username", param->userName, MAX_LEN);
            cout << "param username:" << param->userName << endl;
        }
        mxmlDelete(tree);
    }
    while(errorCode < 0 && g_autoLogin){
        Sleep(3000);
        httpStatus = HttpPost(url, str, buffer, &len);
        if (httpStatus < 0)
        {
            LOG_ERR("httppost failed return value:%d", httpStatus);
            errorCode =  httpStatus;
        }
        else
        {//parse result
            LOG_INFO("get username return value:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            cout << "buffer : " << buffer << endl;
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                XMLFindBase64Element(tree, tree, "Username", param->userName, MAX_LEN);
                cout << "param username:" << param->userName << endl;
            }
            mxmlDelete(tree);
        }
    }
    free(url);
    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
    return errorCode;
}

#include <../ipc/ipcclient.h>
extern IpcClient *g_ipcClient;
int VAccessLoginSession(ST_ACCESS_LOGIN_IN *param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    NETWORK* network = &(param->stParamCommon.network);
    USER_INFO *userInfo = &(param->stUserInfo);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);
    param->stParamCommon.type = TYPE_LOGIN;
    char buffer[BUFFERLENGTH];
    int len, httpStatus;
    mxml_node_t *node = NULL, *tree = NULL;
    char  *url = NULL;
    char str[4096];
    memset(str, 0, 4096);
    memset(login, 0, sizeof(NT_ACCOUNT_INFO));

    strcpy(network->stPresentServer.serverAddress, network->stFirstServer.serverAddress);//strcpy(network->presentServer, network->firstServer);
    strcpy(network->stPresentServer.port, network->stFirstServer.port);
    network->stPresentServer.isHttps = network->stFirstServer.isHttps;
    url = MakeUrl(network, REQUEST_LOGIN_SESSION);
    node = mxmlNewElement(MXML_NO_PARENT, "LoginSession");
    XMLNewElementText(node, "Username", base64_encode(userInfo->username));
    XMLNewElementText(node, "Password", base64_encode(userInfo->password));
    XMLNewElementText(node, "Domain", base64_encode(userInfo->domain));
    if(param->bAutoLogin){
        XMLNewElementText(node, "TerminalUuid", userInfo->uuid);
    }
    mxmlSaveString(node, str, 4096,MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
    LOG_INFO("loginsession = :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);
    if(httpStatus<0 && strlen(network->stAlternateServer.serverAddress)>0 && strlen(network->stAlternateServer.port)>0)//if(httpStatus<0 && strlen(network->alternateServer)>0)
    {
        free(url);
        url = NULL;
        strcpy(network->stPresentServer.serverAddress, network->stAlternateServer.serverAddress);//strcpy(network->presentServer, network->alternateServer);
        strcpy(network->stPresentServer.port, network->stAlternateServer.port);
        network->stPresentServer.isHttps = network->stAlternateServer.isHttps;
        url = MakeUrl(network, REQUEST_LOGIN_SESSION);
        httpStatus = HttpPost(url, str, buffer, &len);
    }
    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        param->stLoginData.stNetwork = *network;
        LOG_INFO("loginsession = :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        if (errorCode >= 0)
        {
            XMLFindStrElement(tree, tree, "LogonTicket", login->logonTicket, MAX_LEN);
            XMLFindBase64Element(tree, tree, "NtUsername", login->ntUsername, MAX_LEN);
            XMLFindBase64Element(tree, tree, "NtPassword", login->ntPassword, MAX_LEN);
            XMLFindBase64Element(tree, tree, "NtDomain", login->ntDomain, MAX_LEN);
            g_ipcClient->sendWebsocketSessionStatus(TRUE, login->logonTicket);
            cout << "vaccesslogin:ntdoamin:" << login->ntDomain << endl;
        }
        mxmlDelete(tree);
    }
    //param_fap = *param;
    memcpy(&param_fap, param, sizeof(param_fap));
    //2014-11-5
    VAccessGetTerminalFunc(param);

    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
	return errorCode;
}

/*2014-11-5 <add> */
int VAccessGetTerminalFunc(ST_ACCESS_LOGIN_IN* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }

    int len, httpStatus, value;
    char *url = NULL;
    char str[4096];
    char buffer[BUFFERLENGTH];
    char str_Uuid[MAX_LEN];
    char name[MAX_LEN];
    mxml_node_t *node = NULL, *tree = NULL;
    NETWORK* network = &(param->stParamCommon.network);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);
    CSysInfoBase *sysinfo_func = new CSysInfoUnix;

    memset(str_Uuid, 0, MAX_LEN);
    sysinfo_func->getProductUuid(str_Uuid);

    url = MakeUrl(network, REQUEST_GET_TERMINAL_FUNC);
    node = mxmlNewElement(MXML_NO_PARENT, "GetTerminalFunc");
    XMLNewElementText(node, "LogonTicket", login->logonTicket);
    XMLNewElementText(node, "Uuid", str_Uuid);
    memset(str, 0, 4096);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

    LOG_INFO("GetTerminalFunc send(url)= :%s", url);
    LOG_INFO("GetTerminalFunc send(str)= :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);

    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        LOG_INFO("GetTerminalFunc receive= :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        if (errorCode >= 0)
        {
            memset(name, 0, MAX_LEN);
            XMLFindStrElement(tree, tree, "Name", name, MAX_LEN);
            value = XMLFindIntElement(tree, tree, "Value");
            if (strlen(name) > 0)
                sysinfo_func->setTerminalFuncSwitch(name, value);
        }
        mxmlDelete(tree);
    }

    free(sysinfo_func);

    return errorCode;
}


/*2014-11-21*/
void * ServerTeacherForFap(void * arg)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 1);

    LOG_INFO("Waiting for FAP, Accepting connections ...\n");
    while (1)
    {
        cliaddr_len = sizeof(cliaddr);
        memset(&cliaddr, 0, sizeof(cliaddr));
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);

        pthread_t thread;
        VCLIENT_THREAD_CREATE(thread, NULL, FapMsgtoControl, &connfd);
        THREAD_DETACH(thread);
    }
}

void* FapMsgtoControl(void *arg)
{
    FapvclientMsg *msgin;
    int connfd = *((int* )arg);
    char buf[MAX_LEN];
    int n, flag_SBS = 0;
    while(1)
    {
        memset(buf, 0, MAX_LEN);
        n = read(connfd, (void *)buf, sizeof(FapvclientMsg));
        if( n <=  0)
            break;
        LOG_INFO("buf: %s\n", buf);
        msgin = (FapvclientMsg *)buf;
        LOG_INFO("received date type: %d\n", msgin->type);
        if (FAP_VCLIENT_MSG_PRESENTATION_START == msgin->type)
        {
            strcpy(param_fap.stLoginData.stLoginInfo.broadcastscreen_ip, msgin->ip);
            param_fap.stLoginData.stLoginInfo.broadcastscreen_port = msgin->port;

            if (0 == VAccessStartClass(&param_fap))
            {
                LOG_INFO("************Start Class Success************");
                if (0 == VAccessStartBroadcastScreen(&param_fap))
                    flag_SBS = 1;
            }
            else
            {
                LOG_ERR("************Start Class Failed************");
            }

        }

        if (FAP_VCLIENT_MSG_PRESENTATION_STOP == msgin->type)
        {
            VAccessEndBroadcastScreen(&param_fap);
            VAccessEndClass(&param_fap);
            /*
            if (1 == flag_SBS)
            {
                if (0 == VAccessEndBroadcastScreen(&param_fap))

            }
            if (0 == VAccessEndClass(&param_fap))
            {
                LOG_ERR("************End Class Success************");
            }
            else
            {
                LOG_ERR("************End Class Failed************");
            }
        */
        }
    }

}

int VAccessStartClass(ST_ACCESS_LOGIN_IN* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }

    int len, httpStatus;
    char *url = NULL;
    char str[4096];
    char buffer[BUFFERLENGTH];

    mxml_node_t *node = NULL, *tree = NULL;
    NETWORK* network = &(param->stParamCommon.network);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);


    url = MakeUrl(network, REQUEST_START_CLASS);
    node = mxmlNewElement(MXML_NO_PARENT, "StartClass");
    XMLNewElementText(node, "LogonTicket", login->logonTicket);

    memset(str, 0, 4096);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

    LOG_INFO("StartClass send(url)= :%s", url);
    LOG_INFO("StartClass send(str)= :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);

    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        LOG_INFO("StartClass recv= :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
    }

    return errorCode;
}

int VAccessEndClass(ST_ACCESS_LOGIN_IN * param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }

    int len, httpStatus;
    char *url = NULL;
    char str[4096];
    char buffer[BUFFERLENGTH];

    mxml_node_t *node = NULL, *tree = NULL;
    NETWORK* network = &(param->stParamCommon.network);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);


    url = MakeUrl(network, REQUEST_END_CLASS);
    node = mxmlNewElement(MXML_NO_PARENT, "EndClass");
    XMLNewElementText(node, "LogonTicket", login->logonTicket);

    memset(str, 0, 4096);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

    LOG_INFO("EndClass send(url)= :%s", url);
    LOG_INFO("EndClass send(str)= :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);

    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        LOG_INFO("EndClass recv= :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
    }

    return errorCode;
}

int VAccessStartBroadcastScreen(ST_ACCESS_LOGIN_IN * param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }

    int len, httpStatus;
    char *url = NULL;
    char str[4096];
    char buffer[BUFFERLENGTH];
    char port[MIN_LEN];

    mxml_node_t *node = NULL, *tree = NULL;
    NETWORK* network = &(param->stParamCommon.network);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);

    url = MakeUrl(network, REQUEST_START_BROADCAST_SCREEN);
    node = mxmlNewElement(MXML_NO_PARENT, "StartBroadcastScreen");
    XMLNewElementText(node, "LogonTicket", login->logonTicket);
    XMLNewElementText(node, "Ip", login->broadcastscreen_ip);
    sprintf(port, "%d", login->broadcastscreen_port);
    XMLNewElementText(node, "Port",  port);
    memset(str, 0, 4096);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

    LOG_INFO("StartBroadcastScreen send(str)= :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);

    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        LOG_INFO("StartBroadcastScreen recv= :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
    }

    return errorCode;
}

int VAccessEndBroadcastScreen(ST_ACCESS_LOGIN_IN * param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }

    int len, httpStatus;
    char *url = NULL;
    char str[4096];
    char buffer[BUFFERLENGTH];

    mxml_node_t *node = NULL, *tree = NULL;
    NETWORK* network = &(param->stParamCommon.network);
    NT_ACCOUNT_INFO *login =&(param->stLoginData.stLoginInfo);

    url = MakeUrl(network, REQUEST_END_BROADCAST_SCREEN);
    node = mxmlNewElement(MXML_NO_PARENT, "EndBroadcastScreen");
    XMLNewElementText(node, "LogonTicket", login->logonTicket);
    memset(str, 0, 4096);
    mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

    LOG_INFO("EndBroadcastScreen send(str)= :%s", str);

    httpStatus = HttpPost(url, str, buffer, &len);

    free(url);
    mxmlDelete(node);
    node = NULL;

    if (httpStatus < 0)
    {
        LOG_ERR("httpget failed return value:%d", httpStatus);
        errorCode =  httpStatus;
    }
    else
    {
        LOG_INFO("EndBroadcastScreen recv= :%s", buffer);
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
    }

    return errorCode;
}




int VAccessKeepSession(ST_ACCESS_KEEPSESSION_IN *param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    param->stParamCommon.type = TYPE_KEEPSESSION;
    NETWORK* network = &(param->stParamCommon.network);
    const char* logonTicket = param->str_SessionTicket.c_str();
    if(NULL == logonTicket)
    {
        LOG_ERR("%s", "loginTicket is NULL");
        errorCode = -5;
    }
    else
    {
        char buffer[BUFFERLENGTH];
        int httpStatus;
        mxml_node_t *tree = NULL;

        httpStatus = PostCommon(network, logonTicket, REQUEST_KEEP_SESSION,
                                "KeepSession",	NULL, NULL, buffer);

        if (httpStatus < 0)
        {
            LOG_ERR("PostCommon failed return value:%d", httpStatus);
            errorCode =  httpStatus;
        }
        else
        {
            LOG_INFO("keepSession return value:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
                param->st_keepSession.i_timeOut = XMLFindIntElement(tree, tree, "Timeout");//*timeout = XMLFindIntElement(tree, tree, "Timeout");

            mxmlDelete(tree);
        }
    }
    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
	return errorCode;
}

int VAccessLogoutSession(ST_ACCESS_LOGOUT_IN* param)
{
    if (has_written_italc_conf == true) {
        has_written_italc_conf = false;
    }
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    param->stParamCommon.type = TYPE_LOGOUT;
    const char* logonTicket = param->str_SessionTicket.c_str();
    if(NULL == logonTicket)
    {
        LOG_ERR("%s", "loginTicket is NULL");
        errorCode = -5;
    }
    else
    {
        errorCode = PostCommonWithErrorCode(&(param->stParamCommon.network),
                logonTicket, REQUEST_LOGOUT_SESSION, "LogoutSession", NULL, NULL);
    }

    g_ipcClient->sendWebsocketSessionStatus(FALSE, NULL);

    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        cerr << "the type in vAccessLogoutSession: " << param->stParamCommon.type << endl;
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
    return errorCode;
}

int VAccessSwitchAccessSession(ST_ACCESS_LOGOUT_IN* param)
{
    if (has_written_italc_conf == true) {
        has_written_italc_conf = false;
    }
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        return -1;
    }
    param->stParamCommon.type = TYPE_SWITCH_ACCESS;
    const char* logonTicket = param->str_SessionTicket.c_str();
    if(NULL == logonTicket)
    {
        LOG_ERR("%s", "loginTicket is NULL");
        errorCode = -5;
    }
    else
    {
        errorCode = PostCommonWithErrorCode(&(param->stParamCommon.network),
                logonTicket, REQUEST_LOGOUT_SESSION, "LogoutSession", NULL, NULL);
    }

    param->stParamCommon.iErrCode = errorCode;
    if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
    {
        cerr << "the type in VAccessSwitchAccessSession: " << param->stParamCommon.type << endl;
        (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
    }
    return errorCode;
}

int VAccessGetUserInfo(ST_ACCESS_GETUSERINFO_IN* param)
{
    int errorCode = 0;
    const char* logonTicket = NULL;
    if(NULL == param ||(NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_GETUSERINFO;
        logonTicket = param->str_SessionTicket.c_str();        
        char buffer[BUFFERLENGTH];
        int len;
        char *str, *url;

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_GET_USER_INFO);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "LogonTicket=%s", logonTicket);

        errorCode = HttpGet(url, str, buffer, &len);
        free(url);
        free(str);
        if (errorCode < 0)
        {
            LOG_ERR("httpget failed. return value:%d", errorCode);
        }
        else
        {
            LOG_INFO("VAccessGetUserInfo:%s", buffer);
            mxml_node_t *tree = NULL, *node;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            LOG_INFO("VaccessGetUserInfo:%d",errorCode);
            if (errorCode >= 0)
            {
                VIRTUALDISK vDisk;
                vector<VIRTUALDISK> * pVDisks = &(param->st_GetuserInfo.vstVirtualDisks);
                for (node = mxmlFindElement(tree, tree, "Disk", NULL, NULL, MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "Disk", NULL, NULL, MXML_DESCEND))
                {
                    memset(&vDisk, 0 ,sizeof(VIRTUALDISK));
                    XMLFindStrElement(node, node, "DevicePath", vDisk.devicePath, MAX_LEN);
                    XMLFindStrElement(node, node, "DiskSize", vDisk.diskSize, MAX_LEN);
                    XMLFindStrElement(node, node, "SizeUnit", vDisk.sizeUnit, MAX_LEN);
                    pVDisks->push_back(vDisk);
                }
                XMLFindBase64Element(tree, tree, "NtUsername", param->st_GetuserInfo.stNtAccountInfo.ntUsername, MAX_LEN);
                XMLFindBase64Element(tree, tree, "NtPassword", param->st_GetuserInfo.stNtAccountInfo.ntPassword, MAX_LEN);
                param->st_GetuserInfo.stNtAccountInfo.Role = XMLFindIntElement(tree, tree, "Role");
                role_dy = param->st_GetuserInfo.stNtAccountInfo.Role;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}


int VAccessListUserResource(ST_ACCESS_LISTRES_IN* param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_LIST_USER_RES;
        const char* logonTicket = param->str_SessionTicket.c_str();
        NETWORK* network = &(param->stParamCommon.network);
        char buffer[BUFFERLENGTH*2];
        int len = 0, httpStatus = 0;
        char *str = NULL, *url = NULL;

        url = MakeUrl(network, REQUEST_GET_USER_RES);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "LogonTicket=%s", logonTicket);
         httpStatus = HttpGet(url, str, buffer, &len);
        free(url);
        free(str);
//        LOG_DEBUG("need get resparam:%d",int(param->b_getResParam));
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpGet failed! return value:%d", httpStatus);
        }
        else
        {
            LOG_DEBUG("VAccessListUserResource:%s", buffer);
            cout << "listuserresource: ======" << buffer << endl;
            for(int i=0; buffer[i] != '\0' && i < BUFFERLENGTH*2; i++) {
                if (buffer[i] == ' ') {
                    buffer[i] = '`';
                }
            }
            mxml_node_t *tree = NULL, *node = NULL;            
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                for (node = mxmlFindElement(tree, tree, "Application", NULL, NULL, MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "Application", NULL, NULL, MXML_DESCEND))
                {
                    APP_LIST app;
                    memset(&app, 0, sizeof(APP_LIST));
                    app.type = XMLFindIntElement(node, node, "Type");
                    app.type == 1 ? app.desktopType = NORMALDESKTOP:app.desktopType = VIRTUALAPP;

                    app.displayprotocol = XMLFindIntElement(node, node, "DisplayProtocol");
                    XMLFindStrElement(node, node, "Uuid", app.uuid, MAX_LEN);
                    XMLFindBase64Element(node, node, "Name", app.name, MAX_LEN);
                    XMLFindStrElement(node, node, "HostName", app.hostname, MAX_LEN);
                    XMLFindBase64Element(node, node, "Description", app.description, MAX_LEN);
                    XMLFindBase64Element(node, node, "HostDescription", app.hostDescription, MAX_LEN);
                    if(param->b_getResParam)
                    {
                        if (app.type == 1)
                            VAccessGetResourceParameters(network, logonTicket, app.uuid, 1,
                                                         &app.resParams);
                        else
                            VAccessGetResourceParameters(network, logonTicket, app.uuid, 0,
                                                         &app.resParams);
                    }

                    /**
                     * Because the Fronview3000 can not support virtual app current time , so ignore it.
                     **/
#ifdef unix
                    if(app.type == 1)
                         param->sListUserRes.stAppList.push_back(app);
#else
                    param->sListUserRes.stAppList.push_back(app);
#endif
                }
                for (node = mxmlFindElement(tree, tree, "DesktopPool", NULL, NULL, MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "DesktopPool", NULL, NULL, MXML_DESCEND))
                {
                    APP_LIST desktopPool;
                    memset(&desktopPool, 0, sizeof(APP_LIST));
                    desktopPool.desktopType = DESKTOPPOOL;
                    XMLFindStrElement(node, node, "Uuid", desktopPool.uuid, MAX_LEN);
                    XMLFindBase64Element(node, node, "Name", desktopPool.name, MAX_LEN);
                    desktopPool.displayprotocol = XMLFindIntElement(node, node, "DisplayProtocol");
                    desktopPool.type = XMLFindIntElement(node, tree, "Type");
                    desktopPool.userAssignment = XMLFindIntElement(node, node, "UserAssignment");
                    desktopPool.sourceType = XMLFindIntElement(node, node, "SourceType");
                    desktopPool.enable = XMLFindIntElement(node, node, "Enable");
                    desktopPool.state = XMLFindIntElement(node, node, "State");
                    XMLFindBase64Element(node, tree, "Description", desktopPool.description, MAX_LEN);
                    desktopPool.powerOnVmNum = XMLFindIntElement(node, node, "PowerOnVmNum");
                    desktopPool.rdpOnVmNum = XMLFindIntElement(node, node, "RdpOnVmNum");
                    desktopPool.rdpServiceState = XMLFindIntElement(node, node, "RdpServiceState");
                    desktopPool.vmState = XMLFindIntElement(node, node, "VmState");
                    XMLFindStrElement(node, node, "Os", desktopPool.OsType, MAX_LEN);
                    for (int i=0; i < MAX_LEN && desktopPool.OsType[i] != '\0'; i++) {
                        if (desktopPool.OsType[i] == '`') {
                            desktopPool.OsType[i] = ' ';
                        }
                    }
                    LOG_INFO("desktopPool.OsType: %s", desktopPool.OsType);
                    if(param->b_getResParam)
                    {
                        VAccessGetResourceParameters(network, logonTicket, desktopPool.uuid, 2,
                                                     &desktopPool.resParams);
                    }
                    param->sListUserRes.stAppList.push_back(desktopPool);
                }
                for(node = mxmlFindElement(tree,tree,"RemoteDesktop",NULL,NULL,MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "RemoteDesktop",NULL,NULL,MXML_DESCEND))
                {
                    APP_LIST remoteDesktop;
                    memset(&remoteDesktop, 0, sizeof(APP_LIST));
                    remoteDesktop.desktopType = REMOTEDESKTOP;
                    XMLFindBase64Element(node, node, "Uuid", remoteDesktop.uuid, MAX_LEN);
                    XMLFindBase64Element( node, node, "Name", remoteDesktop.name, MAX_LEN);
                    remoteDesktop.displayprotocol = XMLFindIntElement(node, node, "DisplayProtocol");
                    XMLFindStrElement(node, node, "HostName", remoteDesktop.hostname, MAX_LEN);
                    remoteDesktop.rdpServiceState = XMLFindIntElement(node, node, "RdpServiceState");
                    remoteDesktop.vmState = XMLFindIntElement(node, node, "VmState");
                    XMLFindBase64Element(node, node, "Description", remoteDesktop.description, MAX_LEN);
                    XMLFindStrElement(node, node, "Os", remoteDesktop.OsType, MAX_LEN);
                    for (int i=0; i < MAX_LEN && remoteDesktop.OsType[i] != '\0'; i++) {
                        if (remoteDesktop.OsType[i] == '`') {
                            remoteDesktop.OsType[i] = ' ';
                        }
                    }
                    LOG_INFO("remoteDesktop.OsType %s", remoteDesktop.OsType);
                    if(param->b_getResParam)
                        VAccessGetResourceParameters(network, logonTicket, remoteDesktop.uuid, 3,
                                                     &remoteDesktop.resParams);
                    param->sListUserRes.stAppList.push_back(remoteDesktop);
                }
                nTerminals = 0;
                for(node = mxmlFindElement(tree,tree,"Terminal",NULL,NULL,MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "Terminal",NULL,NULL,MXML_DESCEND))
                {
                    APP_LIST Terminal;
                    nTerminals++;
                    memset(&Terminal, 0, sizeof(APP_LIST));
                    XMLFindStrElement(node, node, "Uuid", Terminal.uuid, MAX_LEN);
                    Terminal.desktopType = TERMINAL;
                    XMLFindStrElement(node, node, "Ip", Terminal.TerminalIp, MAX_LEN);
                    strcpy(Terminal.name, "终端监控");
                    XMLFindStrElement(node, node,"Name", Terminal.TerminalName,MAX_LEN );
                    Terminal.state = XMLFindIntElement(node, node, "State");
                    Terminal.displayprotocol = ONLY_TERMINAL;
                    if(param->b_getResParam)
                        VAccessGetResourceParameters(network, logonTicket, Terminal.uuid, 3, &Terminal.resParams);
                    param->sListUserRes.stAppList.push_back(Terminal);
                }
                for(unsigned int loop = 0; loop < param->sListUserRes.stAppList.size(); loop++)
                {
                    param->sListUserRes.stAppBakList.push_back(param->sListUserRes.stAppList[loop]);
                    cout << param->sListUserRes.stAppBakList[loop].TerminalName;
                }

                // clean terminals leave only one
                if (has_written_italc_conf == true) {

                    for (int i = nTerminals; i > 1; i--) {
                        qDebug() << "!!!!!We pop to deal with RelistUserRsource: " << i;
                        param->sListUserRes.stAppList.pop_back();
                    }
                } else { // the first ListUserResource call
                    QDomDocument doc( "italc-config-file" );

                    QDomElement italc_config = doc.createElement( "globalclientconfig" );
                    italc_config.setAttribute( "version", "2.0.1" );
                    doc.appendChild( italc_config );

                    QDomElement body = doc.createElement( "body" );
                    italc_config.appendChild( body );

                    QDomElement classroom = doc.createElement("classroom");
                    classroom.setAttribute("name", "vclient");
                    body.appendChild( classroom);

                    QDomElement client;

                    for ( unsigned int i = 0; i < (param->sListUserRes.stAppList).size(); i++)
                    {
                        //CSysInfoBase *sysinfo = new CSysInfoUnix;
                        CSysInfoUnix *sysinfo = new CSysInfoUnix;
                        const int size = 256;
                        char ipAddress[size];
                        sysinfo->getIpAddress(ipAddress);
                        if (0 != strcmp((param->sListUserRes.stAppList)[i].TerminalIp, ipAddress)) {
                            if ((param->sListUserRes.stAppList)[i].desktopType ==TERMINAL) {
                                client = doc.createElement("client");
                                client.setAttribute("hostname", (param->sListUserRes.stAppList)[i].TerminalIp);
                                client.setAttribute("mac", "");
                                client.setAttribute("type", "0");
                                client.setAttribute("id", "");
                                client.setAttribute("name","");
                                classroom.appendChild( client);
                            }
                        }
                     }

                    QString xml = "<?xml version=\"1.0\"?>\n" + doc.toString( 2 );

                    QDir confdir("/root/.italc");

                    if (confdir.exists() == false) {
                        bool mkdirRet = confdir.mkpath("/root/.italc");
                        if( false == mkdirRet) {
                            LOG_ERR("%s", "can't make dir /root/.italc");
                            qDebug() << "can't make dir /root/.italc";
                        }
                    }

                    QString m_file = "/root/.italc/GlobalConfig.xml";

                    QFile( m_file + ".bak" ).remove();
                    QFile( m_file ).copy( m_file +
                                                    ".bak" );
                    QFile outfile( m_file );

                    outfile.open( QFile::WriteOnly | QFile::Truncate );

                    outfile.write( xml.toUtf8() );
                    qDebug() << "we write GlobalConfig.xml";
                    outfile.close();

                    // clean terminals leave only one
                    for (int i = nTerminals; i > 1; i--) {
                        param->sListUserRes.stAppList.pop_back();
                        qDebug() << "!!!!!We pop: " << i;
                    }

                    has_written_italc_conf = true;
                }
            }
            else
                LOG_ERR("listuserresource:\t%s", buffer);
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessGetResourceParam(GET_RES_PARAM_IN* param)
//(const NETWORK *network, const char *logonTicket,
//                                 const char *resourceUuid, const int resourceType,
//                                 RESOURCE_PARAMETERS *resourceParameters)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param|| param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH];
        int len;
        mxml_node_t *node = NULL, *tree = NULL, *nodes = NULL;
        char  *url = NULL, str[4096], type[5];
        memset(str, 0, 4096);

        url = MakeUrl(&(param->stParamCommon.network), "/RestService/Resource/GetResourceParameters");
        node = mxmlNewElement(MXML_NO_PARENT, "GetResourceParameters");
        XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
        XMLNewElementText(node, "ResourceUuid", param->strResUuid.c_str());
        sprintf(type, "%d", param->iResType);
        XMLNewElementText(node, "ResourceType", type);
        //str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        mxmlSaveString(node, str, 4096,MXML_NO_CALLBACK);
#ifdef DEBUG_MODE
//    printf("getresourceparameters = \n%s\n", str);
#endif
        errorCode = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;

        LOG_DEBUG("VAccessGetResourceParameters: request string:%s, \n\t\treturn value:%s", str, buffer);
        if (errorCode < 0)
        {
            LOG_ERR("httpget failed. return value:%d", errorCode);
        }
        else
        {
#ifdef DEBUG_MODE
//	printf("getresourceparameters:\n%s\n", buffer);
#endif
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode < 0)
            {
                LOG_ERR("failed. return value:%d", errorCode);
            }
            else
            {
                strcpy(param->stResParm_out.deskUuid, param->strResUuid.c_str());
                RESOURCE_PARAMETERS* resourceParameters = &(param->stResParm_out.stResPara);
                memset(resourceParameters, 0, sizeof(RESOURCE_PARAMETERS));
                resourceParameters->protocol = XMLFindIntElement(tree, tree, "Protocol");
                XMLFindStrElement(tree, tree, "HostName", resourceParameters->hostName, MAX_LEN);
                XMLFindStrElement(tree, tree, "HostPort", resourceParameters->hostPort,	MAX_LEN);
                XMLFindBase64Element(tree, tree, "ForceUsername", resourceParameters->forceUsername, MAX_LEN);
                XMLFindBase64Element(tree, tree, "ForcePassword", resourceParameters->forcePassword, MAX_LEN);
                XMLFindBase64Element(tree, tree, "ForceDomain", resourceParameters->forceDomain, MAX_LEN);
                XMLFindBase64Element(tree, tree, "AlternateShell", resourceParameters->alternateShell, MAX_LEN);
                XMLFindBase64Element(tree, tree, "WorkingDir", resourceParameters->workingDir, MAX_LEN);
                node = mxmlFindElement(tree, tree, "Rail", NULL, NULL, MXML_DESCEND);
                if (node != NULL)
                {
                    XMLFindBase64Element(node, tree, "ApplicationName",
                                        resourceParameters->rail.applicationName, MAX_LEN);
                    XMLFindBase64Element(node, tree, "WorkingDirectory",
                                        resourceParameters->rail.workingDirectory, MAX_LEN);
                    XMLFindBase64Element(node, tree, "Arguments",
                                        resourceParameters->rail.arguments, MAX_LEN);
                }

                XMLFindStrElement(tree, tree, "ColorDepth", resourceParameters->colorDepth, MIN_LEN);
                XMLFindStrElement(tree, tree, "FullScreen", resourceParameters->fullScreen, MIN_LEN);
                XMLFindStrElement(tree, tree, "Resolution", resourceParameters->resolution, MIN_LEN);
                resourceParameters->compression = XMLFindIntElement(tree, tree, "Compression");
                resourceParameters->performance = XMLFindIntElement(tree, tree, "Performance");
                resourceParameters->audio = XMLFindIntElement(tree, tree, "Audio");
                resourceParameters->audioIn = XMLFindIntElement(tree, tree, "AudioIn");
                resourceParameters->printer = XMLFindIntElement(tree, tree, "Printer");
                resourceParameters->disk = XMLFindIntElement(tree, tree, "Disk");
                resourceParameters->smartcard = XMLFindIntElement(tree, tree, "Smartcard");
                resourceParameters->serialPort = XMLFindIntElement(tree, tree, "SerialPort");
                resourceParameters->parallelPort = XMLFindIntElement(tree, tree, "ParallelPort");
                resourceParameters->clipboard = XMLFindIntElement(tree, tree, "Clipboard");
                for (node = mxmlFindElement(tree, tree, "Usb", NULL, NULL, MXML_DESCEND);
                        node != NULL;
                        node = mxmlFindElement(node, tree, "Usb", NULL, NULL, MXML_DESCEND))
                {
                    if (strlen(resourceParameters->usb) > 0)
                    {
                        strcat(resourceParameters->usb, " ");
                        strcat(resourceParameters->usb, node->child->value.text.string);
                    }
                    else
                        strcpy(resourceParameters->usb, node->child->value.text.string);
                }
                //for usb type
                int add = 0;
                node = mxmlFindElement(tree, tree, "AllowTypes", NULL, NULL, MXML_DESCEND);
                for(nodes = mxmlFindElement(node, node, "Type", NULL, NULL, MXML_DESCEND);
                    nodes != NULL; nodes = mxmlFindElement(nodes, node, "Type", NULL, NULL, MXML_DESCEND))
                {
                    strcpy(resourceParameters->usbType[add], nodes->child->value.text.string);
                    add++;
                }
            } 
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessGetResourceParameters(const NETWORK *network, const char *logonTicket,
								 const char *resourceUuid, const int resourceType,
                                 RESOURCE_PARAMETERS *resourceParameters)
{
	char buffer[BUFFERLENGTH];
	int len, httpStatus, errorCode;
    mxml_node_t *node = NULL, *tree = NULL, *nodes = NULL;
    char  *url = NULL;
    char str[4096];
	char type[5];
    memset(str, 0, 4096);

	url = MakeUrl(network, "/RestService/Resource/GetResourceParameters");
	node = mxmlNewElement(MXML_NO_PARENT, "GetResourceParameters");
	XMLNewElementText(node, "LogonTicket", logonTicket);
	XMLNewElementText(node, "ResourceUuid", resourceUuid);
	sprintf(type, "%d", resourceType);
	XMLNewElementText(node, "ResourceType", type);
    //str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
    mxmlSaveString(node, str, 4096,MXML_NO_CALLBACK);
#ifdef DEBUG_MODE
//    printf("getresourceparameters = \n%s\n", str);
#endif
    LOG_ERR("VAccessGetResourceParameters StartTime ");
	httpStatus = HttpPost(url, str, buffer, &len);
    free(url);
    mxmlDelete(node);

    node = NULL;

	if (httpStatus < 0)
		return httpStatus;
    LOG_DEBUG("VAccessGetResourceParameters:%s", buffer);

#ifdef DEBUG_MODE
//	printf("getresourceparameters:\n%s\n", buffer);
#endif
	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
	errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
	if (errorCode < 0)
    {
		return errorCode;
    }
    LOG_ERR("VAccessGetResourceParameters EndTime ");
	resourceParameters->protocol = XMLFindIntElement(tree, tree, "Protocol");
    XMLFindStrElement(tree, tree, "HostName", resourceParameters->hostName, MAX_LEN);
    XMLFindStrElement(tree, tree, "HostPort", resourceParameters->hostPort,	MAX_LEN);
    XMLFindBase64Element(tree, tree, "ForceUsername", resourceParameters->forceUsername, MAX_LEN);
    XMLFindBase64Element(tree, tree, "ForcePassword", resourceParameters->forcePassword, MAX_LEN);
    XMLFindBase64Element(tree, tree, "ForceDomain", resourceParameters->forceDomain, MAX_LEN);
    XMLFindBase64Element(tree, tree, "AlternateShell", resourceParameters->alternateShell, MAX_LEN);
    XMLFindBase64Element(tree, tree, "WorkingDir", resourceParameters->workingDir, MAX_LEN);
	node = mxmlFindElement(tree, tree, "Rail", NULL, NULL, MXML_DESCEND);
	if (node != NULL)
	{
		XMLFindBase64Element(node, tree, "ApplicationName",
                            resourceParameters->rail.applicationName, MAX_LEN);
		XMLFindBase64Element(node, tree, "WorkingDirectory",
                            resourceParameters->rail.workingDirectory, MAX_LEN);
		XMLFindBase64Element(node, tree, "Arguments",
                            resourceParameters->rail.arguments, MAX_LEN);
	}

    XMLFindStrElement(tree, tree, "ColorDepth", resourceParameters->colorDepth, MIN_LEN);
    XMLFindStrElement(tree, tree, "FullScreen", resourceParameters->fullScreen, MIN_LEN);
    XMLFindStrElement(tree, tree, "Resolution", resourceParameters->resolution, MIN_LEN);
	resourceParameters->compression = XMLFindIntElement(tree, tree, "Compression");
	resourceParameters->performance = XMLFindIntElement(tree, tree, "Performance");
	resourceParameters->audio = XMLFindIntElement(tree, tree, "Audio");
	resourceParameters->audioIn = XMLFindIntElement(tree, tree, "AudioIn");
	resourceParameters->printer = XMLFindIntElement(tree, tree, "Printer");
	resourceParameters->disk = XMLFindIntElement(tree, tree, "Disk");
	resourceParameters->smartcard = XMLFindIntElement(tree, tree, "Smartcard");
	resourceParameters->serialPort = XMLFindIntElement(tree, tree, "SerialPort");
	resourceParameters->parallelPort = XMLFindIntElement(tree, tree, "ParallelPort");
	resourceParameters->clipboard = XMLFindIntElement(tree, tree, "Clipboard");
	for (node = mxmlFindElement(tree, tree, "Usb", NULL, NULL, MXML_DESCEND);
			node != NULL;
			node = mxmlFindElement(node, tree, "Usb", NULL, NULL, MXML_DESCEND))
	{
		if (strlen(resourceParameters->usb) > 0)
		{
			strcat(resourceParameters->usb, " ");
			strcat(resourceParameters->usb, node->child->value.text.string);
		}
		else
			strcpy(resourceParameters->usb, node->child->value.text.string);
	}
    //for usb type
    int add = 0;
    node = mxmlFindElement(tree, tree, "AllowTypes", NULL, NULL, MXML_DESCEND);
    for(nodes = mxmlFindElement(node, node, "Type", NULL, NULL, MXML_DESCEND);
        nodes != NULL; nodes = mxmlFindElement(nodes, node, "Type", NULL, NULL, MXML_DESCEND))
    {
        strcpy(resourceParameters->usbType[add], nodes->child->value.text.string);
        add++;
    }
    mxmlDelete(tree);
    LOG_ERR("VAccessGetResourceParameters FunctionOver ");
	return errorCode;
}

int VAccessLaunchResource(ST_ACCESS_LAUNCHRES_IN* param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && (param->str_SessionTicket.size()<=0 || param->strResUuid.size()<=0)))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0 param->strResUuid.size()<=0");
        errorCode = -1;
    }
    else
    {
        int iTryTimes = 0;
        while (iTryTimes<2)
        {
            param->stParamCommon.type = TYPE_LAUNCH_RES;
            char buffer[BUFFERLENGTH], str[BUFFERLENGTH], string[5];
            int len = 0, httpStatus = 0;
            mxml_node_t *node = NULL, *tree = NULL;
            char  *url = NULL;

            //[haproxy forward] get mac --------start
            CSysInfoBase *sysinfo_mac = new CSysInfoUnix; //2014-9-26 to get mac
            char mac_save[MIN_LEN] = {0};

            sysinfo_mac->getMacAddress(mac_save);
            //[haproxy forward] get mac ---------end

            memset(str, 0, BUFFERLENGTH);
            memset(&(param->stLaunchResData.stResInfo), 0, sizeof(RESOURCE_INFO));
            param->stLaunchResData.stResInfo.iIdleTime = -1; //this is the default
                //value which indicate invalid(the vaccess doesnot pass this value)

            url = MakeUrl(&(param->stParamCommon.network), REQUEST_LAUNCH_RES);
            node = mxmlNewElement(MXML_NO_PARENT, "LaunchResource");
            XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
            XMLNewElementText(node, "ResourceUuid", param->strResUuid.c_str());
            sprintf(string, "%d", param->iResType);
            XMLNewElementText(node, "ResourceType", string);
            sprintf(string, "%d", param->iDisplayProtocol);
            XMLNewElementText(node, "DisplayProtocol", string);
            XMLNewElementText(node, "LocalMac", mac_save); //[haproxy forward] 2014-9-26 add LocalMac
            mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
            LOG_INFO("launchresource:\t%s",str);
            LOG_ERR("VAccessLaunchResource StartTime ");
            httpStatus = HttpPost(url, str, buffer, &len);
             free(url);
            mxmlDelete(node);
            node = NULL;
            if (httpStatus < 0)
            {
                errorCode = httpStatus;
                LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
            }
            else
            {
                LOG_INFO("launchresource:\t%s",buffer);
                LOG_ERR("VAccessLaunchResource EndTime ");
                tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
                errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
                if(errorCode >= 0)
                {
                    int value = XMLFindStrElement(tree,tree,"Ip",param->stLaunchResData.stResInfo.ipAddr,MAX_LEN);

                    if(value == -1)
                    {
                        memset(param->stLaunchResData.stResInfo.ipAddr,0,MAX_LEN);
                        param->stLaunchResData.stResInfo.iIdleTime = -1;
                    }
                    memset(param->stLaunchResData.stResInfo.port, 0, MIN_LEN); //memset
                    XMLFindStrElement(tree, tree, "Port", param->stLaunchResData.stResInfo.port, MIN_LEN);
                    memset(param->stLaunchResData.stResInfo.resourceTicket, 0, MAX_LEN); //memset
                    XMLFindStrElement(tree, tree, "ResourceTicket", param->stLaunchResData.stResInfo.resourceTicket, MAX_LEN);
                    XMLFindBase64Element(tree, tree, "SecurtityToken", param->stLaunchResData.stResInfo.SecurtityToken, MAX_LEN);

                    memset(param->stLaunchResData.stResInfo.SecurtityPort, 0, MAX_LEN); //memset
                    XMLFindStrElement(tree, tree, "SecurtityPort", param->stLaunchResData.stResInfo.SecurtityPort, MAX_LEN);

                    /* get Uuid or SecurityUuid  2014-10-28 change      start*/
                    memset(param->stLaunchResData.stResInfo.Uuid, 0, MAX_LEN);
                    XMLFindStrElement(tree, tree, "Uuid", param->stLaunchResData.stResInfo.Uuid, MAX_LEN); //haproxy: get Uuid
                    LOG_INFO("haproxy: Uuid:\t%s", param->stLaunchResData.stResInfo.Uuid);
                    memset(param->stLaunchResData.stResInfo.SecurityUuid, 0, MAX_LEN);
                    XMLFindStrElement(tree, tree, "SecurityUuid", param->stLaunchResData.stResInfo.SecurityUuid, MAX_LEN);
                    LOG_INFO("haproxy: SecurityUuid:\t%s", param->stLaunchResData.stResInfo.SecurityUuid);

                    param->stLaunchResData.stResInfo.iIdleTime = XMLFindIntElement(tree, tree, "IdleTime");
                }
                mxmlDelete(tree);
            }
            if(ERROR_CREATE_CONNECTION_FAIL == errorCode)
            {//try again
                iTryTimes++;
                LOG_ERR("%s","ERROR_CREATE_CONNECTION_FAIL == errorCode try again..");
                continue;
            }
            else
            {
                iTryTimes = 2;
                break;
            }
        }
    }

    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type); // callback_csession()
        }
    }

	return errorCode;
}

int VAccessShutdownResource(ST_ACCESS_SHUTDOWN_RES_IN* param)
//const NETWORK *network, const char *logonTicket,
//const char *resourceTicket
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && (param->str_SessionTicket.size()<=0||param->str_resTicket.size()<=0)))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0||param->str_resTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_SHUTDOWN_RES;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0, httpStatus = 0;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_SHUTDOWN_RES);
        node = mxmlNewElement(MXML_NO_PARENT, "ShutdownResource");
        XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
        XMLNewElementText(node, "ResourceTicket", param->str_resTicket.c_str());
        char ca_isRelease[8];
        memset(ca_isRelease, 0, 8);
        sprintf(ca_isRelease, "%d", param->iIsRelease);
        XMLNewElementText(node, "IsRelease", ca_isRelease);
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("shutdownresource:%s",str);
        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("shutdownresource:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessLaunchCommon(const NETWORK *network, const char *logonTicket,
						const char *path, const char *name,
						const char *markname, const char *mark, const char *ticketname,
                        SELECTAPPLICATION *selectApp)
{
	char buffer[BUFFERLENGTH];
	int httpStatus, errorCode;
    mxml_node_t *node, *rdpNode, *tree = NULL;

	httpStatus = PostCommon(network, logonTicket, path, name, markname, mark, buffer);

	if (httpStatus < 0)
		return httpStatus;

#ifdef DEBUG_MODE
	printf("launch:\n%s\n", buffer);
#endif
	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
	errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
	if (errorCode < 0)
		return errorCode;

	selectApp->protocol = XMLFindIntElement(tree, tree, "Protocol");
    selectApp->bypassProtocol = XMLFindIntElement(tree, tree, "BypassProtocol");
    XMLFindStrElement(tree, tree, "BypassIP", selectApp->bypassIP, MAX_LEN);
    XMLFindStrElement(tree, tree, "BypassPort", selectApp->bypassPort, MAX_LEN);

	rdpNode = mxmlFindElement(tree, tree, "RdpParams", NULL, NULL, MXML_DESCEND);
	if (rdpNode != NULL)
	{
        XMLFindStrElement(rdpNode, tree, "HostName", selectApp->rdpParams.hostName, MAX_LEN);
        XMLFindStrElement(rdpNode, tree, "HostPort", selectApp->rdpParams.hostPort,	MAX_LEN);
        XMLFindBase64Element(rdpNode, tree, "ForceUsername", selectApp->rdpParams.forceUsername, MAX_LEN);
        XMLFindBase64Element(rdpNode, tree, "ForcePassword", selectApp->rdpParams.forcePassword, MAX_LEN);
        XMLFindBase64Element(rdpNode, tree, "ForceDomain", selectApp->rdpParams.forceDomain, MAX_LEN);
        XMLFindBase64Element(rdpNode, tree, "AlternateShell", selectApp->rdpParams.alternateShell, MAX_LEN);
        XMLFindBase64Element(rdpNode, tree, "WorkingDir", selectApp->rdpParams.workingDir, MAX_LEN);
		node = mxmlFindElement(rdpNode, tree, "Rail", NULL, NULL, MXML_DESCEND);
		if (node != NULL)
		{
			XMLFindBase64Element(node, tree, "ApplicationName",
                                selectApp->rdpParams.rail.applicationName, MAX_LEN);
			XMLFindBase64Element(node, tree, "WorkingDirectory",
                                selectApp->rdpParams.rail.workingDirectory, MAX_LEN);
			XMLFindBase64Element(node, tree, "Arguments",
                                selectApp->rdpParams.rail.arguments, MAX_LEN);
		}
        XMLFindStrElement(rdpNode, tree, "ColorDepth", selectApp->rdpParams.colorDepth, MIN_LEN);
        XMLFindStrElement(rdpNode, tree, "FullScreen", selectApp->rdpParams.fullScreen, MIN_LEN);
        XMLFindStrElement(rdpNode, tree, "Resolution", selectApp->rdpParams.resolution, MIN_LEN);
		selectApp->rdpParams.compression = XMLFindIntElement(rdpNode, tree, "Compression");
		selectApp->rdpParams.performance = XMLFindIntElement(rdpNode, tree, "Performance");
		selectApp->rdpParams.audio = XMLFindIntElement(rdpNode, tree, "Audio");
		selectApp->rdpParams.audioIn = XMLFindIntElement(rdpNode, tree, "AudioIn");
		selectApp->rdpParams.printer = XMLFindIntElement(rdpNode, tree, "Printer");
		selectApp->rdpParams.disk = XMLFindIntElement(rdpNode, tree, "Disk");
		selectApp->rdpParams.smartcard = XMLFindIntElement(rdpNode, tree, "Smartcard");
		selectApp->rdpParams.serialPort = XMLFindIntElement(rdpNode, tree, "SerialPort");
		selectApp->rdpParams.parallelPort = XMLFindIntElement(rdpNode, tree, "ParallelPort");
		selectApp->rdpParams.clipboard = XMLFindIntElement(rdpNode, tree, "Clipboard");
		for (node = mxmlFindElement(rdpNode, tree, "Usb", NULL, NULL, MXML_DESCEND);
				node != NULL;
				node = mxmlFindElement(node, tree, "Usb", NULL, NULL, MXML_DESCEND))
		{
			if (strlen(selectApp->rdpParams.usb) > 0)
			{
				strcat(selectApp->rdpParams.usb, " ");
				strcat(selectApp->rdpParams.usb, node->child->value.text.string);
			}
			else
				strcpy(selectApp->rdpParams.usb, node->child->value.text.string);
		}
	}
    XMLFindStrElement(tree, tree, ticketname, selectApp->ticket, MAX_LEN);
    mxmlDelete(tree);
	return errorCode;
}

int VAccessShutdownDesktopPool(const NETWORK *network, const char *logonTicket,
							   const char *desktopPoolTicket)
{
	return PostCommonWithErrorCode(network, logonTicket,
								   "/RestService/DesktopPool/ShutdownDesktopPool",
								   "shutdownDesktopPool", "DesktopPoolTicket", desktopPoolTicket);
}

int VAccessChangeUserInfo(ST_ACCESS_CHANGE_USER_INFO_IN* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_CHANGE_USER_INFO;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0;
        mxml_node_t *node = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_CHANGE_USER);
        node = mxmlNewElement(MXML_NO_PARENT, "UpdatePassword");
        XMLNewElementText(node, "LogonTicket", param->stAccountInfo.logonTicket);
        XMLNewElementText(node, "Username", base64_encode(param->stUserInfo.username));
        XMLNewElementText(node, "Password", base64_encode(param->strPasswd.c_str()));
        XMLNewElementText(node, "NewPassword", base64_encode(param->strNewPasswd.c_str()));
//        XMLNewElementText(node, "Domain", base64_encode(param->stUserInfo.domain));
//        XMLNewElementText(node, "NtUsername", base64_encode(param->strNtUserName.c_str()));
//        XMLNewElementText(node, "NtPassword", base64_encode(param->strNtPasswd.c_str()));
//        XMLNewElementText(node, "NtDomain", base64_encode(param->stAccountInfo.ntDomain));
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("changeuserinfo:%s", str);

        errorCode = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        errorCode = GetOnlyErrorCode(errorCode, buffer);

        mxml_node_t *tree = NULL;
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
        if (errorCode < 0)
        {
            LOG_ERR("errorCode:\t%d", errorCode);
        }
        else
        {
//            XMLFindStrElement(tree, tree, "Username", param->stUserInfo.username, MAX_LEN);
//            XMLFindStrElement(tree, tree, "Password", param->stUserInfo.password, MAX_LEN);
//            XMLFindStrElement(tree, tree, "Password", param->stUserInfo.domain, MAX_LEN);

//            XMLFindStrElement(tree, tree, "LogonTicket", param->stAccountInfo.logonTicket, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtUsername", param->stAccountInfo.ntUsername, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtPassword", param->stAccountInfo.ntPassword, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtDomain", param->stAccountInfo.ntDomain, MAX_LEN);

//            param->strNtPasswd = param->stAccountInfo.ntPassword;
//            param->strNtUserName = param->stAccountInfo.ntUsername;
//            param->strPasswd = param->stUserInfo.password;
//            strcpy(param->stAccountInfo.ntPassword, param->strNtPasswd.c_str());
//            strcpy(param->stAccountInfo.ntUsername, param->strNtUserName.c_str());
            strcpy(param->stUserInfo.password, param->strNewPasswd.c_str());
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessChangeNtUserInfo(ST_ACCESS_CHANGE_USER_INFO_IN* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_CHANGE_NTUSER_INFO;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0;
        mxml_node_t *node = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_CHANGE_NTUSER);
        node = mxmlNewElement(MXML_NO_PARENT, "User");
        XMLNewElementText(node, "LogonTicket", param->stAccountInfo.logonTicket);
        XMLNewElementText(node, "Username", base64_encode(param->stUserInfo.username));
        XMLNewElementText(node, "Password", base64_encode(param->strNewPasswd.c_str()));
        XMLNewElementText(node, "Domain", base64_encode(param->stUserInfo.domain));
        XMLNewElementText(node, "NtUsername", base64_encode(param->strNtUserName.c_str()));
        XMLNewElementText(node, "NtPassword", base64_encode(param->strNtPasswd.c_str()));
        XMLNewElementText(node, "NtDomain", base64_encode(param->stAccountInfo.ntDomain));
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("changeNtuserinfo:%s", str);

        errorCode = HttpPut(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        errorCode = GetOnlyErrorCode(errorCode, buffer);

        mxml_node_t *tree = NULL;
        tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
        errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
        mxmlDelete(tree);
        if (errorCode < 0)
        {
            LOG_ERR("errorCode:\t%d", errorCode);
        }
        else
        {
//            XMLFindStrElement(tree, tree, "Username", param->stUserInfo.username, MAX_LEN);
//            XMLFindStrElement(tree, tree, "Password", param->stUserInfo.password, MAX_LEN);
//            XMLFindStrElement(tree, tree, "Password", param->stUserInfo.domain, MAX_LEN);

//            XMLFindStrElement(tree, tree, "LogonTicket", param->stAccountInfo.logonTicket, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtUsername", param->stAccountInfo.ntUsername, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtPassword", param->stAccountInfo.ntPassword, MAX_LEN);
//            XMLFindStrElement(tree, tree, "NtDomain", param->stAccountInfo.ntDomain, MAX_LEN);

//            param->strNtPasswd = param->stAccountInfo.ntPassword;
//            param->strNtUserName = param->stAccountInfo.ntUsername;
//            param->strPasswd = param->stUserInfo.password;
            strcpy(param->stAccountInfo.ntPassword, param->strNtPasswd.c_str());
            strcpy(param->stAccountInfo.ntUsername, param->strNtUserName.c_str());
            strcpy(param->stUserInfo.password, param->strNewPasswd.c_str());
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}


int VAccessRequestDesktop(ST_ACCESS_REQUEST_DESKTOP* param)
{
   int errorCode = 0;
   if( NULL == param )
   {
       LOG_ERR("%s","parameter error: NULL == param");
       errorCode = -1;
   }
   else
   {
       param->stParamCommon.type = TYPE_REQUEST_DESKTOP;
       char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
       int len =0,httpStatus;
       mxml_node_t *node = NULL, *tree = NULL;
       char *url = NULL;
       memset(str, 0, BUFFERLENGTH);
       url = MakeUrl(&(param->stParamCommon.network),REQUEST_DESKTOP_INFO);

       node = mxmlNewElement(MXML_NO_PARENT, "RequestDesktop");
       XMLNewElementText(node,"LogonTicket",param->logonTicket);
       XMLNewElementText(node,"Cpu", base64_encode(param->Cpu.c_str()));
       XMLNewElementText(node,"Memory",base64_encode(param->Memory.c_str()));
       XMLNewElementText(node,"Os",base64_encode(param->Os.c_str()));
       XMLNewElementText(node,"Disk",base64_encode(param->Disk.c_str()));
       XMLNewElementText(node,"Discription",base64_encode(param->desktopDescrip.c_str()));

       mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

       LOG_INFO("requestdesktop:%s",str);

       httpStatus = HttpPost(url, str, buffer, &len);
       free(url);
       mxmlDelete(node);
       node = NULL;
       if (httpStatus < 0)
       {
           errorCode = httpStatus;
           LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
       }
       else
       {
           LOG_INFO("requestDesktop:%s",buffer);
           tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
           errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
           mxmlDelete(tree);
       }
   }
   if(NULL != param)
   {
       param->stParamCommon.iErrCode =errorCode;
       if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
       {
           (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
       }
   }
    return errorCode;
}

int VAccessPoweroffTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_POWEROFF_TERMINAL;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *terminal;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_TERMINAL_DESKTOP_CTL);

        node = mxmlNewElement(MXML_NO_PARENT, "ManageTerminals");
        XMLNewElementText(node,"LogonTicket",param->loginTicket);
        XMLNewElementText(node, "Action", "1");
//        XMLNewElementText(node, "Delay", "60");
        terminal = mxmlNewElement(node,"Terminals");
        for( unsigned int loop=0; loop < param->uuid.size(); loop++)
        {
            XMLNewElementText(terminal, "Uuid", param->uuid[loop].c_str());
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("terminaldesktop:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("terminalDesktopCtl:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;

}

int VAccessRestartTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_RESTART_TERMINAL;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *terminal;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_TERMINAL_DESKTOP_CTL);

        node = mxmlNewElement(MXML_NO_PARENT, "ManageTerminals");
        XMLNewElementText(node,"LogonTicket",param->loginTicket);
        XMLNewElementText(node, "Action", "2");
//        XMLNewElementText(node, "Delay", "60");
        terminal = mxmlNewElement(node,"Terminals");
        for(unsigned int loop=0; loop < param->uuid.size(); loop++)
        {
            XMLNewElementText(terminal, "Uuid", param->uuid[loop].c_str());
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("terminaldesktop:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("terminalDesktopCtl:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessMsgTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_MSG_TERMINAL;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *terminal = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_TERMINAL_DESKTOP_CTL);

        node = mxmlNewElement(MXML_NO_PARENT, "ManageTerminals");
        XMLNewElementText(node,"LogonTicket",param->loginTicket);
        XMLNewElementText(node, "Action", "3");
        XMLNewElementText(node,"Message",param->msg.c_str());
        terminal = mxmlNewElement(node,"Terminals");
        for(unsigned int loop=0; loop < param->uuid.size(); loop++)
        {
            XMLNewElementText(terminal, "Uuid", param->uuid[loop].c_str());
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("terminaldesktop:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("terminalDesktopCtl:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessStartDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_START_DESKTOPPOOL;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                               REQUEST_START_DESKPOOL, "StartDesktopPool", "DesktopPoolUuid",
                               param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            mxml_node_t *tree = NULL;
            LOG_INFO("startdesktoppool:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char jobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", jobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = jobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessRestartDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_RESTART_DESKTOPPOOL;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_RESTART_DESPOOL, "RestartDesktopPool", "DesktopPoolUuid",
                                param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            mxml_node_t *tree = NULL;
            LOG_INFO("restartdesktoppool:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char jobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", jobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = jobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessStopDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_STOP_DESKTOPPOOL;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_STOP_DESKPOOL, "StopDesktopPool", "DesktopPoolUuid",
                                param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            mxml_node_t *tree = NULL;
            LOG_INFO("stopdesktoppool:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char jobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", jobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = jobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessAttachVirtualDisk(ST_ACCESS_ATTACH_VDISK* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_ATTACH_DISK;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_ATTACH_VDISK, "AttachVirtualDisk", "DesktopPoolUuid",
                                param->str_desktopUuid.c_str(), buffer);

        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("attachvirtualdisk:%s", buffer);
            mxml_node_t *tree = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char jobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", jobId, MAX_LEN);
                param->stAttachVDisk.strJobId = jobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessAttachVirtualDiskToDesktop(ST_ACCESS_ATTACH_VDISK* param)
{
    int errorCode = 0, len;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_ATTACH_DISK;
        char buffer[BUFFERLENGTH], str[4096], ca_desktopType[4];
        mxml_node_t *node = NULL, *tree = NULL;
        char* url = MakeUrl(&(param->stParamCommon.network), REQUEST_ATTACH_VDISK_TODESKTOP);
        node = mxmlNewElement(MXML_NO_PARENT, "AttachVirtualDiskToDesktop");
        XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
        XMLNewElementText(node, "ResourceUuid", param->str_desktopUuid.c_str());
        sprintf(ca_desktopType, "%d", param->iDesktopType);
        XMLNewElementText(node, "ResourceType", ca_desktopType);
        XMLNewElementText(node, "VirtualDisk", param->devicePath.c_str());
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);
        LOG_INFO("VAccessAttachVirtualDiskToDesktop:%s", str);

        int httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;

        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("VAccessAttachVirtualDiskToDesktop:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char jobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", jobId, MAX_LEN);
                param->stAttachVDisk.strJobId = jobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessCheckDesktopPoolState(ST_ACCESS_CHECK_DESK_STATE* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TPPE_CHECK_DESKTOPPOOL_STATE;
        char buffer[BUFFERLENGTH];
        int httpStatus = 0;
        mxml_node_t *tree = NULL;
        LOG_INFO("sessionticket:%s\t\t uuid:%s", param->str_SessionTicket.c_str(), param->str_deskUuid.c_str());
        httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_DESKPOOL_STATE, "DesktopPoolState",	"DesktopPoolUuid",
                                param->str_deskUuid.c_str(), buffer);

        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("desktoppoolstate:%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                param->stCheckDeskState.iState = XMLFindIntElement(tree, tree, "State");
                char ip[MAX_LEN];
                memset(ip, 0, MAX_LEN);
                XMLFindStrElement(tree, tree, "Ip", ip, MAX_LEN);
                param->stCheckDeskState.strIp = ip;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessQueryAsyncJobResult(ST_ACCESS_ASYN_QUERY* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_QUERY_ASYN_JOB_RESULT;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_ASYN_QUERY, "QueryAsyncJobResult", "JobId",
                                param->strJobId.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            mxml_node_t *tree = NULL;
            LOG_INFO("queryasyncjobresult:\t%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                param->stAsynQuery.iJobStatus = XMLFindIntElement(tree, tree, "JobStatus");
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessOpenChannel(ST_CHANNEL_OP* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_OPEN_CHANNEL;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0, httpStatus = 0;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_OPEN_CHANNEL);
        node = mxmlNewElement(MXML_NO_PARENT, "OpenChannel");
        XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
        XMLNewElementText(node, "Ip", param->str_ip.c_str());
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("openchannel begin:\t%s",str);
        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;

        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("openchannel end:\t%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                param->st_channel_op.iPort = XMLFindIntElement(tree, tree, "Port");
                char chIp[MAX_LEN];
                memset(chIp, 0, MAX_LEN);
                XMLFindStrElement(tree, tree, "Ip", chIp, MAX_LEN);
                param->st_channel_op.str_ip = chIp;
                memset(param->st_channel_op.usb_Uuid, 0, MAX_LEN);
                XMLFindStrElement(tree, tree, "ChannelUuid", param->st_channel_op.usb_Uuid, MAX_LEN);
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessCloseChannel(ST_CHANNEL_OP* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_CLOSE_CHANNEL;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0, httpStatus = 0;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_CLOSE_CHANNEL);
        node = mxmlNewElement(MXML_NO_PARENT, "CloseChannel");
        XMLNewElementText(node, "LogonTicket", param->str_SessionTicket.c_str());
        XMLNewElementText(node, "Ip", param->str_ip.c_str());
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
        LOG_INFO("closechannel begin:\t", str);
        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;

        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("closechannel end:\t", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                param->st_channel_op.iPort = XMLFindIntElement(tree, tree, "Port");
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessAuthToken(ST_AUTH_TOKEN* param)
{
    int errorCode = 0;
    if(NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param ");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_AUTH_TOKEN;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len = 0, httpStatus = 0;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);

        //messy code ;
        memset(param->stLoginData.stLoginInfo.logonTicket, 0, MAX_LEN);
        memset(param->stLoginData.stLoginInfo.ntUsername, 0, MAX_LEN);
        memset(param->stLoginData.stLoginInfo.ntPassword, 0, MAX_LEN);
        memset(param->stLoginData.stLoginInfo.ntDomain, 0, MAX_LEN);

        strcpy(param->stParamCommon.network.stPresentServer.serverAddress, param->stParamCommon.network.stFirstServer.serverAddress);//strcpy(param->stParamCommon.network.presentServer, param->stParamCommon.network.firstServer);
        strcpy(param->stParamCommon.network.stPresentServer.port, param->stParamCommon.network.stFirstServer.port);
        param->stParamCommon.network.stPresentServer.isHttps = param->stParamCommon.network.stFirstServer.isHttps;
        url = MakeUrl(&(param->stParamCommon.network), REQUEST_AUTH_TOKEN);
        node = mxmlNewElement(MXML_NO_PARENT, "TokenAuth");
        XMLNewElementText(node, "Username", base64_encode(param->stUserInfo.username));
        XMLNewElementText(node, "Password", base64_encode(param->stUserInfo.password));
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);//str = mxmlSaveAllocString(node, MXML_NO_CALLBACK);

        LOG_INFO("AuthToken begin:%s, url:%s", str, url);
        httpStatus = HttpPost(url, str, buffer, &len);
        LOG_INFO("AuthToken end:%s", buffer);
        if(httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
            if(strlen(param->stParamCommon.network.stAlternateServer.serverAddress)>0 && strlen(param->stParamCommon.network.stAlternateServer.port)>0)//if(strlen(param->stParamCommon.network.alternateServer)>0)
            {
                free(url);
                url = NULL;
                strcpy(param->stParamCommon.network.stPresentServer.serverAddress, param->stParamCommon.network.stAlternateServer.serverAddress);//strcpy(param->stParamCommon.network.presentServer, param->stParamCommon.network.alternateServer);
                strcpy(param->stParamCommon.network.stPresentServer.port, param->stParamCommon.network.stAlternateServer.port);
                param->stParamCommon.network.stPresentServer.isHttps = param->stParamCommon.network.stAlternateServer.isHttps;
                url = MakeUrl(&(param->stParamCommon.network), REQUEST_AUTH_TOKEN);
                LOG_INFO("AuthToken begin:%s, url:%s", str, url);
                httpStatus = HttpPost(url, str, buffer, &len);
                LOG_INFO("AuthToken end:%s", buffer);
            }
        }
        free(url);
        url = NULL;
        mxmlDelete(node);
        node = NULL;
        if(httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            param->stLoginData.stNetwork = param->stParamCommon.network;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            LOG_INFO("AuthToken begin:%s", buffer);
            cout << "AuthTokenbuffer:" << buffer << endl;
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                XMLFindStrElement(tree, tree, "LogonTicket", param->stLoginData.stLoginInfo.logonTicket, MAX_LEN);
                XMLFindBase64Element(tree, tree, "NtUsername", param->stLoginData.stLoginInfo.ntUsername, MAX_LEN);
                XMLFindBase64Element(tree, tree, "NtPassword", param->stLoginData.stLoginInfo.ntPassword, MAX_LEN);
                XMLFindBase64Element(tree, tree, "NtDomain", param->stLoginData.stLoginInfo.ntDomain, MAX_LEN);
                cout << "vaccess :NtDomain" << param->stLoginData.stLoginInfo.ntDomain << endl;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;

}

int VAccessPowerOnDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_POWERON_DESKTOP;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                               REQUEST_POWER_DESK, "PowerOnDesktop", "DesktopUuid", param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("PowerOnDesktop:\t%s", buffer);
            mxml_node_t *tree = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char chJobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", chJobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = chJobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;

}
int VAccessRebootDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_REBOOT_DESKTOP;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                REQUEST_REBOOT_DESK, "RebootDesktop", "DesktopUuid", param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("RebootDesktop:\t%s", buffer);
            mxml_node_t *tree = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char chJobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", chJobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = chJobId;
            }
            mxmlDelete(tree);
        }
    }

    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessPowerOffDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param)
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_POWEROFF_DESKTOP;
        char buffer[BUFFERLENGTH];
        int httpStatus, errorCode;
        mxml_node_t *tree = NULL;

        httpStatus= PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                               REQUEST_POWEROFF_DESK, "PowerOffDesktop", "DesktopUuid", param->strUuid.c_str(), buffer);
        if (httpStatus < 0)
            return httpStatus;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("PowerOffDesktop:\t%s", buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                char chJobId[MAX_LEN];
                XMLFindStrElement(tree, tree, "JobId", chJobId, MAX_LEN);
                param->stDeskControlJobId.strJobId = chJobId;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
	return errorCode;
}

int VAccessRemoteDesktopState(ST_ACCESS_CHECK_DESK_STATE* param)
                              /*const NETWORK *network,
							  const char *logonTicket, const char *remoteDesktopUuid,
                              int *rdpState, int *vmState,char *ip)*/
{
    int errorCode = 0;
    if(NULL == param || (NULL!=param && param->str_SessionTicket.size()<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->str_SessionTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_CHECK_REMOTEDESKTOP_STATE;
        char buffer[BUFFERLENGTH];
        int httpStatus = PostCommon(&(param->stParamCommon.network), param->str_SessionTicket.c_str(),
                                    REQUEST_REMOTEDEST_STATE, "RemoteDesktopState", "DesktopUuid",
                                param->str_deskUuid.c_str(), buffer);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("postcommon failed %d", httpStatus);
        }
        else
        {
            LOG_INFO("RemoteDesktopState:%s", buffer);
            mxml_node_t *tree = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if (errorCode >= 0)
            {
                param->stCheckDeskState.iState = XMLFindIntElement(tree, tree, "State");
                param->stCheckDeskState.iRdpState = XMLFindIntElement(tree, tree, "RdpServiceState");
                char ip[MAX_LEN];
                memset(ip, 0, MAX_LEN);
                XMLFindStrElement(tree, tree, "HostName", ip, MAX_LEN);
                param->stCheckDeskState.strIp = ip;
            }
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }

	return errorCode;
}

int VAccessGetMonitorsInfo(ST_ACCESS_GET_MONITORSINFO *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_GET_MONITORSINFO;
        const char* logonTicket = param->logonTicket;
        NETWORK* network = &(param->stParamCommon.network);
        char buffer[BUFFERLENGTH*2];
        int len = 0, httpStatus = 0;
        char *str = NULL, *url = NULL;


        url = MakeUrl(network, REQUEST_GET_MONITORSINFO);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "<GetMonitorsInfo><LogonTicket>%s</LogonTicket></GetMonitorsInfo>", logonTicket);
        cout << "logonTicket: +++" << str << endl;
        httpStatus = HttpPost(url, str, buffer, &len);

        free(url);
        free(str);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpGet failed! return value:%d", httpStatus);
        }
        else
        {
            LOG_DEBUG("VAccessGetMonitorsInfo:%s", buffer);
            cout << "monitorsinfo :+++++++++++++++++" << buffer << endl;
            mxml_node_t *tree = NULL, *node = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            param->stListMonitorsInfo.errorcode = errorCode;
            if (errorCode >= 0)
            {
                param->stListMonitorsInfo.monitorNum = XMLFindIntElement(tree, tree, "MonitorNum");
                if(param->stListMonitorsInfo.monitorNum > 0)
                {
                    for (node = mxmlFindElement(tree, tree, "MonitorInfo", NULL, NULL, MXML_DESCEND);
                            node != NULL;
                            node = mxmlFindElement(node, tree, "MonitorInfo", NULL, NULL, MXML_DESCEND))
                    {
                        MONITOR_INFO monitorInfo;
                        memset(&monitorInfo, 0, sizeof(MONITOR_INFO));

                        monitorInfo.monitor = XMLFindIntElement(node, node, "Monitor");
                        monitorInfo.status = XMLFindIntElement(node, node, "Status");
//                        if( monitorInfo.status > 0)
                        {
                            monitorInfo.mode = XMLFindIntElement(node, node, "Mode");
                            XMLFindBase64Element(node, node, "Username", monitorInfo.username, MAX_LEN);
                            XMLFindBase64Element(node, node, "Domain", monitorInfo.domain, MAX_LEN);
                            param->stListMonitorsInfo.stMonitorInfoList.push_back(monitorInfo);
                        }
                    }
                }else{
                    param->stListMonitorsInfo.monitorNum = 0;
                }
            }
            else
                LOG_ERR("monitorinforlist:\t%s", buffer);
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessConnectMonitor(ST_ACCESS_CONNECT_MONITOR* param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_CONNECT_MONITOR;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        char s_monitor[5]= {0}, s_mode[5] = {0};
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_CONNECT_MONITOR);

        node = mxmlNewElement(MXML_NO_PARENT, "ConnectMonitor");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        sprintf(s_monitor, "%d", param->monitor);
        sprintf(s_mode, "%d", param->mode);
        XMLNewElementText(node, "Monitor", s_monitor);
        XMLNewElementText(node, "Mode", s_mode);
        XMLNewElementText(node, "ToMonitorIp", param->toMonitorIp);


        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("connectMonitor:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("connectMonitor:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessDisConnectMonitor(ST_ACCESS_DISCONNECT_MONITOR *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_DISCONNECT_MONITOR;
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        char s_monitor[5] = {0};
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DISCONNECT_MONITOR);

        node = mxmlNewElement(MXML_NO_PARENT, "DisconnectMonitor");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        sprintf(s_monitor, "%d", param->monitor);
        XMLNewElementText(node, "Monitor", s_monitor);


        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("disconnectMonitor:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("disconnectMonitor:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessgetUserOrganizations(ST_ACCESS_GET_USERORGANIZATIONS *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        const char* logonTicket = param->logonTicket;
        NETWORK* network = &(param->stParamCommon.network);
        char buffer[BUFFERLENGTH*2];
        int len = 0, httpStatus = 0;
        char *str = NULL, *url = NULL;
        url = MakeUrl(network, REQUEST_GET_USERORGANIZATIONS);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "<GetUserOrganizations><LogonTicket>%s</LogonTicket></GetUserOrganizations>", logonTicket);
        httpStatus = HttpPost(url, str, buffer, &len);

        free(url);
        free(str);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpGet failed! return value:%d", httpStatus);
        }
        else
        {
            LOG_DEBUG("VAccessgetUserOrganizations: %s", buffer);
            cout << "================" << buffer << endl;
            mxml_node_t *tree = NULL, *node = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if(errorCode >= 0)
            {
                for(node = mxmlFindElement(tree, tree, "Organization", NULL, NULL, MXML_DESCEND); node != NULL;
                    node = mxmlFindElement(node, tree, "Organization", NULL, NULL, MXML_DESCEND))
                {
                    ORGANIZATION organization;
                    memset(&organization, 0, sizeof(ORGANIZATION));
                    XMLFindStrElement(node, node, "Id", organization.id, MAX_LEN);
                    XMLFindBase64Element(node, node, "Name", organization.name, MAX_LEN);
                    XMLFindBase64Element(node, node, "Description", organization.description, MAX_LEN);
                    XMLFindStrElement(node, node, "UniqueName", organization.uniqueName, MAX_LEN);
                    param->userorganizations.organizations.push_back(organization);
                }
            }
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessaddOrganization(ST_ACCESS_ADD_ORGANIZATION *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_ADD_ORGANIZATION);

        node = mxmlNewElement(MXML_NO_PARENT, "AddOrganization");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        if(strlen(param->parentUniqueName))
        {
            cout << "param->parentUniquename" << param->parentUniqueName << endl;
            XMLNewElementText(node, "ParentUniqueName", param->parentUniqueName);
        }
        XMLNewElementText(node, "Name",base64_encode(param->name));
        if(strlen(param->description))
        {
            XMLNewElementText(node, "Description", base64_encode(param->description));
        }


        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("addorganization:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("addorganization:%s",buffer);
            cout << "addorganization:" << buffer << endl;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            XMLFindStrElement(tree, tree, "Id", param->addorganizationData.id, MAX_LEN);
            XMLFindStrElement(tree, tree, "UniqueName", param->addorganizationData.uniquename, MAX_LEN);
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessdeleteOrganization(ST_ACCESS_DELETE_ORGANIZATION *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DELETE_ORGANIZATION);

        node = mxmlNewElement(MXML_NO_PARENT, "DeleteOrganization");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "UniqueName", param->uniqueName);


        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("deleteorganization:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("deleteorganization:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessupdateOrganization(ST_ACCESS_UPDATE_ORGANIZATION *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_UPDATE_ORGANIZATION);

        node = mxmlNewElement(MXML_NO_PARENT, "UpdateOrganization");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        if(strlen(param->parentUniqueName))
        {
            XMLNewElementText(node, "ParentUniqueName", param->parentUniqueName);
        }
        XMLNewElementText(node, "Name",base64_encode(param->name));
        if(strlen(param->description))
        {
            XMLNewElementText(node, "Description", base64_encode(param->description));
        }
        XMLNewElementText(node, "UniqueName", param->uniqueName);



        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("updateorganization:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("updateorganization:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessmoveOrganizationUsers(ST_ACCESS_MOVE_ORGANIZATION_USERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *users = NULL, *user = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_MOVE_ORGANIZATIONUSERS);

        node = mxmlNewElement(MXML_NO_PARENT, "MoveOrganizationUsers");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "DeleteOld", param->deleteOld);
        XMLNewElementText(node, "NewUniqueName",param->newUniqueName);
        XMLNewElementText(node, "OldUniqueName", param->oldUniqueName);
        users = mxmlNewElement(node, "Users");
        for(unsigned int loop = 0; loop < param->users.size(); loop++)
        {
            user = mxmlNewElement(users, "User");
            XMLNewElementText(user, "Username", base64_encode(param->users[loop].username));
            if(strlen(param->users[loop].domain))
            {
                XMLNewElementText(user, "Domain", base64_encode(param->users[loop].domain));
            }
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("moveorganizationusers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(user);
        user = NULL;
        mxmlDelete(users);
        users = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("moveorganizationusers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessaddOrganizationUsers(ST_ACCESS_ADD_ORGANIZATION_USERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *users = NULL, *user = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_ADD_ORGANIZATIONUSERS);

        node = mxmlNewElement(MXML_NO_PARENT, "AddOrganizationUsers");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "UniqueName", param->uniqueName);
        users = mxmlNewElement(node, "Users");
        for(unsigned int loop = 0; loop < param->users.size(); loop++)
        {
            user = mxmlNewElement(users, "User");
            XMLNewElementText(user, "Username",base64_encode(param->users[loop].username));
            if(strlen(param->users[loop].domain))
            {
                XMLNewElementText(user, "Domain", base64_encode(param->users[loop].domain));
            }
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("addorganizationusers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        if(user != NULL){
            mxmlDelete(user);
            user = NULL;
        }
        if(users != NULL)
        {
            mxmlDelete(users);
            users = NULL;
        }
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("addorganizationusers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessdeleteOrganizationUsers(ST_ACCESS_DELETE_ORGANIZATION_USERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *users = NULL, *user = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DELETE_ORGANIZATIONUSERS);

        node = mxmlNewElement(MXML_NO_PARENT, "DeleteOrganizationUsers");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "UniqueName",param->uniqueName);
        users = mxmlNewElement(node, "Users");
        for(unsigned int loop = 0; loop < param->users.size(); loop++)
        {
            user = mxmlNewElement(users, "User");
            XMLNewElementText(user, "Username", base64_encode(param->users[loop].username));
            if(strlen(param->users[loop].domain))
            {
                XMLNewElementText(user, "Domain", base64_encode(param->users[loop].domain));
            }
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("deleteorganizationusers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
		if(user != NULL){
	        mxmlDelete(user);	
	        user = NULL;	
		}
		if(users != NULL){
			mxmlDelete(users);
			users = NULL;
		}
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
		{
            LOG_INFO("deleteorganizationusers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessgetOrganizationDetail(ST_ACCESS_GET_ORGANIZATION_DETAIL *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        const char* logonTicket = param->logonTicket;
        NETWORK* network = &(param->stParamCommon.network);
        char buffer[BUFFERLENGTH*2];
        int len = 0, httpStatus = 0;
        char *str = NULL, *url = NULL;
        url = MakeUrl(network, REQUEST_GET_ORGANIZATIONDETAIL);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "<GetOrganizationDetail><LogonTicket>%s</LogonTicket><UniqueName>%s</UniqueName></GetOrganizationDetail>", logonTicket, param->uniqueName);
        httpStatus = HttpPost(url, str, buffer, &len);

        free(url);
        free(str);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpGet failed! return value:%d", httpStatus);
        }
        else
        {
            LOG_DEBUG("VAccessgetOrganizationDetail: %s", buffer);
            cout << "VAccessgetOrganizationDetail" << buffer << endl;
            mxml_node_t *tree = NULL, *node = NULL, *users = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if(errorCode >= 0)
            {
                for(node = mxmlFindElement(tree, tree, "Organization", NULL, NULL, MXML_DESCEND); node != NULL;
                    node = mxmlFindElement(node, tree, "Organization", NULL, NULL, MXML_DESCEND))
                {
                    ORGANIZATION organization;
                    memset(&organization, 0, sizeof(ORGANIZATION));
                    XMLFindStrElement(node, node, "Id", organization.id, MAX_LEN);
                    XMLFindBase64Element(node, node, "Name", organization.name, MAX_LEN);
                    cout << "Organization:::Name:::::" << organization.name <<endl;
                    XMLFindBase64Element(node, node, "Description", organization.description, MAX_LEN);
                    XMLFindStrElement(node, node, "UniqueName", organization.uniqueName, MAX_LEN);

                    for(users = mxmlFindElement(node, node, "User", NULL, NULL, MXML_DESCEND); users != NULL;
                        users = mxmlFindElement(users, node, "User", NULL, NULL, MXML_DESCEND) )
                    {
                        Userinfo user;
                        memset(&user, 0, sizeof(Userinfo));
                        XMLFindBase64Element(users, users, "Username", user.username, MAX_LEN);
                        cout << "username::::::" << user.username <<  endl;
                        XMLFindBase64Element(users, users, "Domain", user.domain, MAX_LEN);
                        XMLFindStrElement(users, users, "Ip", user.ip, MAX_LEN);
                        XMLFindBase64Element(users, users, "Role", user.role, MAX_LEN);
                        XMLFindStrElement(users, users, "SeatNumber", user.seatnumber, MAX_LEN);
                        organization.users.push_back(user);
                    }
                    param->organizationDetail.organizationDetail.push_back(organization);
                }
            }
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessgetRoles(ST_ACCESS_GET_ROLES *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        const char* logonTicket = param->logonTicket;
        NETWORK* network = &(param->stParamCommon.network);
        char buffer[BUFFERLENGTH*2];
        int len = 0, httpStatus = 0;
        char *str = NULL, *url = NULL;
        url = MakeUrl(network, REQUEST_GET_ROLES);
        str = (char *)malloc(MAX_LEN);
        sprintf(str, "<GetRoles><LogonTicket>%s</LogonTicket></GetRoles>", logonTicket);
        httpStatus = HttpPost(url, str, buffer, &len);

        free(url);
        free(str);
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpGet failed! return value:%d", httpStatus);
        }
        else
        {
            LOG_DEBUG("VAccessgetRoles: %s", buffer);
            cout << "getroles+++++++++++++++++++" << buffer << endl;
            mxml_node_t *tree = NULL, *node = NULL;
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            if(errorCode >= 0)
            {
                for(node = mxmlFindElement(tree, tree, "Role", NULL, NULL, MXML_DESCEND); node != NULL;
                    node = mxmlFindElement(node, tree, "Role", NULL, NULL, MXML_DESCEND))
                {
                    ROLE role;
                    memset(&role, 0, sizeof(ROLE));
                    XMLFindStrElement(node, node, "Id", role.id, MAX_LEN);
                    XMLFindBase64Element(node, node, "Name", role.name, MAX_LEN);
                    XMLFindBase64Element(node, node, "Description", role.description, MAX_LEN);
                    XMLFindStrElement(node, node, "Weight", role.weight, MAX_LEN);
                    param->roles.roles.push_back(role);
                }
            }
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode = errorCode;
        if(param->stParamCommon.bIsBlocked==UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param, param->stParamCommon.type);
        }
    }
    return errorCode;
}

int VAccessaddRole(ST_ACCESS_ADD_ROLE *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_ADD_ROLE);

        node = mxmlNewElement(MXML_NO_PARENT, "AddRole");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "RoleName", base64_encode(param->roleName));
        XMLNewElementText(node, "Weight",param->weight);
        if(strlen(param->description))
        {
            XMLNewElementText(node, "Description", base64_encode(param->description));
        }


        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("addRole:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("addRole:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            XMLFindStrElement(tree, tree, "Id", param->addroleData.id, MAX_LEN);
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessdeleteRole(ST_ACCESS_DELETE_ROLE *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DELETE_ROLE);

        node = mxmlNewElement(MXML_NO_PARENT, "DeleteRole");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "RoleName", base64_encode(param->roleName));

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("deleteRole:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("deleteRole:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessupdateRole(ST_ACCESS_UPDATE_ROLE *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_UPDATE_ROLE);

        node = mxmlNewElement(MXML_NO_PARENT, "UpdateRole");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "RoleName", base64_encode(param->roleName));
        XMLNewElementText(node, "NewRoleName", base64_encode(param->newRoleName));
        if(strlen(param->description))
        {
            XMLNewElementText(node, "Description", base64_encode(param->description));
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("updateRole:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("updateRole:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessaddroletousers(ST_ACCESS_ADDROLE_TOUSERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *users = NULL, *user = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_ADDROLE_TOUSERS);

        node = mxmlNewElement(MXML_NO_PARENT, "AddRoleToUsers");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "RoleName", base64_encode(param->roleName));
        users = mxmlNewElement(node, "Users");
        for(unsigned int loop = 0; loop < param->users.size(); loop++)
        {
            user = mxmlNewElement(users, "User");
            XMLNewElementText(user, "Username", base64_encode(param->users[loop].username));
            if(strlen(param->users[loop].domain))
            {
                XMLNewElementText(user, "Domain", base64_encode(param->users[loop].domain));
            }
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("addroletousers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(user);
        user = NULL;
        mxmlDelete(users);
        users = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("addrolerousers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessdeleterolefromusers(ST_ACCESS_DELETEROLE_FROMUSERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *users = NULL, *user = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DELETEROLE_FROMUSERS);

        node = mxmlNewElement(MXML_NO_PARENT, "DeleteRoleFromUsers");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "RoleName", base64_encode(param->roleName));
        users = mxmlNewElement(node, "Users");
        for(unsigned int loop = 0; loop < param->users.size(); loop++)
        {
            user = mxmlNewElement(users, "User");
            XMLNewElementText(user, "Username", base64_encode(param->users[loop].username));
            if(strlen(param->users[loop].domain))
            {
                XMLNewElementText(user, "Domain", base64_encode(param->users[loop].domain));
            }
        }

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("deleterolefromusers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(user);
        user = NULL;
        mxmlDelete(users);
        users = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("deleteroleusers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessgetuserprivileges(ST_ACCESS_GET_USERPRIVILEGES_PARAM *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *privileges = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_GET_USERPRIVILEGES);

        node = mxmlNewElement(MXML_NO_PARENT, "GetUserPrivileges");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        if(strlen(param->userName)){
          XMLNewElementText(node, "Username", base64_encode(param->userName));
        }
        if(strlen(param->domain)){
            XMLNewElementText(node, "Domain", base64_encode(param->domain));
        }

        XMLNewElementText(node, "Target", param->target);
        XMLNewElementText(node, "TargetId", param->targetId);

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("GetuserPrivileges:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("GetUserPrivileges:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            for( privileges = mxmlFindElement(tree, tree, "Privilege", NULL, NULL, MXML_DESCEND); privileges != NULL;
                 privileges = mxmlFindElement(privileges, tree, "Privilege", NULL, NULL, MXML_DESCEND))
            {
                USERPRIVILEGESDATA privilegeData;
                XMLFindStrElement(privileges, privileges, "Action", privilegeData.action, MAX_LEN);
                XMLFindStrElement(privileges, privileges, "TransMission", privilegeData.transmission,MAX_LEN);
                param->userprivilegesData.privileges.push_back(privilegeData);
            }
        }
        mxmlDelete(privileges);
        mxmlDelete(tree);
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessgetprivileges(ST_ACCESS_GET_PRIVILEGES_PARAM *param)
{
    int errorCode = 0;
    if(NULL == param ||(NULL!=param && strlen(param->logonTicket)<=0))
    {
        LOG_ERR("%s","parameter error: NULL == param || param->logonTicket.size()<=0");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *privileges = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_GET_PRIVILEGES);

        node = mxmlNewElement(MXML_NO_PARENT, "GetPrivileges");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "Owner", param->owner);
        XMLNewElementText(node, "OwnerId", param->ownerId);
        XMLNewElementText(node, "Target", param->target);
        XMLNewElementText(node, "TargetId", param->targetId);

        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("GetPrivileges:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("GetPrivileges:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            for( privileges = mxmlFindElement(tree, tree, "Privilege", NULL, NULL, MXML_DESCEND); privileges != NULL;
                 privileges = mxmlFindElement(privileges, tree, "Privilege", NULL, NULL, MXML_DESCEND))
            {
                USERPRIVILEGESDATA privilegeData;
                XMLFindStrElement(privileges, privileges, "Action", privilegeData.action, MAX_LEN);
                XMLFindStrElement(privileges, privileges, "TransMission", privilegeData.transmission,MAX_LEN);
                param->privilegesData.privileges.push_back(privilegeData);
            }
        }
        mxmlDelete(privileges);
        mxmlDelete(tree);
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessaddprivileges(ST_ACCESS_ADD_PRIVILEGES *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *privileges = NULL, *privilege = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_ADD_PRIVILEGES);

        node = mxmlNewElement(MXML_NO_PARENT, "AddPrivileges");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "Owner", param->owner);
        XMLNewElementText(node, "OwnerId",param->ownerId);
        XMLNewElementText(node, "Target", param->target);
        XMLNewElementText(node, "TargetId", param->targetId);
        privileges = mxmlNewElement(node, "Privileges");
        for(unsigned int loop = 0; loop < param->privileges.size(); loop++)
        {
            privilege = mxmlNewElement(privileges, "Privilege");
            XMLNewElementText(privilege,"Action", param->privileges[loop].action);
            XMLNewElementText(privilege, "Transmission", param->privileges[loop].transmission);
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("addPrivileges:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(privilege);
        privilege = NULL;
        mxmlDelete(privileges);
        privileges = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("addPrivileges:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessdeleteprivileges(ST_ACCESS_DELETE_PRIVILEGES *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *privileges = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_DELETE_PRIVILEGES);

        node = mxmlNewElement(MXML_NO_PARENT, "DeletePrivileges");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        XMLNewElementText(node, "Owner", param->owner);
        XMLNewElementText(node, "OwnerId",param->ownerId);
        XMLNewElementText(node, "Target", param->target);
        XMLNewElementText(node, "TargetId", param->targetId);
        privileges = mxmlNewElement(node, "Privileges");
        for(unsigned int loop = 0; loop < param->privileges.size(); loop++)
        {
            XMLNewElementText(privileges,"Action", param->privileges[loop].action);
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("deletePrivileges:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(privileges);
        privileges = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("deletePrivileges:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessSetSeatNumbers(ST_ACCESS_SETSEATNUMBERS *param)
{
    int errorCode = 0;
    if( NULL == param )
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        char buffer[BUFFERLENGTH], str[BUFFERLENGTH];
        int len =0,httpStatus;
        mxml_node_t *node = NULL, *tree = NULL, *seatNumbers = NULL, *seatnumber = NULL;
        char *url = NULL;
        memset(str, 0, BUFFERLENGTH);
        url = MakeUrl(&(param->stParamCommon.network),REQUEST_SET_SEATNUMBERS);

        node = mxmlNewElement(MXML_NO_PARENT, "SetSeatNumber");
        XMLNewElementText(node,"LogonTicket",param->logonTicket);
        seatNumbers = mxmlNewElement(node, "SeatNumbers");
        for(unsigned int loop = 0; loop < param->seatList.size(); loop++)
        {
            seatnumber = mxmlNewElement(seatNumbers, "SeatNumber");
            XMLNewElementText(seatnumber, "DesktopName", base64_encode(param->seatList[loop].desktopname));
            XMLNewElementText(seatnumber, "Number", param->seatList[loop].number);
        }
        mxmlSaveString(node, str, 4096, MXML_NO_CALLBACK);

        LOG_INFO("setseatnumbers:%s",str);

        httpStatus = HttpPost(url, str, buffer, &len);
        free(url);
        mxmlDelete(seatnumber);
        seatnumber = NULL;
        mxmlDelete(seatNumbers);
        seatNumbers = NULL;
        mxmlDelete(node);
        node = NULL;
        if (httpStatus < 0)
        {
            errorCode = httpStatus;
            LOG_ERR("httpPost failed. return vaule:%d", httpStatus);
        }
        else
        {
            LOG_INFO("setseatnumbers:%s",buffer);
            tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
            errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
            mxmlDelete(tree);
        }
    }
    if(NULL != param)
    {
        param->stParamCommon.iErrCode =errorCode;
        if(param->stParamCommon.bIsBlocked == UNBLOCK && NULL != param->stParamCommon.callback)
        {
            (*(param->stParamCommon.callback))(param,param->stParamCommon.type);
        }
    }
     return errorCode;
}

int VAccessTestConnectState(ST_TEST_CONNECTION_STATE* param)
{
    int errorCode = 0;
    if(NULL == param)
    {
        LOG_ERR("%s","parameter error: NULL == param");
        errorCode = -1;
    }
    else
    {
        param->stParamCommon.type = TYPE_OPEN_CHANNEL;
        char buffer[BUFFERLENGTH];
        int len = 0;
        char *url = NULL;

        url = MakeUrl(&(param->stParamCommon.network), REQUEST_TEST_CONNECTION_STATE);
        LOG_INFO("TestConnectState begin:\t%s",url);
        errorCode = HttpGet(url, "", buffer, &len);
        free(url);

        if (errorCode < 0)
        {
            LOG_ERR("postcommon failed %d", errorCode);
        }
        else
        {
            LOG_INFO("TestConnectState end:\t%s", buffer);
        }
    }
    return errorCode;
}

//////////////////////////////////////////////////////////////////////
//int VAccessGetApplicationList(const NETWORK *network, const char *logonTicket,
//		VAccessApplication *apps, int *appCount)
//{
//	char buffer[BUFFERLENGTH];
//	int len, httpStatus, errorCode;
//    mxml_node_t *tree = NULL, *node;
//	char *str, *url;
//	VAccessApplication *app;
//	int i = 0;

//	url = MakeUrl(network, "/RestService/App/ApplicationList");
//    str = (char *)malloc(MAX_LEN);
//	sprintf(str, "LogonTicket=%s", logonTicket);
//#ifdef DEBUG_MODE
//	printf("getapplicationlist = \n%s\n", str);
//#endif
//	httpStatus = HttpGet(url, str, buffer, &len);
//	free(url);
//	free(str);

//	if (httpStatus < 0)
//		return httpStatus;

//#ifdef DEBUG_MODE
//	printf("applicationlist: \n%s\n", buffer);
//#endif
//	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
//	errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
//	if (errorCode < 0)
//		return errorCode;

//	for (node = mxmlFindElement(tree, tree, "Application", NULL, NULL, MXML_DESCEND);
//			node != NULL;
//			node = mxmlFindElement(node, tree, "Application", NULL, NULL, MXML_DESCEND))
//	{
//		app = (VAccessApplication *)calloc(1, sizeof(VAccessApplication));
//		app->type = XMLFindIntElement(node, tree, "AppType");
//        XMLFindStrElement(node, tree, "AppUuid", app->uuid, MAX_LEN);
//        XMLFindBase64Element(node, tree, "AppName", app->name, MAX_LEN);
//        XMLFindBase64Element(node, tree, "AppDescription", app->description, MAX_LEN);
//        XMLFindBase64Element(node, tree, "HostDescription", app->hostDescription, MAX_LEN);

//		memcpy(&apps[i], app, sizeof(VAccessApplication));
//		if (apps[i].type == 1)
//			VAccessGetResourceParameters(network, logonTicket, apps[i].uuid, 1,
//										 &apps[i].resParams);
//		else
//			VAccessGetResourceParameters(network, logonTicket, apps[i].uuid, 2,
//										 &apps[i].resParams);
//		free(app);
//		i++;
//	}
//	*appCount = i;
//    mxmlDelete(tree);
//	return errorCode;
//}

//int VAccessGetResourceList(const NETWORK *network, const char *logonTicket,
//		VAccessApplication *apps, int *appCount, VAccessDesktopPool *desktopPools,
//		int *dpCount, VIRTUALDISK *virtualDisks, int *diskCount,
//		VAccessRemoteDesktop *remoteDesktops, int *rdCount)
//{
//	char buffer[BUFFERLENGTH];
//	int len, httpStatus, errorCode;
//    mxml_node_t *tree = NULL, *node;
//	char *str, *url;
//	int i = 0;

//	url = MakeUrl(network, "/RestService/User/ResourceList");
//    str = (char *)malloc(MAX_LEN);
//	sprintf(str, "LogonTicket=%s", logonTicket);
//	httpStatus = HttpGet(url, str, buffer, &len);
//#ifdef DEBUG_MODE
//	printf("getresourcelist = \n%s\n", str);
//#endif
//	free(url);
//	free(str);

//	if (httpStatus < 0)
//		return httpStatus;

//#ifdef DEBUG_MODE
//	printf("resourcelist: \n%s\n", buffer);
//#endif
//	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
//	errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
//	if (errorCode < 0)
//		return errorCode;

//	for (node = mxmlFindElement(tree, tree, "Application", NULL, NULL, MXML_DESCEND);
//			node != NULL;
//			node = mxmlFindElement(node, tree, "Application", NULL, NULL, MXML_DESCEND))
//	{
//		memset(&apps[i], 0, sizeof(VAccessApplication));
//		apps[i].type = XMLFindIntElement(node, tree, "Type");
//        XMLFindStrElement(node, tree, "Uuid", apps[i].uuid, MAX_LEN);
//        XMLFindBase64Element(node, tree, "Name", apps[i].name, MAX_LEN);
//        XMLFindStrElement(node, tree, "HostName", apps[i].hostname, MAX_LEN);
//        XMLFindBase64Element(node, tree, "Description", apps[i].description, MAX_LEN);
//        XMLFindBase64Element(node, tree, "HostDescription", apps[i].hostDescription, MAX_LEN);
//		if (apps[i].type == 1)
//			VAccessGetResourceParameters(network, logonTicket, apps[i].uuid, 1,
//										 &apps[i].resParams);
//		else
//			VAccessGetResourceParameters(network, logonTicket, apps[i].uuid, 0,
//										 &apps[i].resParams);
//		i++;
//	}
//	*appCount = i;

//	i = 0;
//	for (node = mxmlFindElement(tree, tree, "DesktopPool", NULL, NULL, MXML_DESCEND);
//			node != NULL;
//			node = mxmlFindElement(node, tree, "DesktopPool", NULL, NULL, MXML_DESCEND))
//	{
//		memset(&desktopPools[i], 0, sizeof(VAccessDesktopPool));
//        XMLFindStrElement(node, tree, "Uuid", desktopPools[i].uuid, MAX_LEN);
//        XMLFindBase64Element(node, tree, "Name", desktopPools[i].name, MAX_LEN);
//        desktopPools[i].type = XMLFindIntElement(node, tree, "Type");
//        desktopPools[i].userAssignment = XMLFindIntElement(node, tree, "UserAssignment");
//        desktopPools[i].sourceType = XMLFindIntElement(node, tree, "SourceType");
//        desktopPools[i].enable = XMLFindIntElement(node, tree, "Enable");
//        desktopPools[i].state = XMLFindIntElement(node, tree, "State");
//        XMLFindBase64Element(node, tree, "Description", desktopPools[i].description, MAX_LEN);
//		VAccessGetResourceParameters(network, logonTicket, desktopPools[i].uuid, 2,
//									 &desktopPools[i].resParams);
//        printf("s=%d, p=%d, ao=%d, ai=%d\n",desktopPools[i].resParams.serialPort, desktopPools[i].resParams.parallelPort,
//               desktopPools[i].resParams.audio, desktopPools[i].resParams.audioIn);
//		i++;
//	}
//	*dpCount = i;

//	i = 0;
//	for(node = mxmlFindElement(tree,tree,"RemoteDesktop",NULL,NULL,MXML_DESCEND);
//			node != NULL;
//			node = mxmlFindElement(node, tree, "RemoteDesktop",NULL,NULL,MXML_DESCEND))
//	{
//		memset(&remoteDesktops[i], 0, sizeof(VAccessRemoteDesktop));
//        XMLFindBase64Element(node, tree, "Uuid", remoteDesktops[i].uuid, MAX_LEN);
//        XMLFindBase64Element( node, tree, "Name", remoteDesktops[i].name, MAX_LEN);
//        XMLFindStrElement(node, tree, "HostName", remoteDesktops[i].hostname, MAX_LEN);
//		remoteDesktops[i].rdpServiceState = XMLFindIntElement(node, tree, "RdpServiceState");
//		remoteDesktops[i].vmState = XMLFindIntElement(node, tree, "VmState");
//        XMLFindBase64Element(node, tree, "Description", remoteDesktops[i].description, MAX_LEN);

//		VAccessGetResourceParameters(network, logonTicket, remoteDesktops[i].uuid, 3,
//									 &remoteDesktops[i].resParams);
//		i++;
//	}
//	*rdCount = i;

//	i = 0;
//	for( node = mxmlFindElement(tree, tree, "Disk", NULL, NULL, MXML_DESCEND);
//		node != NULL;
//		node = mxmlFindElement(node,tree,"Disk",NULL,NULL,MXML_DESCEND))
//	{
//		memset(&virtualDisks[i], 0 , sizeof(VIRTUALDISK));
//        XMLFindStrElement(node, tree, "DevicePath",virtualDisks[i].devicePath,MAX_LEN);
//        XMLFindStrElement(node,tree, "DiskSize",virtualDisks[i].diskSize, MAX_LEN );
//        XMLFindStrElement(node, tree, "SizeUnit", virtualDisks[i].sizeUnit, MAX_LEN);

//		i++;
//	}
//	*diskCount = i;
//    mxmlDelete(tree);
//	return errorCode;
//}

//int VAccessLaunchApplication(const NETWORK *network, const char *logonTicket,
//                             const char *appUuid, SELECTAPPLICATION *selectApp)
//{
//	return VAccessLaunchCommon(network, logonTicket, "/Action/LaunchApplication",
//							   "LaunchApplication", "AppUuid", appUuid, "AppTicket", selectApp);
//}

//int VAccessLaunchDesktopPool(const NETWORK *network, const char *logonTicket,
//                             const char *desktopPoolUuid, SELECTAPPLICATION *selectApp)
//{
//	return VAccessLaunchCommon(network, logonTicket, "/RestService/DesktopPool/LaunchDesktopPool",
//							   "LaunchDesktopPool", "DesktopPoolUuid",
//							   desktopPoolUuid, "DesktopPoolTicket", selectApp);
//}

//int VAccessShutdownApplication(const NETWORK *network, const char *logonTicket,
//							   const char *appTicket)
//{
//	return PostCommonWithErrorCode(network, logonTicket, "/Action/ShutdownApplication",
//					  "ShutdownApplication", "AppTicket", appTicket);
//}
//////////////////////////////////////////////////////////////////////////////




//int VAccessGetDomainList(const NETWORK *network, char (*domainlists)[MAX_LEN],
//					  int *domainCount)
//{
//	char buffer[BUFFERLENGTH];
//	int len, httpStatus, errorCode;
//    mxml_node_t *tree = NULL, *node;
//    char *url = NULL;
//    char domain[MAX_LEN];
//	int i = 0;

//	url = MakeUrl(network, "/RestService/User/DomainList");
//	httpStatus = HttpGet(url, NULL, buffer, &len);
//    if(httpStatus<0 && strlen(network->alternateServer)>0)
//    {
//        free(url);
//        url = NULL;
//        strcpy(network->presentServer, network->alternateServer);
//        url = MakeUrl(network, "/RestService/User/DomainList");
//        httpStatus = HttpGet(url, NULL, buffer, &len);
//    }
//	free(url);

//	if (httpStatus < 0)
//		return httpStatus;

//#ifdef DEBUG_MODE
//	printf("domainlist: \n%s\n", buffer);
//#endif
//	tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK);
//	errorCode = XMLFindIntElement(tree, tree, "ErrorCode");
//	if (errorCode < 0)
//		return errorCode;

//	for (node = mxmlFindElement(tree, tree, "Domain", NULL, NULL, MXML_DESCEND);
//			node != NULL;
//			node = mxmlFindElement(node, tree, "Domain", NULL, NULL, MXML_DESCEND))
//	{
//		memset(domain, 0, sizeof(domain));
//		if (node->child != NULL)
//			strcpy(domain, base64_decode(node->child->value.text.string));
//		strcpy(domainlists[i], domain);
//		i++;
//	}
//	memset(domainlists[i], 0, sizeof(domainlists[i]));
//	*domainCount = i;
//    mxmlDelete(tree);
//	return errorCode;
//}
