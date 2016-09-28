#ifndef COMMON_H
#define COMMON_H
#include <QString>
#include <QWidget>

class QLineEdit;
void SetUsernameValidator(QLineEdit *lineEdit_Username);
void SetUrlValidator(QLineEdit *lineEdit_Url);
bool IsValidUrl(QString str);
void SetPortValidator(QLineEdit *lineEdit_Port);
bool IsValidPort(QString str);

int lounchOnSystemStart(const char* pch_appToStart, int i_forAllUser = 0, int i_opType = 0);

void setDialogInCenter(QWidget *);

int saveUserPath();

#endif // COMMON_H
