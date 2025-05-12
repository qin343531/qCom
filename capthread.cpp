#include "capthread.h"

capThread::capThread(QObject *parent)
    : QObject{parent}, m_stop(false), m_cap(nullptr)
{
}

void capThread::stopCapture()
{
    m_stop = true;
}

void capThread::capWorking(cv::VideoCapture* cap)
{
    if (!cap || !cap->isOpened()) {
        qDebug() << "Camera not opened in thread";
        return;
    }

    m_cap = cap;
    m_stop = false;
    
    while(!m_stop)
    {
        cv::Mat frame;
        if (!m_cap->read(frame))  // 使用read代替>>操作符
        {
            qDebug() << "Failed to read frame";
            continue;
        }
        
        if(frame.empty())
        {
            qDebug() << "Empty frame";
            continue;
        }

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage img(frame.data, frame.cols, frame.rows, 
                  frame.step, QImage::Format_RGB888);
        
        emit finish(img.copy());
        QThread::msleep(1000/30);  // 限制帧率约30fps
    }
}
