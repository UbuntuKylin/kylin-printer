#include "succedfailwindow.h"
#include "popwindow.h"

#include "xatom-helper.h"

SuccedFailWindow::SuccedFailWindow(QWidget *parent) : QMainWindow(parent)
{

    int WIDTH = 420;
    int HEIGHT = 320;
//    this->setFixedSize(WIDTH, HEIGHT);
//    this->setAttribute(Qt::WA_ShowModal,true);//模态窗口



    init();
    setWindow();

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(mainWid->winId(), hints);

    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    mainWid -> move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    mainWid -> setFixedSize(WIDTH,HEIGHT);
//    mainWid -> setAttribute(Qt::WA_ShowModal,true);//模态窗口
    mainWid -> setWindowIcon(QIcon(":/svg/printer_logo.svg"));
    mainWid -> setWindowTitle(tr("打印机驱动"));


    connect(isPrintTimer,&QTimer::timeout,this,&SuccedFailWindow::timeOutSlot);
    connect(closeBtn,&QToolButton::clicked,mainWid,&SuccedFailWindow::hide);
    connect(printTestBtn,&QPushButton::clicked,this,&SuccedFailWindow::printSlot);
    connect(reinstallBtn,&QPushButton::clicked,this,&SuccedFailWindow::showManualWindow);//重新安装则跳转到手动安装界面
    connect(viewDeviceBtn,&QPushButton::clicked,PopWindow::popMutual,&PopWindow::deviceNameSlot);//显示打印机属性界面
    connect(viewDeviceBtn,&QPushButton::clicked,mainWid,&SuccedFailWindow::hide);//隐藏此弹窗


}


void SuccedFailWindow::showManualWindow()
{
    PopWindow::popMutual->manual->mainWid->show();
    this->mainWid->hide();
}

void SuccedFailWindow::init()
{
    titleLabel      = new QLabel(this);
    closeBtn        = new QToolButton(this);
    picBtn          = new QPushButton(this);//对号与叉子图标
    printerName     = new QLabel(this);//打印机名称
    messageLineEdit = new QLineEdit(this);//消息
    printTestBtn    = new QPushButton(this);//打印测试页按钮
    viewDeviceBtn   = new QPushButton(this);//查看设备
    reinstallBtn    = new QPushButton(this);//重新安装
    cloudPrintBtn   = new QPushButton(this);//使用云打印

    mainWid      = new QWidget();
    titleWid     = new QWidget();
    centerWid    = new QWidget();
    bottom_1Wid    = new QWidget();//安装成功时的按钮组Wid
    bottom_2Wid    = new QWidget();//安装失败时的按钮组Wid
    twoButtonStackWid = new QStackedWidget();//安装成功与失败的按钮组的堆叠Wid

    mainLayout   = new QVBoxLayout();
    titleLayout  = new QHBoxLayout();
    centerLayout = new QVBoxLayout();
    bottom_1Layout = new QHBoxLayout();//安装成功时的按钮组layout
    bottom_2Layout = new QHBoxLayout();//安装失败时的按钮组layout

    isPrintTimer = new QTimer;
}

void SuccedFailWindow::setWindow()
{
    titleLabel->setFixedSize(80,20);
    titleLabel->setText(tr("打印机驱动"));
    closeBtn->setFixedSize(30,30);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setAutoRaise(true);

    picBtn->setFixedSize(64,64);
    picBtn->setIconSize(QSize(64,64));
    picBtn->setIcon(QIcon::fromTheme("ukui-dialog-success"));
    picBtn->setStyleSheet("border-radius:4px;");

    printerName->setFixedSize(240,30);

    messageLineEdit->setFixedSize(173,31);

    printTestBtn->setFixedSize(120,36);
    printTestBtn->setText(tr("打印测试"));
    viewDeviceBtn->setFixedSize(120,36);
    viewDeviceBtn->setText(tr("查看设备"));
    reinstallBtn->setFixedSize(120,36);
    reinstallBtn->setText(tr("重新安装"));
    cloudPrintBtn->setFixedSize(120,36);
    cloudPrintBtn->setText(tr("使用云打印"));

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);
    titleLayout->addWidget(closeBtn);
    titleWid->setLayout(titleLayout);

    centerLayout->addStretch();
    centerLayout->addWidget(picBtn,0,Qt::AlignCenter);
    centerLayout->addWidget(printerName,0,Qt::AlignCenter);
//    centerLayout->addWidget(messageLineEdit,0,Qt::AlignCenter);
    centerLayout->addStretch();
    centerWid->setLayout(centerLayout);

    bottom_1Layout->addWidget(printTestBtn);
    bottom_1Layout->addWidget(viewDeviceBtn);
    bottom_1Wid->setLayout(bottom_1Layout);

    bottom_2Layout->addWidget(reinstallBtn);
//    bottom_2Layout->addWidget(cloudPrintBtn);
    bottom_2Wid->setLayout(bottom_2Layout);

    twoButtonStackWid->addWidget(bottom_1Wid);
    twoButtonStackWid->addWidget(bottom_2Wid);
    twoButtonStackWid->setFixedSize(300,60);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(centerWid);
    mainLayout->addWidget(twoButtonStackWid,0,Qt::AlignRight);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(0);
    mainWid->setLayout(mainLayout);

//    mainWid->setObjectName("mainWid");
//    mainWid->setStyleSheet("#mainWid{border:1px solid rgba(0,0,0,0.15);border-radius:6px ;background-color:#FFFFFF;}"); //主窗体圆角
//    this->setWindowFlags((Qt::FramelessWindowHint));                                                                    //设置窗体无边框**加窗管协议后要将此注释调**
//    this->setAttribute(Qt::WA_TranslucentBackground);                                                                   //主窗体透明
//    this->setCentralWidget(mainWid);

}

void SuccedFailWindow::onShowSucceedFailWindow(QString printer,bool isSuccess)
{
    if(printer =="")
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机名称不可为空!"),QMessageBox::Yes);

        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        mainWid->hide();
        return ;
    }
//    isSuccess = false;//测试时使用
    if(isSuccess)
    {
        qDebug()<<"打印机"<<printer;
        printerName->setText("打印机"+printer+"安装成功!");
        picBtn->setIcon(QIcon::fromTheme("ukui-dialog-success"));
        printerDeviceName = printer;
        twoButtonStackWid->setCurrentIndex(0);
    }
    else
    {
        printerName->setText("打印机"+printer+"安装失败!");
        picBtn->setIcon(QIcon::fromTheme("dialog-error"));
        twoButtonStackWid->setCurrentIndex(1);
    }

    mainWid->show();
}

void SuccedFailWindow::timeOutSlot()
{
    //打印测试页时5秒后关闭弹窗
    Msg->close();
    isPrintTimer->stop();
}

void SuccedFailWindow::printSlot()
{
    qDebug() << "StringList的ppd文件";
    //参数为StringList的ppd文件
    //    emit printSignal(QStringList);
    const QString testFileName = "/usr/share/cups/data/testprint";
    bool res = false;
    res = ukuiPrinter::getInstance().printTestPage(printerDeviceName.toStdString(),testFileName.toStdString());
    qDebug()<<"===================打印测试页====================";
    qDebug()<<printerDeviceName;
    qDebug()<<res;
    if(res)
    {
        isPrintTimer->start(5000);
        Msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机正在启动..."),QMessageBox::Yes);
        Msg->button(QMessageBox::Yes)->setText(tr("确认"));
        Msg->exec();


    }
    else
    {
        isPrintTimer->start(5000);
        Msg = new QMessageBox(QMessageBox::Critical,tr("错误"),tr("打印机启动失败,尝试关闭窗口再次打开!"),QMessageBox::Yes);
        Msg->button(QMessageBox::Yes)->setText(tr("确认"));
        Msg->exec();
    }

}

