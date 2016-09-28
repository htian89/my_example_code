#ifndef CLOADINGDIALOG_H
#define CLOADINGDIALOG_H

//#include <QDialog>
#include <QWidget>
namespace Ui {
class CLoadingDialog;
}

class CLoadingDialog : public QWidget
{
    Q_OBJECT
    
public:
    explicit CLoadingDialog(QString text, QWidget *parent = 0);
    ~CLoadingDialog();
    int setText(QString text);
    int setPos(int x, int y, int w =0, int h=0);
    void setMovieStop(bool stop);
    
private:
    Ui::CLoadingDialog *ui;
    QMovie *m_movie;
};

#endif // CLOADINGDIALOG_H
