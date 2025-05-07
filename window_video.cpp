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
    scene = new QGraphicsScene(this);   //创建场景
    pixmapItem = new QGraphicsPixmapItem(); //创建图元

    search_videodev();//遍历系统视频设备
    videoui_init(); //初始化ui界面参数
    //创建定时器
    capTimer = new QTimer(this);
    capTimer->stop();
    connect(capTimer, &QTimer::timeout, this, &window_video::captureFrame);
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

//遍历视频设备
void window_video::search_videodev()
{
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(const QCameraInfo &camerainfo : cameras)
    {
        qDebug() << "Camera device:" << camerainfo.deviceName();
        //qDebug() << "Camera Format:" << camerainfo.description();
        ui->comboBox_videodev->addItem(camerainfo.deviceName());
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
        startcapture(ui->comboBox_videodev->currentIndex());
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
    // 停止定时器
    if (capTimer)
    {
        qDebug() << "停止定时器" <<endl;
        capTimer->stop();
        disconnect(capTimer, &QTimer::timeout, this, &window_video::captureFrame);
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
            qDebug() << "pixmapItem" <<endl;
            pixmapItem = nullptr; // 设置为nullptr避免悬挂指针
        }
        scene->clear();
        qDebug() << "清理图元" <<endl;

    }
    qDebug() << "恢复控件状态" <<endl;
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

void window_video::startcapture(int devNumber)
{
    //提取设备号
    QString devpath = ui->comboBox_videodev->currentText();
    devNumber = extractVideoNumber(devpath);
    if(!cap.open(devNumber))
    {
        qDebug() << "Failed to open camera!";
        ui->textBrowser_msg->append("Failed to open camera!");
        return;
    }
    // 设置分辨率（可选）
    setresolution();
    qDebug() << "设置分辨率（可选）";

    ui->graphicsView->setScene(scene);
    qDebug() << "setScene";

    pixmapItem = new QGraphicsPixmapItem(); //反复实例化
    scene->addItem(pixmapItem);
    qDebug() << "addItem";

    //获取帧率
    fps = ui->comboBox_fps->currentText().toInt();

    capTimer->start(1000 / fps);
    qDebug() << "当前帧率:" << fps <<endl;
    connect(capTimer, &QTimer::timeout, this, &window_video::captureFrame);
    ui->textBrowser_msg->append("当前帧率:" + QString::number(fps));

    setcontrolsta(false);
}

//设置控件状态
void window_video::setcontrolsta(bool status)
{
    qDebug() << "设置控件状态: " << status <<endl;
    ui->comboBox_videodev->setEnabled(status);
    ui->comboBox_fps->setEnabled(status);
    ui->comboBox_resolution->setEnabled(status);
}

//设置分辨率
void window_video::setresolution()
{
    int height, width;
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

//更新视频帧
void window_video::captureFrame()
{
    cv::Mat frame;
    cap>>frame;

    if(frame.empty())
    {
        //qDebug() <<"frame is empty"<<endl;
        ui->textBrowser_msg->append("frame is empty");
        return;
    }

    //转为RGB格式
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    //转为QImage
    QImage img((const uchar*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);


    //显示graphicsView上
    QPixmap pixmap = QPixmap::fromImage(img);
    // 更新图元
    pixmapItem->setPixmap(pixmap);

    ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);
}

void window_video::on_pushButton_close_clicked()
{
    isClose = true;
    stopcapture();
    ui->pushButton_play->setText("播放");
}
