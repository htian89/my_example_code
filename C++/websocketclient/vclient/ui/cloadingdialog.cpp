#include "../config.h"
#include "cloadingdialog.h"
#include "ui_cloadingdialog.h"
#include <QMovie>
#include "../imageconf.h"

CLoadingDialog::CLoadingDialog(QString text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CLoadingDialog),
    m_movie(NULL)
{
    ui->setupUi(this);
//    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint );
    setAttribute(Qt::WA_DeleteOnClose);
    //m_movie = new QMovie(IMAGE_LOADING_MOVIE);
    m_movie = new QMovie(vclient_image.firstloading.data());
    m_movie->setScaledSize(QSize(250, 20));
    if(m_movie==NULL)
        return;
    ui->label_gif->setMovie(m_movie);
    ui->label_text->setText(text);
    ui->label_text->setStyleSheet(STYLE_SHEET_TEXT.data());
    m_movie->stop();
}

CLoadingDialog::~CLoadingDialog()
{
    delete ui;
    ui = NULL;
    if(m_movie!=NULL)
        delete m_movie;
    m_movie = NULL;
}

int CLoadingDialog::setText(QString text)
{
    if(NULL != ui)
    {
//        if(strlen(text.toUtf8().data())>32)
//        {
//            text =text.left(28);
//            text +="...";
//        }
        ui->label_text->setText(text);
    }
    return 0;
}

void CLoadingDialog::setMovieStop(bool stop)
{
    if(stop)
    {
        m_movie->stop();
//        show();
    }
    else
    {
        m_movie->start();
//        hide();
    }
}

int CLoadingDialog::setPos(int x, int y, int w, int h)
{
    QRect rect = geometry();
    setGeometry(x, y, rect.width()>w?rect.width():w, rect.height()>h?rect.height():h);
    return 0;
}
