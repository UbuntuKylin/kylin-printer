#ifndef MANUALINSTALLWINDOW_H
#define MANUALINSTALLWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QComboBox>
#include <QTimer>
#include <QTime>
#include <QStackedWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "ukuiApt.h"
#include "ukuiPrinter.h"



class ManualInstallWindow : public QMainWindow
{
    Q_OBJECT
public:

    explicit ManualInstallWindow(QWidget *parent = nullptr);

    QPushButton *addBtn;//添加
    int debCount = 0;

    QLabel *Namelb;//名称
    QLineEdit *printerName;//打印机名称
private:


    QWidget *mainWid;//主Wid
    QVBoxLayout *mainLayout;//主布局
    QWidget *titleWid;//标题栏Wid
    QHBoxLayout *titleLayout;//标题栏布局
    QWidget *contentWid;//内容Wid
    QVBoxLayout *contentLayout;//内容布局

    QWidget *addDriverWid;//添加驱动的Wid
    QVBoxLayout *addDriverLayout;//拖拽deb的布局
    QWidget *messageWid;//驱动主要信息显示Wid
    QVBoxLayout *messageLayout;//驱动主要信息布局
    QWidget *buttomWid;//取消_添加按钮Wid
    QHBoxLayout *buttomLayout;//取消_添加按钮布局
    QWidget *loadWid;//debWid
    QHBoxLayout *loadLayout;//deb布局

    QWidget *printerNameWid;//打印机名称Wid
    QHBoxLayout *printerNameLayout;//打印机名称布局
    QWidget *locationWid;//位置Wid
    QHBoxLayout *locationLayout;//位置布局
    QWidget *pddFileWid;//pdd文件Wid
    QHBoxLayout *pddFileLayout;//pdd文件布局

    QPushButton *titlePic;//手动安装驱动标题栏图标
    QLabel *titleLabel;//手动安装驱动标签
    QPushButton *closeBtn;//手动安装界面关闭按钮

    QPushButton *addLocalDriverBtn;//添加本地驱动按钮
    QLabel *dropTipsLabel;//提示"添加一个本地驱动到此"
    QLabel *remindUrlLabel;//提醒去Url
    QLabel *downloadUrlLabel;//下载链接
    QPushButton *debPic;//deb包图片
    QLabel *debName;//deb包名

    QTimer *installingTimer;   //安装中延时...
    QPushButton *installPic;   //安装中。。。图标
    int j= 0;


    //打印机名称在public中

    QLabel *locationlb;//位置
    QLineEdit *driverlocalation;//驱动位置
    QLabel *ppdlb;//pdd文件
    QLineEdit *ppd;//pdd文件名
    QComboBox *dropDownList;//下拉框存ppd

    QPushButton *cancelBtn;//取消

    bool debSuccess = false;
    int matchTag = 0;

    ukuiUsbPrinter m_printer;

    void initManualControls();
    void setManualControls();
    void initManualWindow();
    void setManualWindow();

    void displayInstalling();//显示正在安装的图片

protected:

    void dropEvent(QDropEvent *);//放手手
    void dragEnterEvent(QDragEnterEvent *event);//拖拽

signals:
    void updatePpdList();

    void manualAddSignal(QString printerName,bool isSuccess);

    void matchPpdList();

private slots:
    void onShowManualWindow(QString vendor, QString product, QString uri,QStringList ppdList, bool isExact);
    void onPackageInstalled(ukuiInstallStatus status);

    void manualAddPrinter();

    void addLocalDriverSlot();

public slots:

    void alreadyInstallSlot();

    void matchSuccessSlot(QString printerName,QString position,QStringList ppdList);


private:
    QString m_vendor;
    QString m_product;
    QString m_uri;
    QStringList m_ppdList;
    bool m_isExact;
    ukuiApt *m_apt;
};

#endif // MANUALINSTALLWINDOW_H
