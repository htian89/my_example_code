#include "cclientapplication.h"
#include "../common/log.h"

extern bool g_bSystemShutDownSignal;    //From desktoplistdialog.cpp

CClientApplication::CClientApplication(int &argc, char **argv)
    :QApplication(argc, argv)
{
}

#ifdef WIN32
bool CClientApplication::winEventFilter(MSG *msg, long *result)
{
    if(msg->message==WM_QUERYENDSESSION ||WM_QUIT_FORUPDATE == msg->message)
    {
            LOG_ERR("%s=====%d, %d", "Recivice the system showdown or logout signal", msg->message, msg->lParam);
            g_bSystemShutDownSignal = true;
//            return true;
    }
    if(WM_QUIT_FORUPDATE == msg->message)//if(WM_QUIT_FORUPDATE == msg->message ||(ENDSESSION_CLOSEAPP==msg->lParam && msg->message==WM_QUERYENDSESSION))
    {
        //msg->message = WM_CLOSE;
        PostQuitMessage(0);
        return true;
    }

    return QCoreApplication::winEventFilter(msg, result);
}
#endif

bool CClientApplication::event ( QEvent * e )
{
    if(NULL !=  e)
    {
        if(QEvent::ApplicationActivate == e->type())
            emit appIsActivated(1);
        else if(QEvent::ApplicationDeactivate == e->type())
            emit appIsActivated(0);
    }
    return QApplication::event(e);
}
