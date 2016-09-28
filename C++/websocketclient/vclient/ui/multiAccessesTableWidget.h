#ifndef MULTIACCESSESTABLEWIDGET_H
#define MULTIACCESSESTABLEWIDGET_H
#include <QTableWidget>
#include <QString>

class multiAccessesTableWidget :
        public QTableWidget
{
    Q_OBJECT
public:
    multiAccessesTableWidget(QWidget *parent = 0);

    bool readFile();
    bool writeFile();
    int checkValidation();
    bool IsValidIp(QString str);

    ~multiAccessesTableWidget();

public:
    int m_SelectedRow;

private slots:
    void on_itemPressed(QTableWidgetItem*);
};


#endif // MULTIACCESSESTABLEWIDGET_H
