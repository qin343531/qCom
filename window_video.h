#ifndef WINDOW_VIDEO_H
#define WINDOW_VIDEO_H

#include <QMainWindow>
#include <QCameraInfo>
#include <QCamera>
#include <QImage>
#include <QTimer>
#include <QPixmap>
<<<<<<< HEAD
#include <QImage>
#include <QDateTime>
#include <QFileDialog>
=======
>>>>>>> 4a5d97a33abaa675897096ad2681977e74f91a4f
#include <opencv2/opencv.hpp>
#include <QGraphicsPixmapItem>


namespace Ui {
class window_video;
}

class window_video : public QMainWindow
{
    Q_OBJECT

public:
    explicit window_video(QWidget *parent = nullptr);
    ~window_video();
    void search_videodev();
<<<<<<< HEAD
    void startcapture();     //捕获图像
=======
    void startcapture(int devNumber);     //捕获图像
>>>>>>> 4a5d97a33abaa675897096ad2681977e74f91a4f
    void stopcapture();         //停止捕获
    void videoui_init();    //初始化ui界面参数
    void setresolution();        //设置分辨率
    void setcontrolsta(bool status);   //设置控件状态
    int extractVideoNumber(const QString& devicePath);

    cv::VideoCapture cap;
    QTimer *capTimer;
    QGraphicsPixmapItem *pixmapItem;
    QGraphicsScene *scene;
<<<<<<< HEAD
    QImage currentImage; //存储当前帧
    QString defaultPath;

    int fps;//存储用户设置帧率
    int devNumber;
    bool isPlay;
    bool isClose;
    bool isSave;
=======

    int fps;//存储用户设置帧率
    bool isPlay;
    bool isClose;
>>>>>>> 4a5d97a33abaa675897096ad2681977e74f91a4f

private slots:
    void captureFrame();

    void on_pushButton_troggle2_clicked();

    void on_pushButton_play_clicked();

    void on_pushButton_close_clicked();

<<<<<<< HEAD
    void on_pushButton_save_clicked();

    void on_pushButton_open_clicked();

=======
>>>>>>> 4a5d97a33abaa675897096ad2681977e74f91a4f
signals:
    void back_signal();//切换页面信号

private:
    Ui::window_video *ui;
};

#endif // WINDOW_VIDEO_H
