#ifndef CTERMINALITEM_H
#define CTERMINALITEM_H

#include <QCheckBox>
#include "../common/ds_vclient.h"
#include <iostream>
#include <string>
using namespace std;



struct uiTerminalStatus
{
    bool isRestart;
    bool isPoweroff;
    bool isCheck;
    bool isUnCheck;
};

enum ITEMOS
{
    ITEM_POWEROFF = 0,
    ITEM_RESTART = 1,
    ITEM_MSG = 3
};

typedef struct uiTerminalStatus UI_TERMINALSTATUS;

struct terminalItemData
{
    /*char uuid[MAX_LEN]*/
    string uuid;
    APP_LIST *appData;
    UI_TERMINALSTATUS ui_status;
};

typedef struct terminalItemData ITEM_TERMINALDATA;

class CTerminalItem : public QCheckBox
{
    Q_OBJECT

public:
    explicit CTerminalItem(QWidget *parent = 0);
    ~CTerminalItem();


    void setTerminalName(QString name, QString ip);
    void setData(const ITEM_TERMINALDATA& data) { m_itemDada = data; }

    inline ITEM_TERMINALDATA& getTerminalData() { return m_itemDada; }

signals:
    void on_stateChanged_signal(string uuid, int state);

public slots:
    void on_stateChanged(int state);


private:
    ITEM_TERMINALDATA m_itemDada;
    QCheckBox *m_checkBox;
    QWidget *m_baseWidget;
};


#endif
