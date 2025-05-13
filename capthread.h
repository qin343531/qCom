#ifndef CAPTHREAD_H
#define CAPTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class capThread : public QObject
{
    Q_OBJECT
public:
    explicit capThread(QObject *parent = nullptr);
    void stopCapture(); // 添加停止方法

public slots:    
    void capWorking(cv::VideoCapture* cap, int fps);

signals:
    void finish(QImage img); // 修改为传值而不是指针

private:
    bool m_stop;
    cv::VideoCapture* m_cap;
};

#endif // CAPTHREAD_H
