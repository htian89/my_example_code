#ifndef DESKTOPSETTINGDIALOG_H
#define DESKTOPSETTINGDIALOG_H

#include <QDialog>
#include<../common/ds_session.h>

const QString IMAGE_PATH_DESKTOP_SETTING_BACKGROUND = ":image/resource/image/skin_expand.png";

namespace Ui {
class DesktopSettingDialog;
}

class CDesktopListItem;
class CSession;
class CLoadingDialog;
class TitleWidget;

struct desktopSettingParameter
{
    bool hasVirtualDisk;    //Has disk to load?
    CDesktopListItem *item;
};

typedef desktopSettingParameter DESKTOP_SETTING_PARAM;

class DesktopSettingDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DesktopSettingDialog(const DESKTOP_SETTING_PARAM &param, QWidget *parent = 0);
    ~DesktopSettingDialog();
    void processCallBackData(int errorCode, int dType, void *pRespondData);

protected:
    void paintEvent(QPaintEvent *);
//    void mousePressEvent(QMouseEvent *);
//    void mouseMoveEvent(QMouseEvent *);
//    void mouseReleaseEvent(QMouseEvent *);
    void closeEvent(QCloseEvent *);
signals:
    void getResParametersFinished(int, GET_RESOURCE_PARAMETER);
private slots:
    void on_pushBtn_ok_clicked();
    void on_getResParametersFinished(int, GET_RESOURCE_PARAMETER);

private:
    Ui::DesktopSettingDialog *ui;
    bool m_isMove;
    QPoint m_pressPoint;
    DESKTOP_SETTING_PARAM m_param;
    CSession *m_pSession;
    CLoadingDialog* m_dlg_loading;
    TitleWidget *m_titleWidget;
};

#endif // DESKTOPSETTINGDIALOG_H
