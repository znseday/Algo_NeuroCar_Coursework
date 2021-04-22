#ifndef MAINGLWIDGET_H
#define MAINGLWIDGET_H

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

#include "Types.h"
#include "MainModel.h"

using ClockType = std::chrono::steady_clock;

class MainGLWidget : public QOpenGLWidget
{
    Q_OBJECT
private:
    QOpenGLFramebufferObject *MyFBO = nullptr;

    int fps = 0;
    decltype(ClockType::now()) tEndSecond, tStartSecond = ClockType::now();

    QTimer *MainTimer;

    double AspectRatio = 1;

    WorkModeType &WorkMode;
    MainModel &Model;

    double wx = 0, wy = 0, wz = 0;
    bool wExists = false;

    /*mutable*/ GLint vport[4];
    /*mutable*/ GLdouble modl[16], proj[16];

protected:

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *pe) override;
    void keyReleaseEvent(QKeyEvent *pe) override;

    void mousePressEvent(QMouseEvent *pe) override;
    void mouseMoveEvent(QMouseEvent *pe) override;
    void mouseReleaseEvent(QMouseEvent *pe) override;

    void wheelEvent(QWheelEvent *pe) override;

    bool MouseToWorld_v3(int clientX, int clientY,
                                 GLdouble &_worldX, GLdouble &_worldY, GLdouble &_worldZ);


public:
    MainGLWidget() = delete;
    MainGLWidget(WorkModeType &_workMode, MainModel &_model, QWidget *parent = nullptr);

    int GetWidth() const {return this->width();}
    int GetHeight() const {return this->height();}

    void StartMainTimer();
    void StopMainTimer();

//    bool MouseToWorld_v3(int clientX, int clientY,
//                                 GLdouble &_worldX, GLdouble &_worldY, GLdouble &_worldZ);

public slots:

    void SlotReceiveMainTimerTimeout();
    void SlotReceiveRepaint();

signals:
    void SignalSendWorldCoords(double wx, double wy, double wz, bool wExists);
    void SignalSendCurrentFps(int fps);
    void SignalSendTimerStatus(bool isOn);
    void SignalSendCarInfo(double v, double s, int t, double vaver, double accel, double wheelSpeed);
};

#endif // MAINGLWIDGET_H
