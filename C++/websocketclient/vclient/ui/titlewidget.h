#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

class SysButton;
enum TITLEBARTYPE{ALLSYSBUTTON, NOMAXMIZE, NOMINIMIZE, ONLYCLOSE, ONLYMINIMIZE};
enum TITLE_IMAGE_TYPE{MINIMIZE, CLOSE, MAXMIZE};

class QHBoxLayout;
class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TitleWidget(TITLEBARTYPE type, QString titleText, QWidget *parent = 0);
	~TitleWidget();
    int setImage(const QString picName, enum TITLE_IMAGE_TYPE imageType);
    int setLayout_title(int left, int top, int right, int bottom);
signals:

public slots:
    void on_btnMaxmize_clicked();

protected:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

private:
    QHBoxLayout *mainLayout;
	QPoint pressedPoint;
	bool isMove;

	SysButton *btnClose;
    SysButton *btnMinimize;
    SysButton *btnMaxmize;
    int windowState;
    QWidget *parentWidget;
};

#endif // TITILEWIDGET_H
