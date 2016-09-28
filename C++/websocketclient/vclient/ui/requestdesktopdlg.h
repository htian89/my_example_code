#ifndef REQUESTDESKTOPDLG_H
#define REQUESTDESKTOPDLG_H

#include "../backend/csession.h"
#include <QDialog>
#include <QPainter>
#include <config.h>
#include "cloadingdialog.h"
const QString IMAGE_PATH_REQUESTDESKTOP_BACKGROUND = ":image/resource/image/skin_expand.png";
namespace Ui {
class RequestDesktopDlg;
}

class RequestDesktopDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit RequestDesktopDlg(QWidget *parent = 0);
    void processCallBackData(int errorCode, int dType, void *pRespondData);
    void processErrorCode(int errorCode, int opType);
    void setLoadingDlg(bool enable);
    ~RequestDesktopDlg();
protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
signals:
    void on_signal_requestDesktop_finished(int, int);
private slots:
    void on_pushBtn_ok_clicked();
    void on_requestDesktop_finished(int,int);
private:
    Ui::RequestDesktopDlg *ui;
    QPoint m_pressPoint;
    bool m_isMove;
    CLoadingDialog *m_loadingDlg;
    CSession *z_psession;
    REQUEST_DESKTOP z_requestDesktop;
    QPixmap backgroundPixmap;
};

#endif // REQUESTDESKTOPDLG_H
