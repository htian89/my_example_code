#ifndef CTOOLTIP_H
#define CTOOLTIP_H

#include <QWidget>
//#include "FastQImage.h"

const int MARGIN = 10;
const int ROUNDED = 2000;

class CToolTip : public QWidget
{
public:
    CToolTip( const QPixmap & _pixmap, const QString & _title,
                const QString & _description,
                QWidget * _parent, QWidget * _tool_btn = 0 );

    virtual QSize sizeHint( void ) const;


protected:
    virtual void paintEvent( QPaintEvent * _pe );
    virtual void resizeEvent( QResizeEvent * _re );


private:
    void updateMask( void );

    QImage m_icon;
    QString m_title;
    QString m_description;

    QImage m_bg;

    QWidget * m_toolButton;

} ;

#endif // CTOOLTIP_H
