#ifndef CSETDEFAULTAPP_H
#define CSETDEFAULTAPP_H
#include <QWidgetAction>
#include <QMenu>
#include "../common/ds_launchapp.h"

class QPushButton;
class QDialogButtonBox;
class QRadioButton;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;

class CSetDefalutAppWidget;
class CSetDefaultApp:public QWidgetAction//, public QWidget
{
public:
    CSetDefaultApp(QObject *parent = 0);
    ~CSetDefaultApp();
    QWidget* createWidget(QWidget *);
    void deleteWidget(QWidget *);
    int getSetDefaultAppInfo(LAUNCH_TYPE& launchType, bool &bIsOkClicked);
    void setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol);
private:
    CSetDefalutAppWidget* m_setDefaultAppWidget;
};

class CSetDefalutAppWidget:public QWidget
{
    Q_OBJECT
public:
    CSetDefalutAppWidget(CSetDefaultApp* pSetAppAction, QWidget* parent = 0);
    ~CSetDefalutAppWidget();
    int getLaunchType(LAUNCH_TYPE& launchType);
    bool isOkClicked(){return m_bIsOkClicked;}
    void setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol);
public slots:
    void on_okBtn_clicked();
    void on_cancelBtn_clicked();
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
private:
    QLabel* m_labelMsg;

    QRadioButton *m_radioBtn_RDP;
    QRadioButton *m_radioBtn_FAP;
    QRadioButton *m_radioBtn_TERMINAL;


    QDialogButtonBox* m_dlgBox;
    QPushButton* m_btn_ok;
    QPushButton* m_btn_cancel;

    //QHBoxLayout* m_hboxLayout;
    QVBoxLayout* m_mainLayout;
    CSetDefaultApp* m_pSetAppAction;

    bool m_bIsOkClicked;
};

#endif // CSETDEFAULTAPP_H
