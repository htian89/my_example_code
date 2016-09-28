#include "keepsessionthread.h"
#include "../common/log.h"
#include "cclientapplication.h"
#include "cdesktoplistitem.h"
#include <QDebug>
#include <time.h>
#include <iostream>
using namespace std;
extern CClientApplication* pApp; //defined in ui/main.cpp

#define WAIT_TIME_REFRESH_APP_ACTIVATED     10000
#define WAIT_TIME_REFRESH_APP_DEACTIVATED   60000

RefreshListThread::RefreshListThread() :
    QThread(),
    m_stop(false)
{
    if(NULL != pApp)
        connect(pApp, SIGNAL(appIsActivated(int)), this, SLOT(on_appIsActivatedSlot(int)));
    else
    {
        LOG_ERR("%s", "NULL == pApp");
    }
    m_iAppIsActivated = 1;
    m_hasDisconnectSignalAppIsActivated = false;
}

RefreshListThread::~RefreshListThread()
{
    if(NULL!=pApp && false==m_hasDisconnectSignalAppIsActivated)
    {
        m_hasDisconnectSignalAppIsActivated = true;
        disconnect(pApp, SIGNAL(appIsActivated(int)), this, SLOT(on_appIsActivatedSlot(int)));
    }
}

void RefreshListThread::setStop(bool stop)
{
    m_time = 0;
    m_stop = stop;
    if(m_stop)
    {
        m_waitCondition.wakeOne();
        LOG_INFO("%s","going to stop RefreshListThread");
    }
}

void RefreshListThread::on_appIsActivatedSlot(int state)
{
    LOG_INFO("get appIsActivated signal:%d",state);
    qDebug()<<"get appIsActivated signal "<<state;
    m_iAppIsActivated = state;    

    if(state>0&&!m_stop)
        m_waitCondition.wakeOne();
}

void RefreshListThread::run()
{
    LIST_USER_RESOURCE_DATA  listUserResData;
    GET_USER_INFO_DATA getUserInfoData;
    int waitTime = 0, tryCount = 0;
    if(m_iAppIsActivated>0)
        waitTime = WAIT_TIME_REFRESH_APP_ACTIVATED;
    else
        waitTime = WAIT_TIME_REFRESH_APP_DEACTIVATED;
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return ;
    }
    pCall_param->pUi = NULL;
    pCall_param->uiType = 0;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = NULL;
    param.callback_param = pCall_param;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;

    time(&m_time);
    srand((unsigned int)time(NULL));
    waitTime = waitTime + 1000*(rand()%9);
    LOG_INFO("------------------------waittime:%d", waitTime);
    while(!m_stop)
    {        
        m_mutex.lock();
        //int waitTime_rand = qrand()%5;
        //waitTime += waitTime_rand*1000;
        //LOG_INFO("going to wait. time:%d,%d", waitTime,waitTime_rand);
        m_waitCondition.wait(&m_mutex, waitTime);
        LOG_INFO("wait wake up....., m_stop:%d, waittime:%d", int(m_stop), waitTime);
        if(m_stop)
        {
            m_mutex.unlock();
            break;
        }
        m_pSession =CSession::GetInstance();
        if(m_pSession==NULL)
        {
            LOG_ERR("%s", "keep Session failed! NULL == CSession");
            m_mutex.unlock();
            break;
        }
        //m_pSession->getUserInfo(param, &getUserInfoData);
        if(m_iAppIsActivated>0)
        {
            time_t curTime = 0;
            time(&curTime);
            int iInterval = curTime-m_time;
            LOG_INFO("iInterval time:%d, %d, %d", iInterval, curTime, m_time);
            if(iInterval>8||iInterval<0)
                m_time = curTime;
            else
            {//time interval is too short, donot going to refresh immediately
                if(WAIT_TIME_REFRESH_APP_ACTIVATED-iInterval>0)
                    waitTime = WAIT_TIME_REFRESH_APP_ACTIVATED-iInterval;
                m_mutex.unlock();
                continue;
            }
            m_pSession->listUserResource(param, false, &listUserResData);
            cout <<  "stappbaklist:"<< listUserResData.stAppBakList.size() << endl;
            if(param.callback_param->errorCode!=0)
            {
                LOG_ERR("%s, ErrorCode=%d", "Refresh resource failed! NULL == pCall_param", param.callback_param->errorCode);
                if(tryCount>2)
                {
                    if(!m_stop)
                        emit on_signal_update_failed(param.callback_param->errorCode, TYPE_LIST_USER_RES);
                    m_stop = true;
                }
                tryCount++;
                waitTime = waitTime/10;
            }
            else
            {
                tryCount = 0;
                waitTime = WAIT_TIME_REFRESH_APP_ACTIVATED;
                GET_RESOURCE_PARAMETER resParam_out;
                memset(resParam_out.deskUuid, 0, MAX_LEN);
                std::string str_selectedDeskUuid;
                int iDeskType, iRet  = 0;
                iRet = CDesktopListItem::getSelectItemInfo(str_selectedDeskUuid, &iDeskType);
                if(iRet < 0)
                {
                    LOG_ERR("CDesktopListItem::getSelectItemInfo failed return value:%d", iRet);
                }
                else
                {
                    iRet = m_pSession->getResParam(str_selectedDeskUuid.c_str(), iDeskType, &(resParam_out.stResPara));
                    if(iRet < 0)
                    {
                        LOG_ERR("getResParam failed. return value:%d", iRet);
                    }
                    else
                    {
                        strcpy(resParam_out.deskUuid, str_selectedDeskUuid.c_str());
//                    for(std::vector<APP_LIST>::size_type i=0; i<listUserResData.stAppList.size(); ++i)
//                    {
//                        if(0 == strcmp((listUserResData.stAppList)[i].uuid, resParam_out.deskUuid))
//                        {
//                            LOG_INFO("has found the item in APPLIST.deskname:%s", (listUserResData.stAppList)[i].name);
//                            (listUserResData.stAppList)[i].resParams = resParam_out.stResPara;
//                            break;
//                        }
//                    }

                    }
                }
                if(!m_stop)
                    emit on_signal_update_resourceList(listUserResData, getUserInfoData, resParam_out);
                listUserResData.stAppList.clear();
                getUserInfoData.vstVirtualDisks.clear();
            }

        }
        else
        {
            int iRet = m_pSession->testConnectState();
            if(iRet < 0)
            {
                LOG_ERR("testConnectState return value:%d", iRet);
                tryCount++;
                waitTime = waitTime/10;
                if(tryCount>2)
                {
                    if(!m_stop)
                        emit on_signal_update_failed(iRet, TYPE_LIST_USER_RES);
                    m_stop = true;
                }
            }
            else
            {
                tryCount = 0;
                waitTime = WAIT_TIME_REFRESH_APP_DEACTIVATED;
            }
        }
        m_mutex.unlock();
    }
    if(NULL!=pApp && !m_hasDisconnectSignalAppIsActivated)
    {
        m_hasDisconnectSignalAppIsActivated = true;
        disconnect(pApp, SIGNAL(appIsActivated(int)), this, SLOT(on_appIsActivatedSlot(int)));
    }
    if(pCall_param!=NULL)
        delete pCall_param;
    LOG_INFO("%s","RefreshListThread is going to exit....");
}
