#ifndef PROPERTYWINDOW_H
#define PROPERTYWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>

#include "ukuiPrinter.h"

class PropertyWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PropertyWindow(QWidget *parent = nullptr);
private:

    QMessageBox *Msg;
    QTimer *tipsTimer;

    QWidget *mainWid;
    QWidget *titleWid;
    QWidget *centerWid;
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleLayout;
    QVBoxLayout *centerLayout;

    QLabel *titleLabel;
    QToolButton *closeBtn;//关闭按钮
    QPushButton *printTestBtn;//打印测试页

    QPushButton *picBtn;//图标
    QPushButton *bigPic;//大图标

    QLabel *printerName;//打印机名称标签
    QLineEdit *nameLineEdit;//名称
    QLabel *printerLocation;//打印机位置标签
    QLineEdit *locationLineEdit;//位置
    QLabel *printerPPD;//打印机ppd标签
    QLineEdit *ppdLineEdit;//ppd


    QHBoxLayout *bigPicLayout;
    QHBoxLayout *nameLayout;
    QHBoxLayout *locationLayout;
    QHBoxLayout *driverLayout;
    QHBoxLayout *printTestLayout;
    QWidget *bigPicWid;
    QWidget *nameWid;
    QWidget *locationWid;
    QWidget *driverWid;
    QWidget *printTestWid;


    void initWindow();
    void setWindow();

signals:

public slots:
    void displayDevice(QString deviceName,QString ppdName);
    void printTestSlot();
    void timeOutSlot();

};

#endif // PROPERTYWINDOW_H
