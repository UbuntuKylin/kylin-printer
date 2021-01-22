#include "popwindow.h"
#include "xatom-helper.h"

#define g_cupsConnection (CupsConnection4PPDs::getInstance()) //CUPS连接建立实例化

PopWindow *PopWindow ::popMutual = nullptr;
PopWindow::PopWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // qDebug() << "pop pid" << QThread::currentThreadId();
    int WIDTH = 413;
    int HEIGHT = 188;
    popMutual = this;

    setWindowTitle(tr("打印机"));
//    setWindowIcon(QIcon(":/svg/printer_logo.svg"));
    //    move((screen->geometry().width() - WIDTH) /2,(screen->geometry().height() - HEIGHT) / 2);
    initControls();                                           //初始化控件
    initPopWindow();                                          //初始化弹窗

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(mainWid->winId(), hints);

    mainWid ->setFixedSize(WIDTH,HEIGHT);
    QScreen *screen = QGuiApplication::primaryScreen();
    mainWid -> move(screen->geometry().topRight());
    mainWid -> setWindowIcon(QIcon(":/svg/printer_logo.svg"));
    mainWid -> setWindowTitle(tr("打印机"));

    manual = new ManualInstallWindow;                         //手动安装驱动界面
    succeed_fail = new SuccedFailWindow;
    property = new PropertyWindow;
    DeviceMonitor::getInstance()->setFindPrinterOnly = true; //为方便测试，默认false:可以检测U盘或打印机USB接口;如果是true：则只能检测打印机
//    ukuiPrinter::getInstance().addPrinter("usb://Canon/LBP251?serial=d8492fe8aa5e",
//                                          "LBP251-TEST",
//                                          "lsb/usr/CNRCUPSLBP251ZK.ppd",
//                                          "");
    DeviceMonitor::getInstance()->start();
    QObject::connect(DeviceMonitor::getInstance(), &DeviceMonitor::findUsbConnect, [=](const DeviceInformation &test) {
        printer = test;
        emit monitorDriver(test, true); //两个参数:1.打印机信息2.是否安装成功；测试时直接改变true或false就可以

        isExistDriver = true;
    });
    QObject::connect(DeviceMonitor::getInstance(), &DeviceMonitor::findUsbDisconnect, [=](const DeviceInformation &test) {
        printer = test;

        emit monitorDriver(test, false);

        isExistDriver = false;
    });
    //CUPS链接建立
    //*************************************************************************************
    newHttp = g_cupsConnection->ppdRequestConnection; //建立，并获取http链接
    if (!newHttp)
    {
        qDebug() << "CUPsConnection construct failed!";
        exit(1);
    }
    //*************************************************************************************

    //通过http链接获取PPDs
    //*************************************************************************************
    cmdFindPPDs = new FindPPDsThread(newHttp);
    pFindPPDs = new QThread();
    cmdFindPPDs->moveToThread(pFindPPDs);

    connect(pFindPPDs, &QThread::finished, cmdFindPPDs, &QObject::deleteLater); //挂载
    connect(this, &PopWindow::signalFindPPDsThread, cmdFindPPDs, &FindPPDsThread::initPPDMapConstruct);
    connect(cmdFindPPDs, SIGNAL(gotAllHandledPPDs(myMap)), this, SLOT(gotAllHandledPPDs(myMap))); //获取到所有PPD文件列表的初始map
    connect(manual, SIGNAL(updatePpdList()), this, SLOT(prematchResultSlot()));

    qRegisterMetaType<myMap>("myMap"); //注册自己的类型，必须！！！！！

    pFindPPDs->start();
    findTime = new QTimer;
    findTime->start(1000); //开始查找计时超过10秒则未找到要手动安装

    emit signalFindPPDsThread(); //发送开始获取PPDS信号
    //*************************************************************************************

    //通过打印机名字查找匹配的PPDS
    //*************************************************************************************
    cmdMatchPPDs = new MatchPPDsThread();
    pMatchPPDs = new QThread();
    cmdMatchPPDs->moveToThread(pFindPPDs);

    connect(pMatchPPDs, &QThread::finished, cmdMatchPPDs, &QObject::deleteLater); //挂载
    connect(this, SIGNAL(signalMatchPPDsThread(QString, QString, myMap, int)), cmdMatchPPDs, SLOT(initPPDMatch(QString, QString, myMap, int)));
    connect(cmdMatchPPDs, &MatchPPDsThread::matchFailed, this, [=] { qDebug() << "查询失败！"; }); //直接失败
    connect(cmdMatchPPDs, SIGNAL(matchResultSignal(resultPair)), this, SLOT(matchResultSlot(resultPair)));

    //匹配成功后发送给手动界面消息
    connect(this,&PopWindow::matchSuccessSignal,manual,&ManualInstallWindow::matchSuccessSlot);
    qRegisterMetaType<resultPair>("resultPair"); //注册自己的类型，必须！！！！！

    pFindPPDs->start();
    //*************************************************************************************

    connect(timer, &QTimer::timeout, this, &PopWindow::loadingPicDisplay);
    connect(findTime, &QTimer::timeout, this, [=] { qDebug() << "****" << timeTag++; }); //开始查找计时超过10秒则未找到要手动安装
    connect(this, &PopWindow::monitorDriver, this, &PopWindow::popDisplay);
    connect(manualInstallBtn, &QPushButton::clicked, this, &PopWindow::showManualWindow);
    connect(this, SIGNAL(signalClickManualButton(QString, QString, QString,QStringList ,bool )),
            manual, SLOT(onShowManualWindow(QString, QString, QString,QStringList ,bool )));
    connect(printTestBtn, &QPushButton::clicked, this, &PopWindow::print); //绑定打印测试页信号
    connect(deviceViewBtn,&QPushButton::clicked,this,&PopWindow::deviceNameSlot);
    connect(this,&PopWindow::printerNameSignal,property,&PropertyWindow::displayDevice);//查看设备

//    connect(this,&PopWindow::basicParameter,manual,&ManualInstallWindow:);

    connect(closeButton, &QPushButton::clicked, mainWid, &PopWindow::hide);

//    connect(succeed_fail,&SuccedFailWindow::printTestSignal,this,&PopWindow::print);
    //必须在检测插拔的槽函数后执行,先检测是否插拔再执行线程
}

//初始化气泡内控件
void PopWindow::initControls()
{
    //标题栏初始化
    titlePictureBtn = new QToolButton(this); //logo
    appNameLabel = new QLabel(this);         //app名
    seatlb = new QLabel(this);               //占位
    closeButton = new QPushButton(this);     //关闭按钮

    //中间部分初始化
    picButton = new QPushButton(this);   //48*48的图标
    timer = new QTimer(this);            //延时
    isMonitorEdit = new QLineEdit(this); //检测标签
    loadPic = new QPushButton(this);     //加载图片:主题里8张图
    isInstallLabel = new QLabel(this);   //正在安装...标签
    isSuccesslb = new QLabel(this);      //安装成功或失败提示

    //底部初始化
    printTestBtn = new QPushButton(this);     //打印测试按钮
    deviceViewBtn = new QPushButton(this);    //设备查看按钮
    manualInstallBtn = new QPushButton(this); //手动安装按钮
}

//初始化气泡弹窗布局
void PopWindow::initPopWindow()
{
    buttonStack = new QStackedWidget();       //底部的3个按钮堆叠布局
    installStateStack = new QStackedWidget(); //安装状态布局

    mainWid = new QWidget();        //弹窗主Wid
    mainLayout = new QVBoxLayout(); //弹窗主布局
    titleWid = new QWidget();       //标题栏Wid
    middleWid = new QWidget();      //中间Wid
    bottomWid = new QWidget();      //底部Wid
    manualButtonWid = new QWidget();
    messageWid = new QWidget();        //消息提示Wid:两行
    monitorMessageWid = new QWidget(); //检测信息Wid
    installMessageWid = new QWidget(); //安装信息Wid

    titleLayout = new QHBoxLayout();  //标题栏布局
    middleLayout = new QHBoxLayout(); //中间布局
    bottomLayout = new QHBoxLayout(); //底部布局
    manualButtonLayout = new QHBoxLayout();
    messageLayout = new QVBoxLayout();        //消息提示布局
    monitorMessageLayout = new QHBoxLayout(); //检测布局
    installMessageLayout = new QHBoxLayout(); //安装布局
}

//设置气泡内控件布局
void PopWindow::setControls(DeviceInformation printerDevice, bool isSuccess)
{
    //标题栏各个按钮
    titlePictureBtn->setIcon(QIcon(":/svg/printer_logo.svg"));
    titlePictureBtn->setFixedSize(24, 24);
    titlePictureBtn->setIconSize(QSize(24, 24));
    titlePictureBtn->setStyleSheet("border-radius:4px;");
    appNameLabel->setFixedSize(45, 20);
    appNameLabel->setText(tr("打印机"));
    seatlb->setText("  "); //占位
    closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeButton->setFixedSize(30,30);
    closeButton->setProperty("isWindowButton", 0x2);
    closeButton->setProperty("useIconHighlightEffect", 0x8);
    closeButton->setFlat(true);

    //中间部分
    picButton->setIcon(QIcon(":/svg/printer_logo.svg"));
    picButton->setFixedSize(48, 48);
    picButton->setIconSize(QSize(48, 48));
    picButton->setStyleSheet("border-radius:4px;");
    isMonitorEdit->setFixedSize(300, 30);
    isMonitorEdit->setFocusPolicy(Qt::NoFocus);
    isMonitorEdit->setText("检测到打印机:" + printerDevice.vendor + " " + printerDevice.model);
    isMonitorEdit->setStyleSheet("QLineEdit{border:0px;background-color:transparent;}");

    //加载图标、安装中...
    loadPic->setIcon(QIcon::fromTheme("ukui-loading-0")); //初始默认图片0
    loadPic->setFixedSize(24, 24);
    loadPic->setIconSize(QSize(24, 24));
    loadPic->setStyleSheet("border-radius:4px;");
    isInstallLabel->setFixedSize(80, 20);
    isInstallLabel->setText("正在安装...");

    //是否安装成功
    isSuccesslb->setFixedSize(120, 20);
    isSucceed = isSuccess;
    searchResult = 0 ;
    //底部部分
    printTestBtn->setText("打印测试");
    printTestBtn->setFixedSize(120, 36);
    deviceViewBtn->setText("查看设备");
    deviceViewBtn->setFixedSize(120, 36);
    manualInstallBtn->setText("手动安装驱动");
    manualInstallBtn->setFixedSize(120, 36);
}

//设置气泡弹窗布局
void PopWindow::setPopWindow()
{
    printTestBtn->hide();
    deviceViewBtn->hide();
    manualInstallBtn->hide();
    //检测设备布局
    monitorMessageLayout->addWidget(isMonitorEdit);
    monitorMessageLayout->setContentsMargins(0, 0, 25, 0);
    monitorMessageWid->setLayout(monitorMessageLayout);
    monitorMessageWid->setFixedHeight(30);

    //安装布局
    installMessageLayout->addWidget(loadPic);
    installMessageLayout->addWidget(isInstallLabel);
    installMessageLayout->setContentsMargins(0, 0, 100, 0);
    installMessageWid->setLayout(installMessageLayout);
    installMessageWid->setFixedHeight(30);

    installStateStack->addWidget(installMessageWid);
    installStateStack->addWidget(isSuccesslb);

    //消息提示布局
    messageLayout->addWidget(monitorMessageWid, 0, Qt::AlignLeft); //检测Wid
    messageLayout->addWidget(installStateStack, 0, Qt::AlignLeft); //安装消息的小Wid
    messageWid->setLayout(messageLayout);

    //标题栏布局
    titleLayout->addWidget(titlePictureBtn);
    titleLayout->addWidget(appNameLabel);
    titleLayout->addWidget(seatlb);
    titleLayout->addWidget(closeButton);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);
    titleWid->setLayout(titleLayout);
    titleWid->setFixedHeight(35);

    //中间布局
    middleLayout->addWidget(picButton);
    middleLayout->addWidget(messageWid);
    middleLayout->setSpacing(0);
    middleWid->setLayout(middleLayout);

    //底部布局
    bottomLayout->addWidget(printTestBtn);
    bottomLayout->addWidget(deviceViewBtn);
    bottomWid->setLayout(bottomLayout);

    //手动安装按钮布局
    manualButtonLayout->addWidget(manualInstallBtn, 0, Qt::AlignRight);
    manualButtonWid->setLayout(manualButtonLayout);

    buttonStack->addWidget(bottomWid);
    buttonStack->addWidget(manualButtonWid);

    mainLayout->addWidget(titleWid);
    mainLayout->addWidget(middleWid);
    mainLayout->addWidget(buttonStack, 0, Qt::AlignRight);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(0);
    mainWid->setLayout(mainLayout);

//    mainWid->setObjectName("mainWid");
//    mainWid->setStyleSheet(".QWidget{background-color:#FFFFFF;}"); //主窗体圆角
//    this->setWindowFlags((Qt::FramelessWindowHint));                                               //设置窗体无边框**加窗管协议后要将此注释调**
//    this->setAttribute(Qt::WA_TranslucentBackground);                                              //主窗体透明
//    this->setCentralWidget(mainWid);
}

//查找PPD列表，找到
void PopWindow::gotAllHandledPPDs(myMap temp)
{
    mymap = temp;
    canFindPPD = true;
    findTime->stop();
    qDebug() << "总共" << timeTag;
}

void PopWindow::matchResultSlot(resultPair res)
{
    qDebug() << res.second;
    if (res.second)
    {
        isExact = res.second;
        qDebug() << "有精准匹配的PPD！";
        qDebug() << "PPD名字是" << res.first.first();

        qDebug() << "精准:" << printer.uri << res.first.first() << printer.vendor;
        m_printer.ppdName = res.first.first().at(0).toStdString();
        //emit result(res.)//发送给鑫哥,鑫哥返回一个是否安装成功
        searchResult = 1;

    }
    else
    {
        isExact = res.second;
        qDebug() << "没有精准匹配的PPD！";
        foreach (auto it, res.first.keys())
        {
            qDebug() << "找到匹配" << it << "个字符的PPD文件" << res.first[it].size() << "个";
        }
        //        qDebug() << "找到相近的结果有" << res.first.size() << "个";

        if (res.first.size() == 0)
        {
            return;
        }
        QMap<int, QStringList>::iterator it = res.first.end();
        it--;
        qDebug() << "其中最接近的是" << (it.value());
        ppdList = it.value();
        qDebug() << "模糊:" << printer.uri << ppdList.at(0) << printer.vendor;
        searchResult = 2;
        m_printer.ppdName = res.first.first().at(0).toStdString();

    }

    //更新manual三行信息
    emit matchSuccessSignal(printer.vendor+"+"+printer.model,"Office",ppdList);//必须带"+"

}

//气泡弹窗显示
void PopWindow::popDisplay(DeviceInformation printerDevice, bool isSuccess)
{
    static int i=0;
    i++;
    name = printerDevice.vendor + QString("+") + printerDevice.model;
    m_printer.name = name.toStdString();
    m_printer.vendor = printerDevice.vendor.toStdString();
    m_printer.uri = printerDevice.uri.toStdString();
    m_printer.prodect = printerDevice.model.toStdString();


    //    根据打印机型号等信息调用接口查找并安装。之后安装后会返回成功与否isSuccess
    setControls(printerDevice, isSuccess); //这里先假设为true待调用接口后具体值由接口返回
    setPopWindow();
    if (isExistDriver == true) //检测到了驱动就弹出气泡
    {
        qDebug() << "气泡弹出";
        qDebug() << i;
//        qDebug()<<"不带好的撒活动那活动"<<m_printer.name.c_str()<<m_printer.uri.c_str()<<m_printer.ppdName.c_str();
        timer->start(150); //动态加载loading图片
        QString temp = name;
        qDebug() << "timeTag 判断";
        if (timeTag > 10)
        {
            qDebug() << "获取PPD列表超时";
        }
        else
        {

            if (canFindPPD && (!mymap.isEmpty()))
            {
                signalMatchPPDsThread(printerDevice.vendor,printerDevice.model, mymap, USB);
            }
            else
            {
                QTimer::singleShot(1000 * (10 - timeTag), [=]() {
                    if (canFindPPD && (!mymap.isEmpty()))
                    {
                        signalMatchPPDsThread(printerDevice.vendor,printerDevice.model, mymap, USB);
                    }
                    else
                    {
                        qDebug() << "获取PPD列表超时" << timeTag;
                    }
                });
            }
        }

        mainWid->show();
    }
    else
    {
        qDebug() << "气泡消失";
        qDebug() << i;
        timer->stop();
        mainWid->hide();
        succeed_fail->hide();//成功或失败界面消失
        manual->hide(); //手动安装驱动界面消失

//        property->hide();//打印机属性界面消失
    }
}

void PopWindow::loadingPicDisplay()
{
    loadPic->setIcon(QIcon::fromTheme("ukui-loading-" + QString::number(i % 8)));
    i%=8;
    i++;
    if (searchResult == 0)
    {
        return;
    }
    if (searchResult == 1)
    {
        //TODO:ui和功能分开
        bool ret = ukuiPrinter::getInstance().addPrinter(
                    m_printer.uri,
                    m_printer.name,
                    m_printer.ppdName,
                    "");
        if (ret == true)
        {
            isSuccesslb->setText("安装成功!");
            installStateStack->setCurrentIndex(1);
            buttonStack->setCurrentIndex(0);
            printTestBtn->show();
            deviceViewBtn->show();
            manualInstallBtn->hide();
            timer->stop();
        }
        else
        {
            searchResult = 2;
        }
    }
    else if (searchResult == 2)
    {
        isSuccesslb->setText("安装驱动失败!");
        installStateStack->setCurrentIndex(1);
        buttonStack->setCurrentIndex(1);
        printTestBtn->hide();
        deviceViewBtn->hide();
        manualInstallBtn->show();
        timer->stop();
        emit basicParameter(m_printer.name.c_str(),m_printer.uri.c_str(),ppdList.at(0));
//        qDebug()<<"PopWindow:"<<m_printer.name.c_str()<<"       "<<m_printer.uri.c_str()<<"   "<<ppdList.at(0);
    }
}

void PopWindow::print()
{
    qDebug() << "StringList的ppd文件";
    //参数为StringList的ppd文件
    //    emit printSignal(QStringList);
    const QString testFileName = "/usr/share/cups/data/default-testpage.pdf";
    bool res = false;
    res = ukuiPrinter::getInstance().printTestPage(m_printer.name,testFileName.toStdString());
    qInfo() << "======================打印测试页结果为================";
    qInfo() << m_printer.name.c_str();
    qInfo() << res;
}

void PopWindow::showManualWindow()
{

    mainWid->hide();//点击了手动安装时就要隐藏气泡弹窗
    emit this->signalClickManualButton(m_printer.vendor.c_str(), m_printer.prodect.c_str(), m_printer.uri.c_str(), ppdList, isExact);
}

void PopWindow::prematchResultSlot()
{
    qDebug() << "prematchResultSlot";
    emit signalFindPPDsThread();

    QString temp = m_printer.vendor.c_str();
    temp.append(" ");
    temp.append(m_printer.prodect.c_str());

    QTimer::singleShot(10000, [=]() {
        signalMatchPPDsThread(m_printer.vendor.c_str(),m_printer.prodect.c_str(), mymap, USB);
    });

}

void PopWindow::deviceNameSlot()
{

    qDebug()<<"打印机名称:"<<manual->printerName->text().replace("+"," ");

    emit printerNameSignal(manual->printerName->text().replace("+"," "),m_printer.ppdName.c_str());//无论模糊精准都要传此基本三个参数
}
