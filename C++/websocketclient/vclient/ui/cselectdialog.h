#ifndef CSELECTDIALOG_H
#define CSELECTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class CSelectDialog;
}

class CSelectDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CSelectDialog(const QString str_text0, const QString str_text1, const QString str_tips,\
                           const QString& str_title, int selection = 0, QWidget *parent = 0);
    ~CSelectDialog();
    int getSelection();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void on_pushbtnClicked();
private:
    QPoint m_pressPoint;
    bool m_isMove;

    QPixmap m_backgroundPixmap;
    Ui::CSelectDialog *ui;
    int m_seletedItem;
    QString  m_str_title;
    QString m_str_tips;
    QString m_str_text0;
    QString m_str_text1;
};

#endif // CSELECTDIALOG_H
