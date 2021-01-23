#ifndef SUCCEDFAILWINDOW_H
#define SUCCEDFAILWINDOW_H

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
#include <QStackedWidget>

#include "manualinstallwindow.h"


class SuccedFailWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SuccedFailWindow(QWidget *parent = nullptr);

    QPushButton *printTestBtn;//打印测试页按钮
private:

    QMessageBox *Msg;
    QTimer *isPrintTimer;

    QWidget *mainWid;
    QWidget *titleWid;
    QWidget *centerWid;
    QWidget *bottom_1Wid;//安装成功时的按钮组Wid
    QWidget *bottom_2Wid;//安装失败时的按钮组Wid


    QVBoxLayout *mainLayout;
    QHBoxLayout *titleLayout;
    QVBoxLayout *centerLayout;
    QHBoxLayout *bottom_1Layout;//安装成功时的按钮组Layout
    QHBoxLayout *bottom_2Layout;//安装失败时的按钮组Layout

    QLabel *titleLabel;
    QToolButton *closeBtn;
    QPushButton *picBtn;//对号与叉子图标
    QLabel *printerName;//打印机名称
    QLineEdit *messageLineEdit;//消息

    QPushButton *viewDeviceBtn;//查看设备
    QPushButton *reinstallBtn;//重新安装
    QPushButton *cloudPrintBtn;//使用云打印

    QStackedWidget *twoButtonStackWid;

    bool isSucceed = false;

    QString printerDeviceName ;

    void init();
    void setWindow();

public slots:
    void onShowSucceedFailWindow(QString printer,bool isSuccess);
    void printSlot();
    void timeOutSlot();

    void showManualWindow();

signals:

};

#endif // SUCCEDFAILWINDOW_H
