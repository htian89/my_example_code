#include "common.h"
#include <QtGui>
#include <QString>
#include <QDesktopWidget>
#ifdef WIN32
#include <windows.h>
#endif
#include "ds_vclient.h"
#include "filepath.h"
#include "log.h"


void SetUsernameValidator(QLineEdit *lineEdit_Username)
{
#ifdef _WIN32
    QRegExp rx("^[^(\"/\\\\\\\[\\\]:;|=,+*?<>)]{1,20}$");
    QValidator *validator = new QRegExpValidator(rx, NULL);
    lineEdit_Username->setValidator(validator);
#endif
}

//for auto-update.
int saveUserPath()
{
#ifdef _WIN32
    char fileName[MAX_LEN];
    memset(fileName, 0, MAX_LEN);
    if (FALSE == GetModuleFileNameA(NULL, fileName, MAX_LEN))
    {
        return -1;
    }
    for(size_t i = strlen(fileName); i>0; i--)
    {
        if(fileName[i]=='/' || fileName[i]=='\\')
        {
            fileName[i+1] = '\0';
            break;
        }
    }
    strcat(fileName, USERPATH_STORFILE);
    FILE* fp = fopen(fileName, "wb");
    if(NULL != fp)
    {
        fputs(userPath.data(),fp);
        fclose(fp);
    }
    else
    {
        printf("openfile %s failed.", USERPATH_STORFILE);
        LOG_ERR("openfile %s failed.", USERPATH_STORFILE);
    }
#endif
    return 0;
}


void SetUrlValidator(QLineEdit *lineEdit_Url)
{
//    QRegExp rx("^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.{3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]))((:[a-zA-Z0-9]*)?/?([a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~])*)"
//               "|([a-zA-Z0-9_\\-\\.])+\\.(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me)((:[a-zA-Z0-9]*)?/?([a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~])*)$");
//               //"((:[a-zA-Z0-9]*)?/?([a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~])*)$");

    QRegExp rx("^((((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?)|(([a-zA-Z0-9_\\-\\.])+\\.(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me)(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?))$");
    QValidator * validator = new QRegExpValidator(rx,NULL);
    lineEdit_Url->setValidator(validator);
}
// can't checkout 0.0.0.0
bool IsValidUrl(QString str)
{
    //    QRegExp rx("^(((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.)"
    //               "{3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])|([a-zA-Z0-9_\\-\\.])+\\."
    //               "(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me))"
    //               "((:[a-zA-Z0-9]*)?/?([a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~])*)$");
    QRegExp rx("^((((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?)|(([a-zA-Z0-9_\\-\\.])+\\.(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me)(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?))$");

    QRegExpValidator validator(rx,NULL);
    int pos = 0;
    if(validator.validate(str,pos) == QValidator::Acceptable)
        return true;
    return false;
}

void SetPortValidator(QLineEdit *lineEdit_Port)
{
    QValidator *validator = new QIntValidator(0, 65535, NULL);
    lineEdit_Port->setValidator(validator);
}


bool IsValidPort(QString str)
{
    QIntValidator validator(0, 65535, NULL);
    int pos = 0;

    if (validator.validate(str, pos) == QValidator::Acceptable)
        return true;

    return false;
}

void setDialogInCenter(QWidget *widget)
{
#ifdef WIN32
#else
    widget->move((qApp->desktop()->availableGeometry().width()-widget->width())/2, (qApp->desktop()->availableGeometry().height()-widget->height())/2);
#endif
}


#ifdef _WIN32
#define WM_MY_QUERY_LOUNCH_ON_SYSSTART		WM_USER+0x100
#define WM_MY_SET_LOUNCH_ON_SYSSTART		WM_USER+0x101
#define SLOG_NAME		"\\\\.\\mailslot\\vclient_lounchOnSystemStart"
#define WINDOW_TITLE_NAME       "vclient_lounchOnSystemStart_vclient_app"

#define SET_ONLY_ME			0x001
#define SET_START_YES		0x010
//===========================================================
//Function Name:
//	lounchOnSystemStart
//Parameters:
//	pch_appToStart:	const char*   the application(need full path
//						and parameter) to be started when system start
//	i_forAllUser: int	0: for current user only
//						1: for all user
//	i_opType: int		0: add    1: delete 2:query
//
//Return Value:
//	>=0:sucess		<0:failed
//===========================================================
int lounchOnSystemStart(const char* pch_appToStart, int i_forAllUser, int i_opType)
{
    //HANDLE hSlot  = CreateFileA(SLOG_NAME, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//CreateMailslotA(SLOG_NAME, 0, MAILSLOT_WAIT_FOREVER, NULL);
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd,TRUE,NULL,FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    HANDLE hSlot  = CreateMailslotA(SLOG_NAME, 0, MAILSLOT_WAIT_FOREVER, &sa);
    if (hSlot == INVALID_HANDLE_VALUE)
    {
        printf("CreateMailslot failed with %d\n", GetLastError());
        return -1;
    }

    HWND hwnd = FindWindowA(NULL, WINDOW_TITLE_NAME);
    if(NULL == hwnd)
    {
        printf("get window handle failed\n");
        CloseHandle(hSlot);
        return -5;
    }

    int msgType = WM_MY_SET_LOUNCH_ON_SYSSTART;
    if(i_opType == 2)
    {
        msgType = WM_MY_QUERY_LOUNCH_ON_SYSSTART;
    }
    int op=0;
    if(i_forAllUser == 0)
    {
        op|=SET_ONLY_ME;
    }
    if(i_opType ==0)
    {
        op|=SET_START_YES;
    }
    BOOL b = PostMessageA(hwnd, msgType, op, (LPARAM)hSlot);
    if(b==FALSE)
    {
        printf("post msg failed!!!!");
        CloseHandle(hSlot);
        return -50;
    }
    char buffer[12];
    DWORD dwReadBytes = 0;
    memset(buffer, 0, 12);
    printf("lounchOnSystemStart:wait for return===========%s\n", SLOG_NAME);
    if(FALSE == ReadFile(hSlot, buffer, 10, &dwReadBytes, NULL))
    {
        CloseHandle(hSlot);
        printf("lounchOnSystemStart:ReadFile failed %d\n", GetLastError());
        return -1000;
    }
    if(dwReadBytes <=0)
    {
        CloseHandle(hSlot);
        printf("lounchOnSystemStart:dwReadBytes <=0\n");
        return -1500;
    }
    printf("lounchOnSystemStart:%c\n",buffer[0]);
    if(buffer[0] =='y')
    {
        CloseHandle(hSlot);
        return 0;
    }
    CloseHandle(hSlot);
    printf("lounchOnSystemStart:==============\n");
    return -1;

}
#else
int lounchOnSystemStart(const char* pch_appToStart, int i_forAllUser, int i_opType)
{
	//no use func in linux
	if( NULL != pch_appToStart){
		LOG_INFO("appPath: %s, for all user: %d, type: %s", pch_appToStart, i_forAllUser, i_opType);
	}
    return -1;
}
#endif
