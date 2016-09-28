#-------------------------------------------------
#
# Project created by QtCreator 2012-12-17T16:00:54
#
#-------------------------------------------------

QT       += core gui network

TARGET = update_module
TEMPLATE = app

INCLUDEPATH = ./tinyxml
DEPENDPATH = ./tinyxml


SOURCES += main.cpp\
        updateinfoview.cpp \
    myudpsocket.cpp \
    ctcpsocket.cpp \
    log.c \
    chttp.cpp \
    tinyxml/tinyxmlparser.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinystr.cpp \
    cthread.cpp \
    autoupdate.cpp

HEADERS  += updateinfoview.h \
    myudpsocket.h \
    ctcpsocket.h \
    log.h \
    globaldefine.h \
    chttp.h \
    tinyxml/tinyxml.h \
    tinyxml/tinystr.h \
    cthread.h \
    autoupdate.h

FORMS    += updateinfoview.ui

TRANSLATIONS  += update_zh.ts

LIBS += -lmxml

MOC_DIR = ./tmp/
OBJECTS_DIR = ./tmp/

RESOURCES += \
    update_moudle_resource.qrc


