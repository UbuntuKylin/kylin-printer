#include "succedfailwindow.h"
#include "popwindow.h"

SuccedFailWindow::SuccedFailWindow(QWidget *parent) : QMainWindow(parent)
{

    int WIDTH = 420;
    int HEIGHT = 320;
    this->setFixedSize(WIDTH, HEIGHT);
    this->setAttribute(Qt::WA_ShowModal,true);//模态窗口
    setWindowTitle(tr("打印机驱动"));
    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    init();
    setWindow();


    connect(closeBtn,&QToolButton::clicked,this,&SuccedFailWindow::hide);
    connect(printTestBtn,&QPushButton::clicked,this,&SuccedFailWindow::printSlot);
    connect(viewDeviceBtn,&QPushButton::clicked,PopWindow::popMutual,&PopWindow::deviceNameSlot);//显示打印机属性界面
    connect(viewDeviceBtn,&QPushButton::clicked,this,&SuccedFailWindow::hide);//隐藏此弹窗


}

void SuccedFailWindow::init()
{
    titleLabel = new QLabel(this);
    closeBtn = new QToolButton(this);
    picBtn = new QPushButton(this);//对号与叉子图标
    printerName = new QLabel(this);//打印机名称
    messageLineEdit = new QLineEdit(this);//消息
    printTestBtn = new QPushButton(this);//打印测试页按钮
    viewDeviceBtn = new QPushButton(this);//查看设备

    mainWid = new QWidget();
    titleWid = new QWidget();
    centerWid = new QWidget();
    bottomWid = new QWidget();

    mainLayout = new QVBoxLayout();
    titleLayout = new QHBoxLayout();
    centerLayout = new QVBoxLayout();
    bottomLayout = new QHBoxLayout();
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

    printerName->setFixedSize(276,20);

    messageLineEdit->setFixedSize(173,31);

    printTestBtn->setFixedSize(120,36);
    printTestBtn->setText(tr("打印测试"));
    viewDeviceBtn->setFixedSize(120,36);
    viewDeviceBtn->setText(tr("查看设备"));

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->setContentsMargins(8, 8, 8, 0);
    titleLayout->addWidget(closeBtn);
    titleWid->setLayout(titleLayout);

    centerLayout->addStretch();
    centerLayout->addWidget(picBtn,0,Qt::AlignCenter);
    centerLayout->addWidget(printerName,0,Qt::AlignCenter);
//    centerLayout->addWidget(messageLineEdit,0,Qt::AlignCenter);
    centerLayout->addStretch();
    centerWid->setLayout(centerLayout);

    bottomLayout->addWidget(printTestBtn);
    bottomLayout->addWidget(viewDeviceBtn);
    bottomWid->setLayout(bottomLayout);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(centerWid);
    mainLayout->addWidget(bottomWid,0,Qt::AlignRight);
    mainLayout->setMargin(0);
    mainWid->setLayout(mainLayout);

    mainWid->setObjectName("mainWid");
    mainWid->setStyleSheet("#mainWid{border:1px solid rgba(0,0,0,0.15);border-radius:6px ;background-color:#FFFFFF;}"); //主窗体圆角
    this->setWindowFlags((Qt::FramelessWindowHint));                                                                    //设置窗体无边框**加窗管协议后要将此注释调**
    this->setAttribute(Qt::WA_TranslucentBackground);                                                                   //主窗体透明
    this->setCentralWidget(mainWid);

}

void SuccedFailWindow::onShowSucceedFailWindow(QString printer,bool isSuccess)
{
    if(printer =="")
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机名称不可为空!"),QMessageBox::Yes);

        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        this->hide();
        return ;
    }
    if(isSuccess)
    {

        qDebug()<<"打印机"<<printer;
        printerName->setText("打印机"+printer+"安装成功!");
        picBtn->setIcon(QIcon::fromTheme("ukui-dialog-success"));

        printerDeviceName = printer;
        printTestBtn->show();




    }
    else
    {
        printerName->setText("打印机"+printer+"安装失败!");
        picBtn->setIcon(QIcon::fromTheme("dialog-error"));
        printTestBtn->hide();
    }

    show();
}

void SuccedFailWindow::printSlot()
{
    qDebug() << "StringList的ppd文件";
    //参数为StringList的ppd文件
    //    emit printSignal(QStringList);
    const QString testFileName = "/usr/share/cups/data/testprint";
    bool res = false;
    res = ukuiPrinter::getInstance().printTestPage(printerDeviceName.toStdString(),testFileName.toStdString());
    qDebug()<<"=======================打印测试页====================";
    qDebug()<<printerDeviceName;
    qDebug()<<res;
    if(res)
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机正在启动..."),QMessageBox::Yes);
        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
    }
    else
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Critical,tr("错误"),tr("打印机启动失败,尝试关闭窗口再次打开!"),QMessageBox::Yes);
        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
    }

}

