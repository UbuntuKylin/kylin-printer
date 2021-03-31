#include "manualinstallwindow.h"
#include "succedfailwindow.h"
#include "xatom-helper.h"

ManualInstallWindow::ManualInstallWindow(QWidget *parent) : QMainWindow(parent), m_apt(nullptr)
{


    installingTimer = new QTimer(this);

    connect(installingTimer,&QTimer::timeout,this,&ManualInstallWindow::displayInstalling);

    SuccedFailWindow *window = new SuccedFailWindow;
    connect(this,&ManualInstallWindow::manualAddSignal,window,&SuccedFailWindow::onShowSucceedFailWindow);

    initManualControls();
    initManualWindow();
    setManualControls();
    setManualWindow();

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(mainWid->winId(), hints);

    mainWid ->setFixedSize(WIDTH,HEIGHT);

    addDriverWid ->installEventFilter(this);
    addDriverWid ->setAcceptDrops(true);

    QScreen *screen = QGuiApplication::primaryScreen(); //需要引用2个头文件<QApplication>和<QScreen>
    mainWid -> move((screen->geometry().width() - WIDTH) / 2, (screen->geometry().height() - HEIGHT) / 2);
    mainWid -> setWindowIcon(QIcon::fromTheme("kylin-printer",QIcon(":/svg/printer_logo.svg")));
    mainWid -> setWindowTitle(tr("手动安装打印机驱动"));

    connect(closeBtn,  &QPushButton::clicked, mainWid, &ManualInstallWindow::hide);
    connect(cancelBtn, &QPushButton::clicked, mainWid, &ManualInstallWindow::hide);

}


void ManualInstallWindow::keyPressEvent(QKeyEvent  *event)
{

}

bool ManualInstallWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched ==addDriverWid)
    {

        if(event->type() == QDragEnterEvent::DragEnter)
        {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent *>(event);
            if(dragEvent->mimeData()->hasFormat("text/uri-list"))
            {
                dragEvent->acceptProposedAction();
                qDebug() << "拖拽的文件名是:"<<dragEvent->mimeData()->text();
            }
        }
        if(event->type() == QDropEvent::Drop)
        {
            QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
            auto urls = dropEvent->mimeData()->urls();
            if (urls.isEmpty())
            {
                //此路径为空，此情况应该不存在。(你无法拖拽一个其路径为空的东西)
                return false;
            }
            else
            {
                QStringList localpath;
                for (auto &url : urls)
                {
                    localpath << url.toLocalFile();
                }
                if(localpath.count() == 1)
                {
                    qDebug()<<"您将drop"<<localpath.at(0);
                    dropDebInstall(localpath.at(0));//执行安装方法
                }
                else
                {
                    QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("您不能同时安装多个包!"),QMessageBox::Yes);
                    msg->button(QMessageBox::Yes)->setText(tr("确认"));
                    msg->exec();
                    return false;
                }


            }

        }

    }
    return QObject::eventFilter(watched , event);
}

void ManualInstallWindow::dropDebInstall(QString debPath)
{
    QFileInfo fileinfo(debPath);
    QString fileSuffix = fileinfo.suffix();
    qDebug()<<"后缀名:"<<fileSuffix;
    if(fileSuffix != "deb")
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("请选择deb包!!!"),QMessageBox::Yes);

        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        return ;
    }
    if (!debPath.isEmpty())
    {
        if (m_apt == nullptr)
        {
            /* code */

            m_apt = new ukuiApt(debPath);
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

                    QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("您不能使用非法的deb包!"),QMessageBox::Yes);
                    msg->button(QMessageBox::Yes)->setText(tr("确认"));
                    msg->exec();
                    disconnect(m_apt, &ukuiApt::reportInstallStatus, this, &ManualInstallWindow::onPackageInstalled);
                    disconnect(m_apt, &ukuiApt::alreadyInstallSignal,this, &ManualInstallWindow::alreadyInstallSlot);
                    delete m_apt;
                    m_apt = nullptr;
                    return ;
                }

            }            

        }
    }
    else
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("路径不能为空!"),QMessageBox::Yes);
        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
    }

}

void ManualInstallWindow::dropEvent(QDropEvent *event) //放下事件
{

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
    connect(printerName,&QLineEdit::textChanged,this,&ManualInstallWindow::inputDisplay);
    //位置行
    locationlb = new QLabel(this);          //位置
    driverlocalation = new QLineEdit(this); //打印机位置
    //ppd行
    ppdlb = new QLabel(this);  //ppd文件
    ppd = new QLineEdit(this); //ppd文件名
    dropDownList = new QComboBox(this);//下拉列表

    //取消和添加按钮
    cancelBtn = new QPushButton(this);
    addBtn = new QPushButton(this);

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
    closeBtn->setFixedSize(30, 30);
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setFlat(true);

    //添加处控件
    addLocalDriverBtn->setFixedSize(120, 36);
    addLocalDriverBtn->setText("添加本地驱动");
    dropTipsLabel->setFixedSize(120, 36);
    dropTipsLabel->setText("拖拽一个驱动到此");
    downloadUrlLabel->setFixedSize(300, 36);

    downloadUrlLabel->setText(tr("或者您可以去相关网址下载:")+"<a href = http://www.kylinos.cn>www.kylinos.cn</a>");
    downloadUrlLabel->setOpenExternalLinks(true);

//    addLocalDriverBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
//                                     "QPushButton:hover{background-color:#3790FA;color:white;}"
//                                     "QPushButton:pressed{background-color:#4169E1;color:white;}");
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
    printerName->setText("default-printer-lasevcP1106");
    printerName->setAcceptDrops(false);

    locationlb->setText(tr("位置"));
    driverlocalation->setFixedSize(442, 36);
    driverlocalation->setText(tr("办公室"));
    driverlocalation->setAcceptDrops(false);
    ppdlb->setText(tr("驱动"));
    ppd->setFixedSize(442, 36);
//    ppd->setText(tr("手动选择驱动方案"));
    dropDownList->setFixedSize(442, 36);
    dropDownList->setItemText(0,tr("手动选择驱动方案"));


    //取消和添加按钮
    cancelBtn->setFixedSize(120, 36);
    cancelBtn->setText("取消");
    addBtn->setFixedSize(120, 36);
    addBtn->setText("添加");


//    addBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
//                          "QPushButton:hover{background-color:#3790FA;color:white;}"
//                          "QPushButton:pressed{background-color:#4169E1;color:white;}");
//    cancelBtn->setStyleSheet("QPushButton{background-color:#E7E7E7;color:black;}"
//                          "QPushButton:hover{background-color:#3790FA;color:white;}"
//                          "QPushButton:pressed{background-color:#4169E1;color:white;}");
}

//输入显示
void ManualInstallWindow::inputDisplay()
{
    QString str = printerName->text();
    qDebug()<<"输入的是:"<<str;
    if(str != "")
    {
        QString temp = str.at(str.length()-1);
        qDebug()<<"长度:"<<str.length()<<"末尾字符为:"<<temp;
        if(temp=="#"||temp == "?"||temp == "'"||temp == "\""||temp == "\/"||temp == "\\"||temp == " ")
        {
            QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("不能输入非法字符!"),QMessageBox::Yes);
            msg->button(QMessageBox::Yes)->setText(tr("确认"));
            msg->exec();
            printerName->clear();
        }
    }
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
    if(!fileName.isEmpty())
    {
        if (m_apt == nullptr)
        {
            /* code */
            m_apt = new ukuiApt(fileName);
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

                    QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("您不能使用非法的deb包!"),QMessageBox::Yes);
                    msg->button(QMessageBox::Yes)->setText(tr("确认"));
                    msg->exec();
                    disconnect(m_apt, &ukuiApt::reportInstallStatus, this, &ManualInstallWindow::onPackageInstalled);
                    disconnect(m_apt, &ukuiApt::alreadyInstallSignal,this, &ManualInstallWindow::alreadyInstallSlot);
                    delete m_apt;
                    m_apt = nullptr;
                    return ;
                }

            }

        }
    }
    else
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("路径不能为空!"),QMessageBox::Yes);
        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
    }

}

void ManualInstallWindow:: alreadyInstallSlot()
{
    QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("已经安装过包!"),QMessageBox::Yes);
    installingTimer->stop();
    installPic->hide();
    msg->button(QMessageBox::Yes)->setText(tr("确认"));
    msg->exec();
    if(m_apt!=nullptr){
        disconnect(m_apt, &ukuiApt::reportInstallStatus, this, &ManualInstallWindow::onPackageInstalled);
        disconnect(m_apt, &ukuiApt::alreadyInstallSignal,this, &ManualInstallWindow::alreadyInstallSlot);
        delete m_apt;
        m_apt = nullptr;
    }
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
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);
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

//    addDriverWid->setStyleSheet(".QWidget{background-color: #F2F2F2;}");

    //三行lineedit布局
    printerNameLayout->addWidget(Namelb);
    printerNameLayout->addWidget(printerName);
    printerNameWid->setLayout(printerNameLayout);
    printerNameWid->setFixedSize(554, 50);
//    printerNameWid->setStyleSheet("background-color: #F2F2F2;");

    locationLayout->addWidget(locationlb);
    locationLayout->addWidget(driverlocalation);
    locationWid->setLayout(locationLayout);
    locationWid->setFixedSize(554, 50);
//    locationWid->setStyleSheet("background-color: #F2F2F2;");

    pddFileLayout->addWidget(ppdlb);
//    pddFileLayout->addWidget(ppd);
    pddFileLayout->addWidget(dropDownList);
    pddFileWid->setLayout(pddFileLayout);
    pddFileWid->setFixedSize(554, 50);
//    pddFileWid->setStyleSheet("background-color: #F2F2F2;");

    printerName->setReadOnly(false);
    printerName->setMaxLength(127);
    driverlocalation->setReadOnly(false);
    driverlocalation->setMaxLength(50);
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
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(0);
    mainWid->setLayout(mainLayout);


//    mainWid->setObjectName("mainWid");
//    mainWid->setStyleSheet("#mainWid{background-color:#FFFFFF;}"); //主窗体圆角

//    this->setWindowFlags(Qt::FramelessWindowHint);    //设置窗体无边框**加窗管协议后要将此注释调**
//    this->setAttribute(Qt::WA_TranslucentBackground); //窗体透明
//    this->setStyleSheet("border-radius:6px;");        //主窗体圆角(注意：窗体透明与主窗体圆角要搭配使用否则无效)
//    this->setCentralWidget(mainWid);
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
    mainWid->show();
//    emit manualAddSignal(m_vendor + " " + m_product,false);
}
void ManualInstallWindow::onPackageInstalled(ukuiInstallStatus status)
{
    qDebug()<<"onPackageInstalled"<<(int)status;
    switch (status){
        case ukuiInstallStatus::UKUI_INSTALL_START:
        {
            qDebug()<<"onPackageInstalled "<<"UKUI_INSTALL_START";
        }
        break;
        case ukuiInstallStatus::UKUI_INSTALL_IN_PROGRESS:
        {
            qDebug()<<"onPackageInstalled "<<"UKUI_INSTALL_IN_PROGRESS";
        }
        break;
        case ukuiInstallStatus::UKUI_INSTALL_SUCCESS:
        {
            qDebug()<<"onPackageInstalled "<<"UKUI_INSTALL_SUCCESS";
            QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("安装成功，重新匹配驱动!"),QMessageBox::Yes);
            msg->button(QMessageBox::Yes)->setText(tr("确认"));
            msg->exec();
            emit updatePpdList();
            debSuccess = true;
        }
        break;
        case ukuiInstallStatus::UKUI_INSTALL_FAIL:
        {

            qDebug()<<"onPackageInstalled "<<"UKUI_INSTALL_FAIL";
            debSuccess = false;
            installingTimer->stop();
            installPic->hide();

            QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("安装失败!"),QMessageBox::Yes);
            msg->button(QMessageBox::Yes)->setText(tr("确认"));
            msg->exec();
            disconnect(m_apt, &ukuiApt::reportInstallStatus, this, &ManualInstallWindow::onPackageInstalled);
            disconnect(m_apt, &ukuiApt::alreadyInstallSignal,this, &ManualInstallWindow::alreadyInstallSlot);
            delete m_apt;
            m_apt = nullptr;
            return ;
        }
        break;
        default:
        break;
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
    else
    {
        qDebug()<<"deb安装失败!";//刚开始没有deb包安装时也会输出此行，不必管他;安装上驱动包之后就不会执行此分支了!!
    }


    qDebug()<<"打印机名:"<<printerName;
    dropDownList->clear();
    this->printerName->setText(printerName);
    driverlocalation->setText(position);
    dropDownList->addItems(ppdList);
}


void ManualInstallWindow::manualAddPrinter()
{
    qDebug()<<"正在安装打印机驱动...";
//    emit manualAddSignal(m_vendor + " " + m_product,bool isSuccess);
    if(printerName->text() =="")
    {
        QMessageBox *msg = new QMessageBox(QMessageBox::Warning,tr("警告"),tr("打印机名称不可为空!"),QMessageBox::Yes);
//        msg->move((mainWid->geometry().width() - WIDTH) / 2, (mainWid->geometry().height() - HEIGHT) / 2);
        msg->button(QMessageBox::Yes)->setText(tr("确认"));
        msg->exec();
        return ;
    }
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
    mainWid->hide();
}
