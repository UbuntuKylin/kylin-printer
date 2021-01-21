#include "manualinstallwindow.h"
#include "succedfailwindow.h"

ManualInstallWindow::ManualInstallWindow(QWidget *parent) : QMainWindow(parent), m_apt(nullptr)
{
    int WIDTH = 620;
    int HEIGHT = 660;
    this->setFixedSize(WIDTH, HEIGHT);

    setWindowTitle(tr("手动安装打印机驱动"));

    setAcceptDrops(true);

    installingTimer = new QTimer(this);

    connect(installingTimer,&QTimer::timeout,this,&ManualInstallWindow::displayInstalling);

    SuccedFailWindow *window = new SuccedFailWindow;
    connect(this,&ManualInstallWindow::manualAddSignal,window,&SuccedFailWindow::onShowSucceedFailWindow);

    this->setWindowIcon(QIcon(":/svg/printer_logo.svg"));
    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);

    initManualControls();
    initManualWindow();
    setManualControls();
    setManualWindow();

    connect(closeBtn, &QPushButton::clicked, this, &ManualInstallWindow::hide);
    connect(cancelBtn,&QPushButton::clicked,this,&ManualInstallWindow::hide);

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

    qDebug() << "localpath:" << localpath;//如果他拖拽了一堆则默认选择第0个元素
    QFileInfo fileinfo(localpath.at(0));
    QString fileSuffix = fileinfo.suffix();
    if(fileSuffix != "deb")
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("请选择deb包!!!"),QMessageBox::Yes);

        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        return ;
    }

    if (!localpath.isEmpty())
    {

        if (m_apt == nullptr)
        {
            /* code */
            m_apt = new ukuiApt(localpath.at(0));
            if (m_apt != nullptr)
            {

                installPic->show();

                connect(m_apt, &ukuiApt::reportInstallStatus,
                        this, &ManualInstallWindow::onPackageInstalled);
                connect(m_apt,&ukuiApt::alreadyInstallSignal,this,&ManualInstallWindow::alreadyInstallSlot);
                if(m_apt->install())
                {
                    installingTimer->start(100);

                }
                else
                {
                    installingTimer->stop();
                    installPic->hide();
                    m_apt = nullptr;
                    disconnect(m_apt, &ukuiApt::reportInstallStatus,this, &ManualInstallWindow::onPackageInstalled);
                }


            }

        }
    }
}

void ManualInstallWindow::displayInstalling()
{
    installPic->setIcon(QIcon::fromTheme("ukui-loading-" + QString::number(j % 8)));
    j%=8;
    j++;

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
    debPic = new QPushButton(this);        //deb包图片
    debName = new QLabel(this);                //deb包名
    installPic = new QPushButton(this);   //安装中。。。图标
    installPic->hide();

    //名称行
    Namelb = new QLabel(this);       //名称
    printerName = new QLineEdit(this); //打印机名称
    //位置行
    locationlb = new QLabel(this);          //位置
    driverlocalation = new QLineEdit(this); //驱动位置
    //ppd行
    ppdlb = new QLabel(this);  //ppd文件
    ppd = new QLineEdit(this); //ppd文件名
    dropDownList = new QComboBox(this);//下拉列表

    //取消和添加按钮
    cancelBtn = new QPushButton(this);
    addBtn = new QPushButton(this);
    qDebug()<<"setenable shixiaoshuo";
    addBtn->setEnabled(true);
    connect(addBtn,&QPushButton::clicked,this,&ManualInstallWindow::manualAddPrinter);
    connect(addLocalDriverBtn,&QPushButton::clicked,this,&ManualInstallWindow::addLocalDriverSlot);
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
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setFlat(true);

    //添加处控件
    addLocalDriverBtn->setFixedSize(120, 36);
    addLocalDriverBtn->setText("添加本地驱动");
    dropTipsLabel->setFixedSize(120, 36);
    dropTipsLabel->setText("拖拽一个驱动到此");
    downloadUrlLabel->setFixedSize(300, 36);

    downloadUrlLabel->setText(tr("或者您可以去相关网址下载:")+"<a href = http://www.baidu.com>www.baidu.com</a>");
    downloadUrlLabel->setOpenExternalLinks(true);

    addLocalDriverBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
                                     "QPushButton:hover{background-color:#3790FA;color:white;}"
                                     "QPushButton:pressed{background-color:#4169E1;color:white;}");
    installPic->setFixedSize(48,48);
    installPic->setIconSize(QSize(48, 48));
    installPic->setIcon(QIcon::fromTheme("ukui-loading-" + QString::number(0)));
    installPic->setStyleSheet("border-radius:24px;");

    debPic ->setFixedSize(30,30);
    debPic ->setText("Test");
    debPic ->setStyleSheet("");
    debName ->setText("test.ppd");

    //三行lineEdit
    Namelb->setText(tr("名称"));
    printerName->setFixedSize(442, 36);
    printerName->setText("HP-printer-lasevcP1106");
    locationlb->setText(tr("位置"));
    driverlocalation->setFixedSize(442, 36);
    driverlocalation->setText(tr("办公室"));
    ppdlb->setText(tr("驱动"));
    ppd->setFixedSize(442, 36);
    ppd->setText(tr("手动选择驱动方案"));
    dropDownList->setFixedSize(442, 36);


    //取消和添加按钮
    cancelBtn->setFixedSize(120, 36);
    cancelBtn->setText("取消");
    addBtn->setFixedSize(120, 36);
    addBtn->setText("添加");


    addBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
                          "QPushButton:hover{background-color:#3790FA;color:white;}"
                          "QPushButton:pressed{background-color:#4169E1;color:white;}");
    cancelBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
                          "QPushButton:hover{background-color:#3790FA;color:white;}"
                          "QPushButton:pressed{background-color:#4169E1;color:white;}");
}

void ManualInstallWindow::addLocalDriverSlot()//系统弹窗的选择deb包
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("请选择一个deb包"),
                QDir::currentPath(),
                "Deb(*.deb)");
    QFileInfo fileinfo(fileName);
    QString filesuffix = fileinfo.suffix();
    if(!fileinfo.isFile()||!filesuffix.contains("deb"))
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("请选择deb包!!!"),QMessageBox::Yes);

        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        return ;
    }
    if (m_apt == nullptr)
    {
        /* code */
        m_apt = new ukuiApt(fileName);
        if (m_apt != nullptr)
        {

            installPic->show();

            connect(m_apt, &ukuiApt::reportInstallStatus,
                    this, &ManualInstallWindow::onPackageInstalled);
            if(m_apt->install())
            {
                installingTimer->start(100);

            }
            else
            {
                installingTimer->stop();
                installPic->hide();
//                m_apt->deleteLater();
                m_apt = nullptr;
                disconnect(m_apt, &ukuiApt::reportInstallStatus,this, &ManualInstallWindow::onPackageInstalled);
            }


        }

    }
}

void ManualInstallWindow:: alreadyInstallSlot()
{
    QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("已经安装过包!"),QMessageBox::Yes);

    msg->button(QMessageBox::Yes)->setText(tr("确认"));
    msg->exec();

    return ;
}

void ManualInstallWindow::initManualWindow()
{
    //Wid
    titleWid = new QWidget();     //标题栏Wid
    contentWid = new QWidget();   //内容Wid
    loadWid = new QWidget();   //debWid
    addDriverWid = new QWidget(); //添加驱动的Wid
    messageWid = new QWidget();   //主要信息显示Wid
    buttomWid = new QWidget();    //底部按钮Wid
    mainWid = new QWidget();      //主WId

    //布局
    titleLayout = new QHBoxLayout();     //标题栏布局
    contentLayout = new QVBoxLayout();   //内容布局

    loadLayout = new QHBoxLayout();       //deb布局
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

    loadLayout->addWidget(installPic);
    loadWid->setLayout(loadLayout);

    //添加拖拽界面
    addDriverLayout->addStretch();
    addDriverLayout->addWidget(addLocalDriverBtn, 0, Qt::AlignCenter);
    addDriverLayout->addWidget(dropTipsLabel, 0, Qt::AlignCenter);
    addDriverLayout->addWidget(downloadUrlLabel, 0, Qt::AlignCenter);
    addDriverLayout->addWidget(loadWid, 0, Qt::AlignCenter);
    addDriverLayout->addStretch();
    addDriverWid->setLayout(addDriverLayout);
    addDriverWid->setFixedSize(554, 328);
    addDriverWid->setStyleSheet("background-color: #F2F2F2;");

    //三行lineedit布局
    printerNameLayout->addWidget(Namelb);
    printerNameLayout->addWidget(printerName);
    printerNameWid->setLayout(printerNameLayout);
    printerNameWid->setFixedSize(554, 50);
    printerNameWid->setStyleSheet("background-color: #F2F2F2;");

    locationLayout->addWidget(locationlb);
    locationLayout->addWidget(driverlocalation);
    locationWid->setLayout(locationLayout);
    locationWid->setFixedSize(554, 50);
    locationWid->setStyleSheet("background-color: #F2F2F2;");

    pddFileLayout->addWidget(ppdlb);
//    pddFileLayout->addWidget(ppd);
    pddFileLayout->addWidget(dropDownList);
    pddFileWid->setLayout(pddFileLayout);
    pddFileWid->setFixedSize(554, 50);
    pddFileWid->setStyleSheet("background-color: #F2F2F2;");

    printerName->setReadOnly(false);
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
//    buttomWid->setStyleSheet("background-color:pink;");

    //内容布局
    contentLayout->addWidget(addDriverWid, 0, Qt::AlignCenter);
    contentLayout->addWidget(messageWid, 0, Qt::AlignCenter);
    contentLayout->addWidget(buttomWid, 0, Qt::AlignRight);
    contentWid->setLayout(contentLayout);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(contentWid);
    mainLayout->setMargin(0);
    mainWid->setLayout(mainLayout);

    mainWid->setObjectName("mainWid");
    mainWid->setStyleSheet("#mainWid{background-color:#FFFFFF;}"); //主窗体圆角

    this->setWindowFlags(Qt::FramelessWindowHint);    //设置窗体无边框**加窗管协议后要将此注释调**
    this->setAttribute(Qt::WA_TranslucentBackground); //窗体透明
    this->setStyleSheet("border-radius:6px;");        //主窗体圆角(注意：窗体透明与主窗体圆角要搭配使用否则无效)
    this->setCentralWidget(mainWid);
    qDebug() << "mainWid";
}

void ManualInstallWindow::onShowManualWindow(QString vendor, QString product, QString uri,QStringList ppdList, bool isExact)
{
    m_vendor = vendor;
    m_product = product;
    m_uri = uri;
    m_ppdList = ppdList;
    m_isExact = isExact;
    qDebug() << "Vendor:"   << m_vendor;
    qDebug() << "Product"   << m_product;
    qDebug() << "Uri:"      << m_uri;
    qDebug() << "isExact??" << m_isExact ;
    printerName->setText(m_vendor + "+" + m_product);
//    dropDownList->addItems(m_ppdList);
    show();
//    emit manualAddSignal(m_vendor + " " + m_product,false);
}
void ManualInstallWindow::onPackageInstalled(ukuiInstallStatus status)
{
    qDebug()<<"onPackageInstalled"<<(int)status;
    if((int)status)
    {
        //安装deb成功后需要在list刷新
        emit updatePpdList();
//        emit matchPpdList();
        debSuccess = true;
        dropDownList->addItem("2");


    }
    else
    {
        //安装失败
    }

}

void ManualInstallWindow::matchSuccessSlot(QString printerName,QString position,QStringList ppdList)
{

    //找了再次更新三行信息
    matchTag++;
    qDebug()<<"匹配"<<matchTag<<"次!!!!!";

    installingTimer->stop();
    installPic->hide();
    m_apt->deleteLater();
    m_apt = nullptr;
    disconnect(m_apt, &ukuiApt::reportInstallStatus,this, &ManualInstallWindow::onPackageInstalled);
    if(debSuccess&&matchTag>1)
    {
        addBtn->setEnabled(true);

    }

    dropDownList->clear();

    this->printerName->setText(printerName);
    driverlocalation->setText(position);
    dropDownList->addItems(ppdList);
}


void ManualInstallWindow::manualAddPrinter()
{
    qDebug()<<"正在安装打印机驱动...";
//    emit manualAddSignal(m_vendor + " " + m_product,bool isSuccess);

    m_printer.name = printerName->text().toStdString();
    m_printer.uri = m_uri.toStdString();
    m_printer.ppdName = dropDownList->currentText().toStdString();
    qDebug()<<m_printer.name.c_str()<<" "<<m_printer.uri.c_str()<<" "<<m_printer.ppdName.c_str();
    bool isManualInstallSuccess = ukuiPrinter::getInstance().addPrinter(
                m_printer.uri,
                m_printer.name,
                m_printer.ppdName,
                "");

    if(isManualInstallSuccess == true)
    {
        emit manualAddSignal(printerName->text(),isManualInstallSuccess);

    }
    else
    {
        emit manualAddSignal(printerName->text(),isManualInstallSuccess);
    }

}
