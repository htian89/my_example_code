#ifndef MULTIACCESSESDIALOG_H
#define MULTIACCESSESDIALOG_H

#include <QDialog>
#include "multiAccessesTableWidget.h"
#include "networksettingdialog.h"
#include "ui_multiAccessesDialog.h"
#include "sysbutton.h"

namespace Ui {
class multiAccessesDialog;
}

class multiAccessesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit multiAccessesDialog(NetWorkSettingDialog *parent = 0);
    multiAccessesTableWidget *m_multiAccessesTableWidget;

    ~multiAccessesDialog();
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonOk_clicked();
    void on_pushButtonDel_clicked();
private:
    Ui::multiAccessesDialog *ui;
    NetWorkSettingDialog *m_caller;
    QPoint m_pressPoint;
    bool m_isMove;
    QPixmap m_backgroundPixmap;

    SysButton* sysClose;
};

#endif // MULTIACCESSESDIALOG_H
