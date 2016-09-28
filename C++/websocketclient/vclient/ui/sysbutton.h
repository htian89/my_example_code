#ifndef SYSBUTTON_H
#define SYSBUTTON_H

#include <QPushButton>

class SysButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SysButton(QString picName, QString tipText, QWidget *parent = 0);
    void setPixmap(QString picName);
    void setEnabled_ex(bool);

signals:

public slots:

private:
	enum buttonStatus{NORMAL, ENTER, PRESS, NOSTATUS};

	buttonStatus status;
    QPixmap pixmap;

	int btnWidth;
	int btnHeight;
    bool m_allowPaint;

protected:
	void paintEvent(QPaintEvent *);
	void enterEvent(QEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void leaveEvent(QEvent *);

};

#endif // SYSBUTTON_H
