TARGET = vClient
TEMPLATE = app
#QMAKE_LFLAGS_RELEASE += -static-libstdc++

DEPENDPATH += ./ \
            ui/ \
            backend/ \
            common/ \
            dependences/ \
            dependences/openssl/lib/

INCLUDEPATH += ../ \
            ui/  \
            backend/ \
            common/ \
            dependences/

HEADERS += \
    backend/wait_obj.h \
    backend/tcp.h \
    backend/mxml.h \
    backend/http.h \
    backend/base64.h \
	backend/globaldefine.h \
    common/log.h \
    common/errorcode.h \
    common/ds_vclient.h \
    config.h \
    backend/vaccess.h \
    backend/csession.h \
    common/ds_access.h \
    common/ds_session.h \
    ui/userlogindlg.h \
    ui/titlewidget.h \
    ui/sysbutton.h \
    ui/cmytabwidget.h \
    ui/cmypushbutton.h \
    ui/ui_interact_backend.h \
    ui/cmessagebox.h \
    ui/desktoplistdialog.h \
    ui/ccontextmenu.h \
    ui/cselfservicedialog.h \
    ui/cdesktoplistitem.h \
    common/common.h \
    common/cmutexop.h \
    ui/csettingwindow.h \
    common/cconfiginfo.h \
    common/ds_settings.h \
    dependences/tinyxml/tinyxml.h \
    dependences/tinyxml/tinystr.h \
    backend/cthreadpool.h \
    ui/cloadingdialog.h \
    common/filepath.h \
    ui/ctooltip.h \
    ui/FastQImage.h \
    ui/cpersonalsetting.h \
    ui/keepsessionthread.h \
    frontend/cfrontendbase.h \
    common/cprocessop.h \
    frontend/cfrontendrdp.h \
    frontend/cfrontendfap.h \
    frontend/claunchapp.h \
    common/ds_launchapp.h \
    common/cthread.h \
    ui/networksettingdialog.h \
    ui/desktopsettingdialog.h \
    frontend/cusbmap.h \
    common/cevent.h \
    ui/cclientapplication.h \
    ui/cupdate.h \
    ipc/ipcclient.h \
    ipc/cJSON.h \
    ui/cselectdialog.h \
    ui/csetdefaultapp.h \
    ui/requestdesktopdlg.h \
    frontend/cfrontendterminal.h \
    ui/multiAccessesDialog.h \
    ui/multiAccessesTableWidget.h \
    common/rh_ifcfg.h \
    common/MultiAccesses.h \
    backend/globaldefine.h \
    backend/csysinfobase.h \
    ui/terminaldesktoplist.h \
    ui/cterminalitem.h \
    ipc/ipcitalc.h \
    ui/autologindialog.h \
    imageconf.h

SOURCES += \
    backend/wait_obj.c \
    backend/tcp.c \
    backend/http.c \
    backend/base64.c \
    common/log.c \
    ui/main.cpp \
    backend/csession.cpp \
    backend/vaccess.cpp \
    ui/userlogindlg.cpp \
    ui/titlewidget.cpp \
    ui/sysbutton.cpp \
    ui/cmytabwidget.cpp \
    ui/cmypushbutton.cpp \
    ui/ui_interact_backend.cpp \
    ui/cmessagebox.cpp \
    ui/desktoplistdialog.cpp \
    ui/ccontextmenu.cpp \
    ui/cselfservicedialog.cpp \
    ui/cdesktoplistitem.cpp \
    common/common.cpp \
    common/cmutexop.cpp \
    ui/csettingwindow.cpp \
    common/cconfiginfo.cpp \
    dependences/tinyxml/tinyxmlparser.cpp \
    dependences/tinyxml/tinyxmlerror.cpp \
    dependences/tinyxml/tinyxml.cpp \
    dependences/tinyxml/tinystr.cpp \
    backend/cthreadpool.cpp \
    ui/cloadingdialog.cpp \
    ui/ctooltip.cpp \
    ui/FastQImage.cpp \
    ui/cpersonalsetting.cpp \
    ui/keepsessionthread.cpp \
    frontend/cfrontendbase.cpp \
    common/cprocessop.cpp \
    frontend/cfrontendrdp.cpp \
    frontend/cfrontendfap.cpp \
    frontend/claunchapp.cpp \
    common/cthread.cpp \
    ui/networksettingdialog.cpp \
    ui/desktopsettingdialog.cpp \
    frontend/cusbmap.cpp \
    common/cevent.cpp \
    ui/cclientapplication.cpp \
    ui/cupdate.cpp \
    ipc/ipcclient.cpp \
    ipc/cJSON.c \
    ui/cselectdialog.cpp \
    ui/csetdefaultapp.cpp \
    ui/requestdesktopdlg.cpp \
    frontend/cfrontendterminal.cpp \
    ui/multiAccessesDialog.cpp \
    ui/multiAccessesTableWidget.cpp \
    common/rh_ifcfg.cpp \
    common/MultiAccesses.cpp \
    backend/csysinfobase.cpp \
    ui/terminaldesktoplist.cpp \
    ui/cterminalitem.cpp \
    ipc/ipcitalc.cpp \
    ui/autologindialog.cpp \
    ui/imageconf.cpp

FORMS += \
    ui/userlogindlg.ui \
    ui/desktoplistdialog.ui \
    ui/cselfservicedialog.ui \
    ui/settingwindow.ui \
    ui/cmessagebox.ui \
    ui/cpersonalsetting.ui \
    ui/cloadingdialog.ui \
    ui/networksettingdialog.ui \
    ui/desktopsettingdialog.ui \
    ui/cselectdialog.ui \
    ui/requestdesktopdlg.ui \
    ui/multiAccessesDialog.ui \
    ui/terminaldesktoplist.ui \
    ui/autologindialog.ui


QT += network gui core xml
TRANSLATIONS  += ui/vclient_zh.ts
win32 {
RC_FILE =  ui/resource/icon.rc
LIBS  += -lwsock32 "dependences/openssl/lib/ssleay32.lib" "dependences/openssl/lib/libeay32.lib" "dependences/mxml.lib"
INCLUDEPATH   += "dependences/openssl/include"

Release:MOC_DIR = tmp/moc/release_shared
Release:RCC_DIR = tmp/rcc/release_shared
Release:OBJECTS_DIR = tmp/obj/release_shared
Debug:MOC_DIR = tmp/moc/debug_shared
Debug:RCC_DIR = tmp/rcc/debug_shared
Debug:OBJECTS_DIR = tmp/obj/debug_shared
}

unix{
LIBS += -lmxml -lssl -lcrypto -lcpprest 
LIBS += -lboost_random -lboost_chrono -lboost_system -lboost_thread -lboost_locale -lboost_regex -lboost_filesystem -lboost_log -lboost_log_setup
}

RESOURCES += \
    ui/vclient_resource.qrc
