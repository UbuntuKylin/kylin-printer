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
    QWidget *bottomWid;

    QVBoxLayout *mainLayout;
    QHBoxLayout *titleLayout;
    QVBoxLayout *centerLayout;
    QHBoxLayout *bottomLayout;

    QLabel *titleLabel;
    QToolButton *closeBtn;
    QPushButton *picBtn;//对号与叉子图标
    QLabel *printerName;//打印机名称
    QLineEdit *messageLineEdit;//消息

    QPushButton *viewDeviceBtn;//查看设备

    bool isSucceed = false;

    QString printerDeviceName ;

    void init();
    void setWindow();

public slots:
    void onShowSucceedFailWindow(QString printer,bool isSuccess);
    void printSlot();
    void timeOutSlot();

signals:

};

#endif // SUCCEDFAILWINDOW_H
