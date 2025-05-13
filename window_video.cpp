#include "window_video.h"
#include "ui_window_video.h"

window_video::window_video(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::window_video)
{
    ui->setupUi(this);
    setWindowTitle("视频终端");

    isPlay = false;
    isClose = false;
    isSave = false;
    scene = new QGraphicsScene(this);   //创建场景
    ui->graphicsView->setScene(scene);
    scene = new QGraphicsScene(this);   //创建场景
    pixmapItem = new QGraphicsPixmapItem(); //创建图元

    search_videodev();//遍历系统视频设备
    videoui_init(); //初始化ui界面参数

}

window_video::~window_video()
{
    stopcapture();
    delete ui;
}

//初始化ui界面参数
void window_video::videoui_init()
{
    //帧率
    ui->comboBox_fps->addItems(QStringList() << "30" << "25" << "15" << "5");
    /*
     * 查看分辨率
     * v4l2-ctl -d /dev/video0 --list-formats-ext
     */
    //分辨率
    ui->comboBox_resolution->addItems(QStringList() << "1280x720" << "640x480" << "352x288" << "320x240" << "160x120");
}

void window_video::on_pushButton_troggle2_clicked()
{
    emit this->back_signal();//发送切换页面信号
    this->hide();
}

void window_video::search_videodev()
{
    QDir v4lDir("/sys/class/video4linux");
    QFileInfoList list = v4lDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &dirInfo, list)
    {
        QString devName = dirInfo.fileName();  // e.g., video0
        QString devPath = "/dev/" + devName;

        // 读取设备名称 (可选)
        QFile nameFile(dirInfo.absoluteFilePath() + "/name");
        QString cameraName = devPath;
        if (nameFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            cameraName += " - " + QString(nameFile.readLine()).trimmed();
            nameFile.close();
        }

        ui->comboBox_videodev->addItem(cameraName, devPath);
        qDebug() << "检测到设备:" << cameraName;
    }

    if (ui->comboBox_videodev->count() == 0)
    {
        qDebug() << "没有检测到任何摄像头设备!";
    }
}



void window_video::on_pushButton_play_clicked()
{
    if(isClose)
    {
        isClose = false;
        ui->pushButton_play->setText("播放");
        stopcapture();
        scene->clear();
        return;
    }
    if(!isPlay)
    {
        isPlay = true;
        ui->pushButton_play->setText("暂停");
        startcapture();
    }
    else
    {
        isPlay = false;
        ui->pushButton_play->setText("播放");
        stopcapture();
    }

}

void window_video::stopcapture()
{
    if(m_capThread)
    {
        m_capThread->stopCapture();
    }
    if(m_thread && m_thread->isRunning())
    {
        m_thread->quit();
        m_thread->wait();
    }
    //pixmapItem的内存释放需要放在scene前面,否则会导致崩溃,scene->clear会清除pixmapItem项,但是不会清除内存
    // 清理图元
    if (scene && isClose)
    {
        isClose = false;
        // 删除之前的图元，防止内存泄漏
        if (pixmapItem)
        {
            scene->removeItem(pixmapItem);
            delete pixmapItem;
            //qDebug() << "pixmapItem" <<Qt::endl;
            pixmapItem = nullptr; // 设置为nullptr避免悬挂指针
        }
        scene->clear();
        //qDebug() << "清理图元" <<Qt::endl;

    }
    //qDebug() << "恢复控件状态" <<Qt::endl;
    setcontrolsta(true); // 恢复控件状态
}

//提取设备号
int window_video::extractVideoNumber(const QString& devicePath)
{
    if (devicePath.startsWith("/dev/video"))
    {
        QString numPart = devicePath.mid(QString("/dev/video").length());
        bool ok;
        int number = numPart.toInt(&ok);
        if (ok)
            return number;
    }
    return -1; // 没找到时返回 -1
}

void window_video::startcapture()
{
    // 创建线程对象
    m_thread = new QThread(this);
    m_capThread = new capThread;
    m_capThread->moveToThread(m_thread);

    // 连接信号槽
    connect(this, &window_video::startthread, m_capThread, &capThread::capWorking);
    connect(m_capThread, &capThread::finish, this, &window_video::displayFrame);
    connect(m_thread, &QThread::finished, m_capThread, &QObject::deleteLater);


    isSave = true;
    //提取设备号
    QString devpath = ui->comboBox_videodev->currentData().toString();  // 使用currentData获取完整设备路径
    if(!cap.open(devpath.toStdString()))  // 使用完整路径打开设备
    {
        qDebug() << "Failed to open camera:" << devpath;
        ui->textBrowser_msg->append("Failed to open camera!");
        return;
    }
    
    // 设置分辨率
    setresolution();
    
    scene->clear();  // 清理之前的场景
    pixmapItem = new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);

    fps = ui->comboBox_fps->currentText().toInt();
    ui->textBrowser_msg->append("当前帧率:" + QString::number(fps));

    setcontrolsta(false);
    
    // 启动线程
    if (!m_thread->isRunning()) {
        m_thread->start();
    }
    emit startthread(&cap, fps);
}

//设置控件状态
void window_video::setcontrolsta(bool status)
{
    //qDebug() << "设置控件状态: " << status <<Qt::endl;
    ui->comboBox_videodev->setEnabled(status);
    ui->comboBox_fps->setEnabled(status);
    ui->comboBox_resolution->setEnabled(status);
}

//设置分辨率
void window_video::setresolution()
{
    int height = 480;  // 设置默认值
    int width = 640;   // 设置默认值
    int reso = ui->comboBox_resolution->currentIndex();
    switch (reso)
    {
    case 0://1280x720
        width = 1280;height = 720;
        break;
    case 1://640x480
        width = 640;height = 480;
        break;
    case 2://352x288
        width = 352;height = 288;
        break;
    case 3://320x240
        width = 320;height = 240;
        break;
    case 4://160x120
        width = 160;height = 120;
        break;
    }
    // 先检查是否成功打开摄像头
    if (!cap.isOpened())
    {
        qDebug() << "摄像头未成功打开!";
        return;
    }

    // 设置分辨率
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    ui->textBrowser_msg->append("当前分辨率是:" + QString::number(width) + " x " + QString::number(height));

}


void window_video::on_pushButton_close_clicked()
{
    isClose = true;
    isSave = false;
    stopcapture();
    ui->pushButton_play->setText("播放");
}

void window_video::on_pushButton_save_clicked()
{
    if(isSave)
    {
        if(isPlay)
        {
            //正在播放停止,已经暂停不需要再次停止
            stopcapture();
        }
        QString fileName = "/home/qin/qt_project/Serialport/images_save/image_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".png";

        currentImage.save(fileName);
        ui->textBrowser_msg->append("保存成功,保存路径:/home/qin/qt_project/Serialport/images_save");
        if(isPlay)
        {
            //恢复播放状态,暂停状态不需要播放
            startcapture();
        }
    }
    else
    {
        ui->textBrowser_msg->append("保存失败,无图像或未打开摄像头");
    }
}

void window_video::on_pushButton_open_clicked()
{
    defaultPath = "/home/qin/qt_project/Serialport/images_save/";
    //打开对话框,筛选出PNG文件
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", defaultPath, "Images (*.png)");

    if(!fileName.isEmpty())
    {
        stopcapture();
        //更新默认路径
        defaultPath = QFileInfo(fileName).absolutePath();

        //加载图像
        QPixmap pixmap(fileName);
        if(!pixmap.isNull())
        {
            //显示图像
            scene->clear();  // 清空之前的内容
            pixmapItem = new QGraphicsPixmapItem();
            scene->addItem(pixmapItem);
            pixmapItem->setPixmap(pixmap);

            ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
            ui->textBrowser_msg->append("加载图片成功");
        }
        else
        {
            ui->textBrowser_msg->append("加载图片失败");
        }

    }
    stopcapture();
    ui->pushButton_play->setText("播放");
}

void window_video::on_pushButton_record_clicked()
{
    if(!isRecord)
    {
        //停止录制->正在录制
        ui->pushButton_record->setText("停止");
        startRecord();
    }
    else
    {
        //正在录制->停止录制
        ui->pushButton_record->setText("录制");
        stopRecord();
    }

    // 获取保存路径

}

//开始录制
void window_video::startRecord()
{
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".avi";
    QString savePath = "/home/qin/qt_project/Serialport/videos_save/" + fileName;

    if (savePath.isEmpty())
    {
        return;
    }
    // 设置视频写入对象
    writer.open(savePath.toStdString(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
    if (!writer.isOpened())
    {
        ui->textBrowser_msg->append("无法保存视频");
        ui->pushButton_record->setText("录制");
        isRecord = false;
        return;
    }
    ui->textBrowser_msg->append("开始录制视频");
    isRecord = true;
}

//停止录制
void window_video::stopRecord()
{
    if (!isRecord)
    {
        ui->textBrowser_msg->append("没有录制正在进行中");
        ui->pushButton_record->setText("录制");
        return;
    }

    writer.release();  // 释放视频写入对象
    ui->textBrowser_msg->append("保存成功,保存路径:/home/qin/qt_project/Serialport/videos_save");
    ui->textBrowser_msg->append("停止录制视频");
    isRecord = false;
}

void window_video::displayFrame(QImage img)
{
    if (!isPlay) return;  // 如果不是播放状态，不更新画面
    
    currentImage = img;  // 保存当前帧用于截图
    QPixmap pixmap = QPixmap::fromImage(img);
    if (pixmapItem) 
    {
        pixmapItem->setPixmap(pixmap);
        ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
    }
    
    // 如果正在录制，需要将QImage转换回cv::Mat进行保存
    if (isRecord) 
    {
        // 转换QImage为cv::Mat
        cv::Mat frame(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
        cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);  // 转换回BGR格式
        writer.write(frame);
    }
}

