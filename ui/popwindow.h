#ifndef POPWINDOW_H
#define POPWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>

#include "deviceMonitor.h"
#include "manualinstallwindow.h"

#include "cupsconnection4ppds.h"
#include "findppdsthread.h"
#include "matchppdsthread.h"
class PopWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PopWindow(QWidget *parent = nullptr);

private:
    ManualInstallWindow *manual; //手动安装驱动界面
    DeviceInformation printer;

    QWidget *mainWid;           //弹窗主Wid
    QWidget *titleWid;          //标题栏Wid
    QWidget *middleWid;         //中间Wid
    QWidget *bottomWid;         //底部Wid
    QWidget *manualButtonWid;   //手动安装按钮Wid
    QWidget *messageWid;        //消息提示Wid：两行标签
    QWidget *monitorMessageWid; //检测消息Wid
    QWidget *installMessageWid; //安装消息Wid

    QHBoxLayout *titleLayout;  //标题栏布局
    QHBoxLayout *middleLayout; //中间布局
    QHBoxLayout *bottomLayout; //底部布局
    QHBoxLayout *manualButtonLayout;
    QVBoxLayout *messageLayout;        //消息提示布局
    QHBoxLayout *monitorMessageLayout; //检测消息布局
    QHBoxLayout *installMessageLayout; //安装消息布局
    QVBoxLayout *mainLayout;           //弹窗主布局

    QStackedWidget *installStateStack; //安装状态stack
    QStackedWidget *buttonStack;       //按钮stack

    QToolButton *titlePictureBtn;
    QLabel *appNameLabel;
    QLabel *seatlb; //占位
    QPushButton *closeButton;
    QPushButton *picButton;
    QLineEdit *isMonitorEdit;
    QPushButton *loadPic;   //加载中图标
    QLabel *isInstallLabel; //安装中标签
    QLabel *isSuccesslb;    //安装成功
    QTimer *timer;
    QTimer *findTime; //查找所有pdd
    int timeTag = 0;

    QPushButton *printTestBtn;     //打印测试页按钮
    QPushButton *deviceViewBtn;    //设备查看按钮
    QPushButton *manualInstallBtn; //手动安装按钮

    bool isExistDriver = false;
    bool isSucceed = false;
    int searchResult = 0;
    int i = 0;
    void initControls();                       //初始化控件
    void setControls(DeviceInformation, bool); //设置控件属性值
    void initPopWindow();                      //初始化PopWindow布局
    void setPopWindow();                       //设置PopWindow布局
    void loadingPicDisplay();                  //加载图标动态显示

    ukuiUsbPrinter m_printer;
    myMap mymap;
    bool canFindPPD = false;
    /***********************链接CUPS查找PPDS部分用的**************************/
    http_t *newHttp = nullptr;
    ipp_t *ppdRequest = nullptr;
    ipp_t *cupsAnswer = nullptr;
    ipp_attribute_t *attr = nullptr;

    QMap<QStringList, QString> originPPDs = {}; //从CUPS请求到的原始PPDS列表

    FindPPDsThread *cmdFindPPDs = nullptr; //从CUPS链接获取PPDS列表的线程
    QThread *pFindPPDs = nullptr;
    MatchPPDsThread *cmdMatchPPDs = nullptr; //从PPDS列表匹配打印机名称的线程
    QThread *pMatchPPDs = nullptr;

    bool ppdsMapisOK = false;                                  //是否有精准匹配的PPDS
    QMap<QString, QMap<QString, PPDsAndAttr>> originData = {}; //QMap<厂商名，QMap<型号，PPDS属性数据结构>>的原始数据
    /***********************链接CUPS查找PPDS部分用的**************************/
    
signals:

    void monitorDriver(DeviceInformation, bool);
    void printSignal(QStringList); //打印信号携带ppd文件信息

    void signalFindPPDsThread();                                   //发送开始连接CUPS链接，查找获取已有的PPDS
    void signalMatchPPDsThread(QString printername,myMap origin, int devicetype); //获取打印机名字，把CUPS链接传回的PPDS一并输入，进行检索
    void signalClickManualButton(QString, QString, QString);
private slots:

    void popDisplay(DeviceInformation, bool); //弹窗显示
    void print();                             //发送打印测试页
    void showManualWindow();                  //显示手动安装
    void gotAllHandledPPDs(myMap);
    void matchResultSlot(resultMap res);
};

#endif // POPWINDOW_H
