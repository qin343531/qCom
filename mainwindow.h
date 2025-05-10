#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QtSerialPort>
#include <QString>
#include <QtDebug>
#include <QTimer>
#include <QMessageBox>
#include <QFrame>
#include <QStackedWidget>
#include "window_video.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTimer *porttimer;  //创建定时器对象，实时更新串口设备
    QTimer *sendtimer;  //创建定时发送对象
    bool isOpen; //串口状态
    bool isSend; //发送状态
    QSerialPort *serialPorts;//串口对象
    bool setparam_SerialPort();
    void tofalse_comboBox(bool status);

    window_video *videoui;  //保存页面2地址

private slots:
    void on_pushButton_serialop_clicked();
    void updateserialPorts();   //更新串口设备槽函数
    void readSerialData();      //串口接收函数
    void sendcallback();        //定时发送回调函数
    void sendMsg();             //发送信息
    void troggle_page();        //切换页面

    void on_pushButton_send_clicked();

    void on_checkBox_timersend_stateChanged(int arg1);

    void on_pushButton_clear_clicked();

    void on_lineEdit_cmd_returnPressed();


    void on_pushButton_troggle_clicked();

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
