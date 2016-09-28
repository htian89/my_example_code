#include "ctooltip.h"
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QToolBar>

CToolTip::CToolTip( const QPixmap & _pixmap, const QString & _title,
                const QString & _description,
                QWidget * _parent, QWidget * _tool_btn ) :
    QWidget( _parent, Qt::ToolTip ),
    m_icon( _pixmap.toImage().scaled( 72, 72 ) ),//m_icon( FastQImage( _pixmap ).scaled( 72, 72 ) ),
    m_title( _title ),
    m_description( _description ),
    m_toolButton( _tool_btn )
{
    setAttribute( Qt::WA_DeleteOnClose, TRUE );
    setAttribute( Qt::WA_NoSystemBackground, TRUE );

    resize( sizeHint() );
    updateMask();
}




QSize CToolTip::sizeHint( void ) const
{
    QFont f = font();
    f.setBold( TRUE );
    int title_w = QFontMetrics( f ).width( m_title );
    QRect desc_rect = fontMetrics().boundingRect( QRect( 0, 0, 250, 100 ),
                    Qt::TextWordWrap, m_description );

    return QSize( MARGIN + m_icon.width() + MARGIN +
                qMax( title_w, desc_rect.width() ) + MARGIN,
            MARGIN + qMax( m_icon.height(), fontMetrics().height() +
                        MARGIN + desc_rect.height() ) +
                                MARGIN );
}




void CToolTip::paintEvent( QPaintEvent * _pe )
{
    QPainter p( this );
    p.drawImage( 0, 0, m_bg );
}




void CToolTip::resizeEvent( QResizeEvent * _re )
{
    const QColor color_frame = QColor( 48, 48, 48 );
    m_bg = QImage( size(), QImage::Format_ARGB32 );
    m_bg.fill( color_frame.rgba() );
    QPainter p( &m_bg );
    p.setRenderHint( QPainter::Antialiasing );
    QPen pen( color_frame );
    pen.setWidthF( 1.5 );
    p.setPen( pen );
    QLinearGradient grad( 0, 0, 0, height() );
    const QColor color_top = palette().color( QPalette::Active,
                        QPalette::Window ).light( 100 );
    grad.setColorAt( 0, color_top );
    grad.setColorAt( 1, palette().color( QPalette::Active,
                        QPalette::Window ).
                            light( 80 ) );
    p.setBrush( grad );
    p.drawRoundRect( 0, 0, width() - 1, height() - 1,
                    ROUNDED / width(), ROUNDED / height() );
    if( m_toolButton )
    {
        QPoint pt = m_toolButton->mapToGlobal( QPoint( 0, 0 ) );
        p.setPen( color_top );
        p.setBrush( color_top );
        p.setRenderHint( QPainter::Antialiasing, FALSE );
        p.drawLine( pt.x() - x(), 0,
                pt.x() + m_toolButton->width() - x() - 2, 0 );
        const int dx = pt.x() - x();
        p.setRenderHint( QPainter::Antialiasing, TRUE );
        if( dx < 10 && dx >= 0 )
        {
            p.setPen( pen );
            p.drawImage( dx+1, 0, m_bg.copy( 20, 0, 10-dx, 10 ) );
            p.drawImage( dx, 0, m_bg.copy( 0, 10, 1, 10-dx*2 ) );
        }
    }
    p.setPen( Qt::black );

    p.drawImage( MARGIN, MARGIN, m_icon );
    QFont f = p.font();
    f.setBold( TRUE );
    p.setFont( f );
    const int title_x = MARGIN + m_icon.width() + MARGIN;
    const int title_y = MARGIN + fontMetrics().height() - 2;
    p.drawText( title_x, title_y, m_title );

    f.setBold( FALSE );
    p.setFont( f );
    p.drawText( QRect( title_x, title_y + MARGIN,
                    width() - MARGIN - title_x,
                    height() - MARGIN - title_y ),
                    Qt::TextWordWrap, m_description );

    updateMask();
    QWidget::resizeEvent( _re );
}




void CToolTip::updateMask( void )
{
    // as this widget has not a rectangular shape AND is a top
    // level widget (which doesn't allow painting only particular
    // regions), we have to set a mask for it
    QBitmap b( size() );
    b.clear();

    QPainter p( &b );
    p.setBrush( Qt::color1 );
    p.setPen( Qt::color1 );
    p.drawRoundRect( 0, 0, width() - 1, height() - 1,
                    ROUNDED / width(), ROUNDED / height() );

    if( m_toolButton )
    {
        QPoint pt = m_toolButton->mapToGlobal( QPoint( 0, 0 ) );
        const int dx = pt.x()-x();
        if( dx < 10 && dx >= 0 )
        {
            p.fillRect( dx, 0, 10, 10, Qt::color1 );
        }
    }

    setMask( b );
}
