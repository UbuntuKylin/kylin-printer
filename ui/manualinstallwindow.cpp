#include "manualinstallwindow.h"

ManualInstallWindow::ManualInstallWindow(QWidget *parent) : QMainWindow(parent), m_apt(nullptr)
{
    int WIDTH = 620;
    int HEIGHT = 660;
    this->setFixedSize(WIDTH, HEIGHT);
    setWindowTitle(tr("手动安装打印机驱动"));

    setAcceptDrops(true);

    this->setWindowIcon(QIcon(":/svg/printer_logo.svg"));
    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    initManualControls();
    initManualWindow();
    setManualControls();
    setManualWindow();

    connect(closeBtn, &QPushButton::clicked, this, &ManualInstallWindow::close);
}

void ManualInstallWindow::dragEnterEvent(QDragEnterEvent *event) //拖进事件
{

    // 判断拖拽文件类型，文件名 接收该动作
    if (event->mimeData()->hasFormat("text/uri-list"))
    {
        //        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        qDebug() << event->mimeData()->text();
    }
    QWidget::dragEnterEvent(event);
}

void ManualInstallWindow::dropEvent(QDropEvent *event) //放下事件
{
    auto urls = event->mimeData()->urls();
    if (urls.isEmpty())
    {
        return;
    }
    QStringList localpath;
    for (auto &url : urls)
    {
        localpath << url.toLocalFile();
    }

    qDebug() << "localpath中的元素个数:" << localpath.count();
    qDebug() << localpath;
    if (!localpath.isEmpty())
    {
        //        emit fromFilemanager(localpath);
        //        emit addFile(localpath);
        if (m_apt == nullptr)
        {
            /* code */
            m_apt = new ukuiApt(localpath.at(0));
            if (m_apt != nullptr)
            {
                connect(m_apt, &ukuiApt::reportInstallStatus,
                        this, &ManualInstallWindow::onPackageInstalled);
                m_apt->install();
            }
        }
    }
}

void ManualInstallWindow::initManualControls()
{
    //标题栏
    titlePic = new QPushButton(this);
    titleLabel = new QLabel(this);
    closeBtn = new QPushButton(this);

    //中间拖拽区间
    addLocalDriverBtn = new QPushButton(this); //添加本地驱动按钮
    dropTipsLabel = new QLabel(this);          //提示"添加一个本地驱动到此"
                                               //    remindUrlLabel  = new QLabel(this);//提醒去Url
    downloadUrlLabel = new QLabel(this);       //下载链接

    //名称行
    Namelb = new QLabel(this);       //名称
    printName = new QLineEdit(this); //打印机名称
    //位置行
    locationlb = new QLabel(this);          //位置
    driverlocalation = new QLineEdit(this); //驱动位置
    //ppd行
    ppdlb = new QLabel(this);  //ppd文件
    ppd = new QLineEdit(this); //ppd文件名

    //取消和添加按钮
    cancelBtn = new QPushButton(this);
    addBtn = new QPushButton(this);
}

void ManualInstallWindow::setManualControls()
{
    //标题栏
    titlePic->setIcon(QIcon(":/svg/printer_logo.svg"));
    titlePic->setFixedSize(24, 24);
    titlePic->setIconSize(QSize(24, 24));
    titlePic->setStyleSheet("border-radius:4px;");
    titleLabel->setFixedSize(180, 20);
    titleLabel->setText(tr("手动安装打印机驱动"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet("QPushButton{border-radius:4px;}"
                            "QPushButton:hover{background-color:#F86457;}"
                            "QPushButton:pressed{background-color:#E44C50;}");

    //添加处控件
    addLocalDriverBtn->setFixedSize(120, 36);
    addLocalDriverBtn->setText("添加本地驱动");
    dropTipsLabel->setFixedSize(120, 36);
    dropTipsLabel->setText("拖拽一个驱动到此");
    downloadUrlLabel->setFixedSize(300, 36);
    downloadUrlLabel->setText("或者您可以去相关网址下载:www.kylinos.cn");
    addLocalDriverBtn->setStyleSheet("QPushButton{border-radius:4px;background-color:#3790FA;color:white;border:1px solid rgba(0,0,0,0.15);}"
                                     "QPushButton::hover{color:black;}");

    //三行lineEdit
    Namelb->setText("名称");
    printName->setFixedSize(442, 36);
    printName->setText("HP-printer-lasevcP1106");
    locationlb->setText("位置");
    driverlocalation->setFixedSize(442, 36);
    driverlocalation->setText("/user/share/bjc/...");
    ppdlb->setText("驱动");
    ppd->setFixedSize(442, 36);
    ppd->setText("手动选择驱动方案");

    //取消和添加按钮
    cancelBtn->setFixedSize(120, 36);
    cancelBtn->setText("取消");
    addBtn->setFixedSize(120, 36);
    addBtn->setText("添加");
}

void ManualInstallWindow::initManualWindow()
{
    //Wid
    titleWid = new QWidget();     //标题栏Wid
    contentWid = new QWidget();   //内容Wid
    addDriverWid = new QWidget(); //添加驱动的Wid
    messageWid = new QWidget();   //主要信息显示Wid
    buttomWid = new QWidget();    //底部按钮Wid
    mainWid = new QWidget();      //主WId

    //布局
    titleLayout = new QHBoxLayout();     //标题栏布局
    contentLayout = new QVBoxLayout();   //内容布局
    addDriverLayout = new QVBoxLayout(); //添加拖拽布局
    messageLayout = new QVBoxLayout();   //驱动主要信息布局
    buttomLayout = new QHBoxLayout();    //底部按钮布局
    mainLayout = new QVBoxLayout();      //主布局

    printerNameWid = new QWidget(); //打印机名称Wid
    locationWid = new QWidget();    //位置名称Wid
    pddFileWid = new QWidget();     //ppd文件名Wid

    printerNameLayout = new QHBoxLayout(); //打印机名称布局
    locationLayout = new QHBoxLayout();    //位置布局
    pddFileLayout = new QHBoxLayout();     //ppd布局
}

void ManualInstallWindow::setManualWindow()
{
    //标题栏布局
    titleLayout->addWidget(titlePic);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);
    //    titleLayout ->setContentsMargins(8,8,8,0);
    titleWid->setLayout(titleLayout);
    titleWid->setFixedHeight(35);

    //添加拖拽界面
    addDriverLayout->addWidget(addLocalDriverBtn, 0, Qt::AlignCenter);
    addDriverLayout->addWidget(dropTipsLabel, 0, Qt::AlignCenter);
    addDriverLayout->addWidget(downloadUrlLabel, 0, Qt::AlignCenter);
    addDriverWid->setLayout(addDriverLayout);
    addDriverWid->setFixedSize(554, 328);
    addDriverWid->setStyleSheet("background-color: #F2F2F2;");

    //三行lineedit布局
    printerNameLayout->addWidget(Namelb);
    printerNameLayout->addWidget(printName);
    printerNameWid->setLayout(printerNameLayout);
    printerNameWid->setFixedSize(554, 50);
    printerNameWid->setStyleSheet("background-color: #F2F2F2;");

    locationLayout->addWidget(locationlb);
    locationLayout->addWidget(driverlocalation);
    locationWid->setLayout(locationLayout);
    locationWid->setFixedSize(554, 50);
    locationWid->setStyleSheet("background-color: #F2F2F2;");

    pddFileLayout->addWidget(ppdlb);
    pddFileLayout->addWidget(ppd);
    pddFileWid->setLayout(pddFileLayout);
    pddFileWid->setFixedSize(554, 50);
    pddFileWid->setStyleSheet("background-color: #F2F2F2;");

    printName->setReadOnly(false);
    driverlocalation->setReadOnly(false);
    ppd->setReadOnly(true);
    //驱动主要信息布局
    messageLayout->addWidget(printerNameWid);
    messageLayout->addWidget(locationWid);
    messageLayout->addWidget(pddFileWid);
    messageWid->setLayout(messageLayout);

    //底部两个按钮布局
    buttomLayout->addWidget(cancelBtn);
    buttomLayout->addWidget(addBtn);
    buttomWid->setLayout(buttomLayout);

    //内容布局
    contentLayout->addWidget(addDriverWid, 0, Qt::AlignCenter);
    contentLayout->addWidget(messageWid, 0, Qt::AlignCenter);
    contentLayout->addWidget(buttomWid, 0, Qt::AlignRight);
    contentWid->setLayout(contentLayout);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(contentWid);
    mainWid->setLayout(mainLayout);
    mainWid->setStyleSheet("background-color:#FFFFFF;");

    mainWid->setObjectName("mainWid");
    mainWid->setStyleSheet("#mainWid{border:1px solid rgba(0,0,0,0.15);background-color:#FFFFFF;}"); //主窗体圆角

    this->setWindowFlags(Qt::FramelessWindowHint);    //设置窗体无边框**加窗管协议后要将此注释调**
    this->setAttribute(Qt::WA_TranslucentBackground); //窗体透明
    this->setStyleSheet("border-radius:6px;");        //主窗体圆角(注意：窗体透明与主窗体圆角要搭配使用否则无效)
    this->setCentralWidget(mainWid);
    qDebug() << "mainWid";
}

void ManualInstallWindow::onShowManualWindow(QString vendor, QString product, QString uri)
{
    m_vendor = vendor;
    m_product = product;
    m_uri = uri;
    qDebug() << "Vendor:" << m_vendor;
    qDebug() << "Product" << m_product;
    qDebug() << "Uri:" << m_uri;
    show();
}
void ManualInstallWindow::onPackageInstalled(ukuiInstallStatus status)
{
    qDebug()<<"onPackageInstalled"<<(int)status;
    emit updatePpdList();
}