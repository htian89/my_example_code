#include <QApplication>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include "multiAccessesTableWidget.h"

#include "../common/common.h"
#include "cmessagebox.h"
#include "../common/MultiAccesses.h"
#include <QByteArray>

multiAccessesTableWidget::multiAccessesTableWidget(QWidget *parent) :
     QTableWidget(parent)
{
    // you need to set RowCount and ColumnCount before setting headerlabels
    setRowCount(1);
    setColumnCount(5);

    QString tabStr ="QTabBar::tab{"
            "font: bold 15px;"
            "color: rgb(0, 0, 0);"
            "border: 0px solid #C4C4C3;"
            "border-bottom-color: #C2C7CB;"
            "border-top-left-radius: 0px;"
            "border-top-right-radius: 0px;"
            "min-width: 80px;"
            "margin-left:3px;"
            "min-height: 18px;"
            "padding: 5px;}"
            "QTabBar::tab:selected{"
            "font: bold 15px;"
            "border-color: #9B9B9B;"
            "border-top-left-radius:2px;"
            "border-top-right-radius:2px;"
            "border-bottom-color: #C2C7CB;"
            "background-color: #e9e9e9;}"
            "QTabBar::tab:!selected{"
            "font: bold 15px;"
            "border-radius:4px;"
            "margin-top: 0px;}"
            "QTabBar::tab:first{"
            "margin-left: 20px;"
            "}";

    setStyleSheet(tabStr);

    setHorizontalHeaderLabels(
                QStringList() << tr("Server IP") << tr("Local IP") << tr("Netmask") << tr("Gateway") << tr("DNS")
                );

    connect(this, SIGNAL(itemPressed(QTableWidgetItem*)),
            this, SLOT(on_itemPressed(QTableWidgetItem*)));

    if (!readFile()) {
        exit(-1);
    }

    m_SelectedRow = -1;
}

multiAccessesTableWidget::~multiAccessesTableWidget()
{

}

bool multiAccessesTableWidget::readFile()
{
#ifndef _WIN32
    int savedRowCount = 1;
    MultiAccesses *pMultiAccesses = new MultiAccesses;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    pMultiAccesses->readFile();
    qDebug() << "TWidget size: " << pMultiAccesses->size();
    AccessStruct topHolder;

    if (pMultiAccesses->size() > 0) {
        savedRowCount = pMultiAccesses->size();
    }
    setRowCount(savedRowCount);
    setColumnCount(5);

   for(int row = 0; row < rowCount(); row++) {
        for (int column = 0; column < columnCount(); column++) {
            setItem(row, column, new QTableWidgetItem);
            if (pMultiAccesses->size() > 0) {
                memcpy(&topHolder, &(pMultiAccesses->top()), sizeof(AccessStruct));
            } else {
                memcpy(&topHolder, &(pMultiAccesses->get_default()), sizeof(AccessStruct));
            }
            switch (column) {
            case 0:
                item(row, column)->setText(topHolder.AccessIp);
                qDebug() << item(row, column)->text();
                break;
            case 1:
                item(row, column)->setText(topHolder.ip);
                qDebug() << item(row, column)->text();
                break;
            case 2:
                item(row, column)->setText(topHolder.netmask);
                qDebug() << item(row, column)->text();
                break;
            case 3:
                item(row, column)->setText(topHolder.gateway);
                qDebug() <<item(row, column)->text();
                break;
            case 4:
                item(row, column)->setText(topHolder.dns1);
                qDebug() << item(row, column)->text();
                break;
            default:
                break;
            }
        }
        qDebug() << "row: " << row;
        pMultiAccesses->pop();
   }
    QApplication::restoreOverrideCursor();
#endif

    return true;
}

bool multiAccessesTableWidget::writeFile()
{
    MultiAccesses *pMultiAccesses = new MultiAccesses;
    AccessStruct tmpAccessStruct;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int row = 0; row < rowCount(); row++ ) {
        for (int column = 0; column < columnCount(); column++) {
            switch (column) {
            case 0:
                strcpy(tmpAccessStruct.AccessIp, item(row, column)->text().toStdString().c_str());
                break;
            case 1:
                strcpy(tmpAccessStruct.ip , item(row, column)->text().toStdString().c_str());
                break;
            case 2:
                strcpy(tmpAccessStruct.netmask , item(row, column)->text().toStdString().c_str());
                break;
            case 3:
                strcpy(tmpAccessStruct.gateway , item(row, column)->text().toStdString().c_str());
                break;
            case 4:
                strcpy(tmpAccessStruct.dns1 , item(row, column)->text().toStdString().c_str());
                break;
            default:
                break;
            }
        }
        pMultiAccesses->push(tmpAccessStruct);
    }
    strcpy(tmpAccessStruct.AccessIp , item(0, 0)->text().toStdString().c_str());
    strcpy(tmpAccessStruct.ip , item(0, 1)->text().toStdString().c_str());
    strcpy(tmpAccessStruct.netmask ,item(0, 2)->text().toStdString().c_str());
    strcpy(tmpAccessStruct.gateway , item(0, 3)->text().toStdString().c_str());
    strcpy(tmpAccessStruct.dns1 , item(0, 4)->text().toStdString().c_str());
    pMultiAccesses->set_default(tmpAccessStruct);

    pMultiAccesses->writeFile();

    QApplication::restoreOverrideCursor();
    return true;
}

int multiAccessesTableWidget::checkValidation()
{
    if(this->rowCount() == 0){
        CMessageBox::CriticalBox(
                                    tr("Input at least one Server IP."), this);
        return -1;
    }
    for(int row = 0; row < this->rowCount(); row++){
        if(this->item(row, 0)->text().isEmpty()){
            CMessageBox::TipBox(tr("mush have writen at least Server IP."), this);
            return -1;
        }
    }
//    if ( this->item(0, 0)->text().isEmpty()) {
//        CMessageBox::CriticalBox(
//                            tr("Input at least one vAccess IP."));
//        return -1;
//    }

    for (int row = 0; row < this->rowCount(); row++) {
        for (int column = 0; column < this->columnCount(); column++) {
            if(!this->item(row, column)->text().isEmpty() && !this->IsValidIp(this->item(row, column)->text()))
            {
                CMessageBox::CriticalBox(
                            QString("(") + QString(this->item(row, column)->text()) + QString(")")
                            + tr("IP format not correct."));
                return -1;
            }
        }
    }

    return 0;
}
bool multiAccessesTableWidget::IsValidIp(QString str)
{
    QRegExp rx("^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$");
    QRegExpValidator validator(rx,NULL);
    int pos = 0;
    if(validator.validate(str,pos) == QValidator::Acceptable)
        return true;
    return false;
}

void multiAccessesTableWidget::on_itemPressed(QTableWidgetItem*)
{
    m_SelectedRow = currentItem()->row();
    qDebug() << "m_SelectedRow " << m_SelectedRow << endl;
}
