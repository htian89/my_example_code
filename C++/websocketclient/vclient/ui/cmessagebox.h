#ifndef CMESSAGEBOX_H
#define CMESSAGEBOX_H

#include <QtGui>
#include "config.h"
#include "sysbutton.h"
#include "string"
using namespace std;
namespace Ui {
    class CMessageBox;
}

enum CMSGBOXVALUE{REJECTED, ACCEPTED};
const QString BACKGROUND_IMAGE = ":image/resource/image/skin_expand.png";
const QString CRITICAL_IMAGE = ":image/resource/image/icon_erro.png";
const QString WARNING_IMAGE = ":image/resource/image/icon_hint.png";
//const QString ABOUT_IMAGE = WINDOWS_ICON;
const QString SELECT_IAMGE = ":image/resource/image/icon_info.png";
const QString VERTICAL_LINE_IMAGE = ":image/resource/image/vertical_line.png";
class CMessageBox : public QDialog
{
    Q_OBJECT
public:
    explicit CMessageBox(QWidget *parent = 0);
    ~CMessageBox();
    void setTitle(QString title);
    void setImage(QPixmap pixmap);
    void setImage2(QPixmap pixmap, bool bShow);
    void addButton(QPushButton *btn);
    void setContentText(QString text);
    Ui::CMessageBox * getUi(){return m_pUi;}

    static CMSGBOXVALUE SelectedBox(const QString &text, QWidget *parent = 0);
    static CMSGBOXVALUE messageBox(QWidget *parent);
    static CMSGBOXVALUE messageBox_setseatNumber(QWidget *parent = 0);

    static void CriticalBox(const QString &text, QWidget *parent = 0);
    static void WarnBox(const QString &text, QWidget *parent = 0);
    static void AboutBox(QWidget *parent =0);
    static void TipBox(const QString &text, QWidget *parent = 0);
    static void TipBox_seatNumber(const QString &text, QWidget *parent = 0);
    static void TipBox_Notes(const QString &text, QWidget *parent = 0);
    static QString btStyleSheet;
    static string m_message;

    static void setSeatNumber();
    static void sendSeatNumber();
    static void showSeatNumber();
    static void showNotes();
    QString setSeat;
    QString showSeat;

signals:
    void on_signal_setSeatNumber();
    void on_signal_setSeatNumber_finished();
    void on_signal_showSeatNumber();
    void on_signal_showNotes();

public slots:
    void on_text_change(QString msg);
    void on_text_change_seat(QString text);
    void on_slot_setseatNumber();
    void on_setSeatNumber();
    void on_finish_setSeatNumber();
    void on_showSeatNumber();
    void on_showNotes();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    Ui::CMessageBox *m_pUi;
    QPoint m_pressPoint;
    bool m_isMove;

};
#endif // CMESSAGEBOX_H
