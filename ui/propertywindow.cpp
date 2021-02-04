#include "propertywindow.h"
#include "popwindow.h"
#include "xatom-helper.h"

PropertyWindow::PropertyWindow(QWidget *parent) : QMainWindow(parent)
{

    int WIDTH = 599;
    int HEIGHT = 580;
//    this->setFixedSize(WIDTH, HEIGHT);

//    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
//    move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);


    initWindow();
    setWindow();

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(mainWid->winId(), hints);

    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    mainWid -> move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    mainWid -> setFixedSize(WIDTH,HEIGHT);
    mainWid -> setAttribute(Qt::WA_ShowModal,true);//模态窗口
    mainWid -> setWindowIcon(QIcon::fromTheme("kylin-printer",QIcon(":/svg/printer_logo.svg")));
    mainWid -> setWindowTitle(tr("打印机驱动"));

}

void PropertyWindow::initWindow()
{
    tipsTimer = new QTimer;

    mainWid = new QWidget();
    titleWid = new QWidget();
    centerWid = new QWidget();
    mainLayout = new QVBoxLayout();
    titleLayout = new QHBoxLayout();
    centerLayout = new QVBoxLayout();
    printTestLayout = new QHBoxLayout();

    titleLabel = new QLabel(this);
    closeBtn = new QToolButton(this);//关闭按钮
    picBtn= new QPushButton(this);//图标
    bigPic= new QPushButton(this);//大图标
    printTestBtn = new QPushButton(this);//打印测试按钮

    printerName = new QLabel(this);//打印机名称标签
    nameLineEdit = new QLineEdit(this);//名称
    printerLocation = new QLabel(this);//打印机位置标签
    locationLineEdit = new QLineEdit(this);//位置
    printerPPD = new QLabel(this);//打印机ppd标签
    ppdLineEdit = new QLineEdit(this);//ppd


    bigPicLayout = new QHBoxLayout;
    nameLayout = new QHBoxLayout;
    locationLayout = new QHBoxLayout;
    driverLayout = new QHBoxLayout;
    bigPicWid = new QWidget;
    nameWid = new QWidget;
    locationWid = new QWidget;
    driverWid = new QWidget;
    printTestWid = new QWidget;
    connect(closeBtn,&QToolButton::clicked,mainWid,&PropertyWindow::hide);
    connect(printTestBtn,&QPushButton::clicked,this,&PropertyWindow::printTestSlot);
    connect(tipsTimer,&QTimer::timeout,this,&PropertyWindow::timeOutSlot);

}

void PropertyWindow::setWindow()
{
    titleLabel->setFixedSize(80,20);
    titleLabel->setText(tr("打印机属性"));
    closeBtn->setFixedSize(30,30);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setAutoRaise(true);
    picBtn->setFixedSize(24,24);
    picBtn->setIconSize(QSize(24,24));
    picBtn->setIcon(QIcon(":/svg/printer_logo.svg"));
    picBtn->setStyleSheet("border-radius:4px;");
    bigPic->setFixedSize(128,128);
    bigPic->setIconSize(QSize(128,128));
    bigPic->setIcon(QIcon(":/svg/printer_logo.svg"));
    bigPic->setStyleSheet("border-radius:4px;");

    printerName->setText(tr("名称"));
    printerLocation->setText(tr("位置"));
    printerPPD->setText(tr("驱动"));
    printTestBtn->setText("打印测试页");
    printTestBtn->setFixedSize(120,36);
    nameLineEdit->setFocusPolicy(Qt::NoFocus);
    nameLineEdit->setFixedSize(400,36);
    locationLineEdit->setFocusPolicy(Qt::NoFocus);
    locationLineEdit->setFixedSize(400,36);
    ppdLineEdit->setFocusPolicy(Qt::NoFocus);
    ppdLineEdit->setFixedSize(400,36);



    titleLayout->addWidget(picBtn);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);
    titleWid->setLayout(titleLayout);
    titleWid->setFixedHeight(40);


    bigPicLayout->addWidget(bigPic);
    bigPicWid->setLayout(bigPicLayout);

    nameLayout->addWidget(printerName);
    nameLayout->addWidget(nameLineEdit);
    nameWid->setLayout(nameLayout);

    locationLayout->addWidget(printerLocation);
    locationLayout->addWidget(locationLineEdit);
    locationWid->setLayout(locationLayout);

    driverLayout->addWidget(printerPPD);
    driverLayout->addWidget(ppdLineEdit);
    driverWid->setLayout(driverLayout);

    printTestLayout->addWidget(printTestBtn);
    printTestWid->setLayout(printTestLayout);

    centerLayout->addStretch();
    centerLayout->addWidget(bigPicWid,0,Qt::AlignCenter);
    centerLayout->addWidget(nameWid,0,Qt::AlignCenter);
    centerLayout->addWidget(locationWid,0,Qt::AlignCenter);
    centerLayout->addWidget(driverWid,0,Qt::AlignCenter);
    centerLayout->addWidget(printTestWid,0,Qt::AlignCenter);
    centerLayout->addStretch();
    centerWid->setLayout(centerLayout);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(centerWid);
    mainLayout->setContentsMargins(4, 0, 4, 4);
    mainLayout->setSpacing(0);
    mainWid->setLayout(mainLayout);

//    mainWid->setObjectName("mainWid");
//    mainWid->setStyleSheet("#mainWid{border:1px solid rgba(0,0,0,0.15);border-radius:6px ;background-color:#FFFFFF;}"); //主窗体圆角
//    this->setWindowFlags((Qt::FramelessWindowHint));                                                                    //设置窗体无边框**加窗管协议后要将此注释调**
//    this->setAttribute(Qt::WA_TranslucentBackground);                                                                   //主窗体透明
//    this->setCentralWidget(mainWid);



}

void PropertyWindow::displayDevice(QString deviceName,QString ppdName)
{
    nameLineEdit->setText(deviceName);
    locationLineEdit->setText(PopWindow::popMutual->manual->driverlocalation->text());
    ppdLineEdit->setText(ppdName);
    qDebug()<<deviceName<<"************"<<ppdName;
    mainWid->show();
}

void PropertyWindow::printTestSlot()
{
    const QString testFileName = "/usr/share/cups/data/testprint";
    bool res = false;
    res = ukuiPrinter::getInstance().printTestPage(nameLineEdit->text().toStdString(),testFileName.toStdString());
    qDebug()<<"===================打印测试页====================";
    qDebug()<<"打印机名称:"<<nameLineEdit->text();
    qDebug()<<res;
    if(res)
    {
        tipsTimer->start(5000);
        Msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机正在启动..."),QMessageBox::Yes);
        Msg->button(QMessageBox::Yes)->setText(tr("确认"));
        Msg->exec();


    }
    else
    {
        tipsTimer->start(5000);
        Msg = new QMessageBox(QMessageBox::Critical,tr("错误"),tr("打印机启动失败,尝试重新添加打印机再次打开!"),QMessageBox::Yes);
        Msg->button(QMessageBox::Yes)->setText(tr("确认"));
        Msg->exec();
    }
}

void PropertyWindow::timeOutSlot()
{
    //打印测试页时5秒后关闭弹窗
    Msg->hide();
    tipsTimer->stop();
}
