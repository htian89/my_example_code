#ifndef UI_INTERACT_BACKEND_H
#define UI_INTERACT_BACKEND_H

#include "common/ds_session.h"
#include "backend/csession.h"
enum UITYPE{LOGINDLG, SERVERSETTING, MAINWINDOW, PERSONALSETTING, DESKTOP_SETTING_DLG,REQUESTDESKTOP,TERMINALDESKTOPLISTDLG,AUTOLOGINDLG};

int uiCallBackFunc(CALLBACK_PARAM_UI*, int, void*);

void showUiErrorCodeTip(int errorCode);

#endif // UI_INTERACT_BACKEND_H
