#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->videoui = new window_video;
    connect(this->videoui, &window_video::back_signal, this, &MainWindow::troggle_page);

    setWindowTitle("qCom");
    ui->comboBox_serialport->clear();
    isOpen = false;
    isSend = false;
    serialPorts = new QSerialPort(this);//初始化串口对象
    porttimer = new QTimer(this);   //初始化定时器
    connect(porttimer, &QTimer::timeout, this, &MainWindow::updateserialPorts);//连接槽函数，每一秒更新一次设备
    porttimer->start(1000);
    sendtimer = new QTimer(this);   //初始化定时发送对象
    connect(sendtimer, &QTimer::timeout, this, &MainWindow::sendcallback);
    sendtimer->stop();

   //添加波特率
    ui->comboBox_bound->addItems(QStringList() << "115200" << "38400" << "9600" << "1500000");
    //添加校验位
    ui->comboBox_val->addItems(QStringList() << "无校验位" << "偶校验" << "奇校验");
    //添加停止位
    ui->comboBox_stop->addItems(QStringList() << "1停止位" << "1.5停止位" << "2停止位");
    //添加数据位
    ui->comboBox_data->addItems(QStringList() << "8" << "7" << "6" << "5");
    //添加流控
    ui->comboBox_flow->addItems(QStringList() << "无流控" << "硬件流控" << "软件流控");

}

MainWindow::~MainWindow()
{
    if(serialPorts->isOpen())   // 增加关闭串口的检查
    {
        serialPorts->close();
    }
    delete serialPorts;
    delete porttimer;
    delete sendtimer;
<<<<<<< HEAD
=======
    delete videoui;
>>>>>>> 4a5d97a33abaa675897096ad2681977e74f91a4f
    delete ui;
}

//更新串口设备函数
void MainWindow::updateserialPorts()
{
    ui->comboBox_serialport->clear();
    // 获取并添加所有可用串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        if(portName.startsWith("ttyUSB"))
        {
            //qDebug() << "找到串口:" << portName;
            ui->comboBox_serialport->addItem("/dev/" + portName);
        }
    }
}

//判断串口状态
void MainWindow::on_pushButton_serialop_clicked()
{
    if(!isOpen)
    {
        ui->pushButton_serialop->setText("关闭串口");
        isOpen = true;
        tofalse_comboBox(false); //禁止其他控件可选
        MainWindow::setparam_SerialPort();
    }
    else
    {
        tofalse_comboBox(true);
        ui->pushButton_serialop->setText("打开串口");
        serialPorts->close();//关闭串口
        disconnect(serialPorts, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
        isOpen = false;
    }
}

//配置串口参数
void MainWindow::setparam_SerialPort()
{
    //获取用户参数
    QString portname = ui->comboBox_serialport->currentText();  //获取串口设备
    int bound   = ui->comboBox_bound->currentText().toInt();    //获取波特率
    int val     = ui->comboBox_val->currentIndex();             //获取校验位
    int data    = ui->comboBox_data->currentIndex();            //获取数据位
    int stop    = ui->comboBox_stop->currentIndex();            //获取停止位
    int flow    = ui->comboBox_flow->currentIndex();            //获取流控

    qDebug()<<"portName="<<portname<<"bound="<<bound<<endl<<"val="<<val<<endl<<"data="<<data<<endl<<"stop="<<stop<<endl;

    //对获取的参数进行判断
    //校验位需要判断指针索引
    QSerialPort::Parity paritybits; //创建串口校验位对象
    switch (val)
    {
    case 0:     paritybits = QSerialPort::NoParity;     break;  //无校验位
    case 1:     paritybits = QSerialPort::EvenParity;   break;  //偶校验
    case 2:     paritybits = QSerialPort::OddParity;    break;  //奇校验
    default:    paritybits = QSerialPort::NoParity;     break;  //默认无校验
    }

    //对停止位进行判断
    QSerialPort::StopBits stopbits;
    switch(stop)
    {
    case 0:     stopbits = QSerialPort::OneStop;        break;//1停止位
    case 1:     stopbits = QSerialPort::OneAndHalfStop; break;//1.5停止位
    case 2:     stopbits = QSerialPort::TwoStop;        break;//2停止位
    default:    stopbits = QSerialPort::OneStop;        break;
    }

    //对数据位进行判断
    QSerialPort::DataBits databits;
    switch(data)
    {
    case 0:     databits = QSerialPort::Data8;  break;      //8停止位
    case 1:     databits = QSerialPort::Data7;  break;      //7停止位
    case 2:     databits = QSerialPort::Data6;  break;      //6停止位
    case 3:     databits = QSerialPort::Data5;  break;      //5停止位
    default:    databits = QSerialPort::Data8;  break;      //8停止位
    }

    //对硬件流控进行判断
    QSerialPort::FlowControl flowcon;
    switch(flow)
    {
    case 0:     flowcon = QSerialPort::NoFlowControl;   break;  //无流控
    case 1:     flowcon = QSerialPort::HardwareControl; break;  //硬件流控
    case 2:     flowcon = QSerialPort::SoftwareControl; break;  //软件流控
    default:    flowcon = QSerialPort::NoFlowControl;   break;  //无流控
    }

    //配置参数
    serialPorts->setPortName(portname);     //串口设备名字
    serialPorts->setBaudRate(bound);        //波特率
    serialPorts->setParity(paritybits);     //校验位
    serialPorts->setStopBits(stopbits);     //停止位
    serialPorts->setDataBits(databits);     //数据位
    serialPorts->setFlowControl(flowcon);   //无流控
    serialPorts->setRequestToSend(ui->checkBox_RTS->isChecked());
    serialPorts->setDataTerminalReady(ui->checkBox_DTR->isChecked());

    //打开串口
    if(!serialPorts->open(QIODevice::ReadWrite))
    {
        QString errorMeg = serialPorts->errorString();  //获取详细信息
        QMessageBox::warning(this, "串口设备错误", "串口打开失败！\n原因：" + errorMeg);
        isOpen = false;
        ui->pushButton_serialop->setText("打开串口");

    }
    else
    {
        qDebug() << "打开串口成功" <<endl;
        if("串口未打开或不可写!!!" == ui->lineEdit_cmd->text())
            ui->lineEdit_cmd->clear();//清除发送控件的内容
        serialPorts->clear();//清除串口缓冲区
        //成功后连接槽函数
        connect(serialPorts, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
        qDebug() << "连接回车槽函数" <<endl;
    }
}

//显示接收信息
void MainWindow::readSerialData()
{
    if(isSend)
    {
        isSend = false; //恢复接收函数
        serialPorts->clear();
        return;
    }
    //把接受到的数据显示到窗口上
    QByteArray data = serialPorts->readAll();//读取全部数据
    // 去除多余的换行符或回车符
    qDebug() << "原始数据：" << data.toHex(' ').toUpper();

    data = data.trimmed();  // 去除首尾空白字符
    data = data.replace("\r\n", "\n");  // 替换回车换行符为单独的换行符
    if(!ui->checkBox_shell->isChecked())
    {
        if(ui->checkBox_display->isChecked())
        {
            QString hexstr = data.toHex(' ').toUpper();
            ui->textBrowser->append("HEX接收:" + hexstr);   //显示
            qDebug() << "接收到的十六进制数据：" << hexstr;  // 打印接收到的数据

        }
        else
        {
            QString str = QString::fromUtf8(data);//转为字符串
            ui->textBrowser->append("接收:" + str);   //显示
            qDebug() << "接收到的数据：" << str;  // 打印接收到的数据
        }

    }
    else
    {
        //终端模式
        QString str = QString::fromUtf8(data);//转为字符串
        ui->textBrowser->append(str);   //显示
        qDebug() << "接收到的数据：" << str;  // 打印接收到的数据
    }
}

//发送信息槽函数
void MainWindow::on_pushButton_send_clicked()
{
    isSend = true; //发送信息,禁止本地回环
    sendMsg();
}

//发送信息
void MainWindow::sendMsg()
{
    QString textToSend = ui->lineEdit_cmd->text();
    qDebug() << "【sendMsg】lineEdit 内容：" << textToSend;
    if(textToSend.isEmpty()) {
            qDebug() << "输入框为空，无法发送信息";
            return;  // 输入框为空，直接返回
        }
    if(serialPorts->isOpen() && serialPorts->isWritable())
    {
        //发送信息
        if(ui->checkBox_sendHex->isChecked())
        {
            QByteArray hexData = QByteArray::fromHex(textToSend.simplified().toUtf8());
            hexData.append("\n");
            serialPorts->write(hexData);
            serialPorts->flush();//刷新缓冲区
            qDebug() << "发送数据（十六进制）：" << hexData;  // 打印发送的十六进制数据
        }
        else
        {
            serialPorts->write(textToSend.toUtf8() + "\n");
            serialPorts->flush();//刷新缓冲区
            qDebug() << "发送数据：" << ui->lineEdit_cmd->text();
        }
        qDebug() << "发送完毕"<<endl;  // 打印发送的十六进制数据
    }
    else
    {
        ui->lineEdit_cmd->clear();
        qDebug() << "串口未打开或不可写!!!";
        ui->lineEdit_cmd->setText("串口未打开或不可写!!!");
        sendtimer->stop();
    }
}

//串口开启期间,禁止其他控件可选
void MainWindow::tofalse_comboBox(bool status)
{
    ui->comboBox_val->setEnabled(status);
    ui->comboBox_data->setEnabled(status);
    ui->comboBox_flow->setEnabled(status);
    ui->comboBox_stop->setEnabled(status);
    ui->comboBox_bound->setEnabled(status);
    ui->comboBox_serialport->setEnabled(status);
}

//定时发送回调函数
void MainWindow::sendcallback()
{
    sendMsg();
}

//勾选定时发送
void MainWindow::on_checkBox_timersend_stateChanged(int arg1)
{
    if(isOpen)
    {
        switch(arg1)
        {
        case Qt::Unchecked://未选中
            qDebug() << "正常发送" << endl;
            sendtimer->stop();
            break;
        case Qt::Checked://勾选
            qDebug() << "定时发送" << endl;
            int sendtime = ui->lineEdit_send->text().toInt();
            if (sendtime > 0)  // 如果转换成功并且时间大于0
            {
                qDebug() << "定时发送，发送时间：" << sendtime << "ms";
                sendtimer->start(sendtime);  // 启动定时器
            }
            else
            {
                qDebug() << "请输入有效的发送时间（大于0的整数）";
            }
            break;

        }
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();//清空窗口
    ui->lineEdit_cmd->clear();//清空发送内容
}


void MainWindow::on_lineEdit_cmd_returnPressed()
{
    qDebug()<<"回车"<<endl;
    sendMsg();
    ui->lineEdit_cmd->clear();//清空发送内容
}

//切换页面
void MainWindow::on_pushButton_troggle_clicked()
{
    this->hide();//隐藏当前页面
    videoui->show();//打开页面2
}

 void MainWindow::troggle_page()
 {
     this->show();
     videoui->hide();
 }
