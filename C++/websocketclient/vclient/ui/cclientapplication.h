#ifndef CCLIENTAPPLICATION_H
#define CCLIENTAPPLICATION_H

#include <QApplication>
#ifdef WIN32
#include <windows.h>
#define WM_QUIT_FORUPDATE WM_USER+5
#endif

class CClientApplication : public QApplication
{
    Q_OBJECT
public:
   CClientApplication(int &argc, char **argv);
#ifdef WIN32
   bool winEventFilter ( MSG * msg, long * result );
#endif
protected:
   bool	event ( QEvent * e );

    
signals:
   void appIsActivated(int);
    
public slots:
    
};

#endif // CCLIENTAPPLICATION_H
