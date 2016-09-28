#ifndef CDESKTOPLISTITEM_H
#define CDESKTOPLISTITEM_H

#include <QWidget>
#include <QLabel>
#include "ccontextmenu.h"
#include "../common/ds_settings.h"
#include "../common/ds_vclient.h"

class SysButton;
class CSelfServiceDialog;
class CToolTip;
class QMutex;

enum ITEMSTATUS{ITEM_UNABLE = 0,
                ITEM_ENABLE = 1,
                ITEM_CONNECTED = 2
                };

enum CONNECT_TYPE{RDP_CONNECT, FAP_CONNECT, TERMINAL_CONNECT};

const QString BUSYIMAGE = ":image/resource/image/busy.gif";

struct uiStatus
{
    bool isConnected;
    bool isStarting;
    bool isReStarting;

    //added by qc
    bool isDisconnecting;

    bool isShutdown;
    SERIAL_PARALLEL_PORT stPort;    //Serial port and parallel port
    bool hasVDisk;      //Load the persional disk?
    bool hasUsbMapping;        //Map usb when connect?
    bool bHasFileSystemMapping;
    CONNECT_TYPE connectType;    //The way connect to desktop user select
    SUPPORT_PROTOCAL support_protocal;  //what connect protocal the desktop support
    ITEMSTATUS item_status;      //Item status
    bool bItemClicked;              //Is item current selected flag
    bool bHasGetLoadIconFromVaccess;//has get icon from vAccess
};
typedef struct uiStatus UI_STATUS;

struct itemData
{
    QString uuid;
    APP_LIST *appData;
    UI_STATUS ui_status;
    VIRTUALDISK *vDisk;
};
typedef struct itemData ITEM_DATA;

class CDesktopListItem : public QWidget
{
    Q_OBJECT
    
public:
    explicit CDesktopListItem(QWidget *parent = 0);
    ~CDesktopListItem();
    void setBaseWidget();
    void setDesktopIconWidget();
    void setContentWidget();
    void setDesktopIconLink();
    void setContextMenuStatus(SUPPORT_PROTOCAL protocal);

    void setItemStatus(ITEMSTATUS status = ITEM_ENABLE);
    void setDesktopName(QString name);
    void setDesktopDescription(QString description);
    void setSelfServiceBtnEnable(bool enable = true);
    void setSelfServiceStatus(int status=0 ); /* -1. All buttons can not be used,
                                                  0.  Only close and reboot can be used
                                                  1.  Only open button can be used*/
    void hideSelfServiceDlg(bool bHide);
    void setData(const ITEM_DATA &data){m_data = data; }
    inline ITEM_DATA& getData() {return m_data;}
    void setBusy(bool isBusy);

    int setAppIconData(char* pIconData, int len);
protected:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void fillDesktopDescription(QString &descr);
    void itemPressStatus();
    void setItemBorder(const QString strBorderStyle);

signals:
    void on_signal_launch_desktop(QString);
    void on_signal_start_desktop(QString);
    void on_signal_restart_desktop(QString);
    void on_signal_shutdown_desktop(QString);
    void on_signal_set_desktop(QString);
    void on_signal_disconnect(QString);
    void on_signal_setDefaultApp(int,LAUNCH_TYPE, APP_LIST);//<0 failed.
                                                //1 set as default app
                                                //2 cancel default app set
    void mouseLeftButton();

public slots:
    void controlBtn_clicked_slot();
    void on_rdpAction_clicked_slot();
    void on_fapAction_clicked_slot();
    void on_terminalAction_clicked_slot();
    void on_desktopSettingAction_clicked_slot();
    void on_setDefaultApp_clicked_slot();

    //void on_desktop_start_slot();
    void on_desktop_disconnect_slot();
    void on_desktop_restart_slot();
    void on_desktop_shutdown_slot(int);////0 :shutdown 1 start
    void on_close_toolTip();
    void on_rotate_controlBtn_pixmap();
    void on_selfServiceDlg_close();
    
private:
    QWidget *m_baseWidget;
    QWidget *m_iconWidget;
    QWidget *m_contentWidget;
    QLabel *m_desktopIconLabel;
    QLabel *m_linkIconLabel;
    QLabel *m_busyStatusLabel;
    QMovie *m_busyMovie;
    QLabel *m_desktopNameLabel;
    QLabel *m_desktopDescription;
    SysButton *m_controlBtn;
    ITEMSTATUS m_itemStatus;
    int m_selfServiceState;
    bool m_isBusy;
    bool m_selfServiceBtnEnable;

    CContextMenu m_contextMenu;
    CToolTip *m_toolTip;
    CSelfServiceDialog *m_selfServiceDlg;
    ITEM_DATA m_data;
public:
    static QMutex* m_pmutex_GetSelectedItem;
    int setSelectedItem(CDesktopListItem*);
    CDesktopListItem* getSelectedItem();
    static int getSelectItemInfo(std::string& str_resourceUuid, int* pIResType);
};

#endif // CDESKTOPLISTITEM_H
