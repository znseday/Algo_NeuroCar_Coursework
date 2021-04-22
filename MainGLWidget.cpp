#include "MainGLWidget.h"

#include <chrono>

#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QDebug>

#include "Types.h"

MainGLWidget::MainGLWidget(WorkModeType &_workMode, MainModel &_model, QWidget *parent)
    : QOpenGLWidget(parent), WorkMode(_workMode), Model(_model)
{
    MainTimer = new QTimer(this);
    connect(MainTimer, SIGNAL(timeout()),
                 this, SLOT(SlotReceiveMainTimerTimeout()));

    connect(&Model, SIGNAL(SignalSendRepaint()),
            this, SLOT(SlotReceiveRepaint()));
}
//-------------------------------------------------------------

void MainGLWidget::StartMainTimer()
{
    MainTimer->start(dt_60fps);
    emit SignalSendTimerStatus(MainTimer->isActive());
}
//-------------------------------------------------------------

void MainGLWidget::StopMainTimer()
{
    MainTimer->stop();
    emit SignalSendTimerStatus(MainTimer->isActive());
}
//-------------------------------------------------------------

void MainGLWidget::SlotReceiveMainTimerTimeout()
{
    if (WorkMode == WorkModeType::RunningManual || WorkMode == WorkModeType::RunningNeuroCar
            || WorkMode == WorkModeType::Competition)
    {
        Model.NextStep(dt_60fps);

        const auto & car = Model.GetFirstCar();

        emit SignalSendCarInfo(car.GetSpeed(), car.GetSsum(), car.Get_tInternal(), car.GetVaver(), car.GetAccel(), car.GetWheelAngleSpeed());
    }
    else
        StopMainTimer();

    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::SlotReceiveRepaint()
{
    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::initializeGL()
{
//    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
//    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glEnable(GL_DEPTH_TEST);
}
//-------------------------------------------------------------

void MainGLWidget::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    AspectRatio = w / static_cast<double>(h);

    if (MyFBO)
        delete MyFBO;

    QOpenGLFramebufferObjectFormat format;
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    MyFBO = new QOpenGLFramebufferObject(QSize(width(), height()), format);

    Model.OnResize(this->width(), this->height());
}
//-------------------------------------------------------------

void MainGLWidget::paintGL()
{
    fps++;
    tEndSecond = ClockType::now();

    double secTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(tEndSecond - tStartSecond).count();
    if (secTime >= 1000)
    {
        tStartSecond = ClockType::now();
        emit SignalSendCurrentFps(fps);
        fps = 0;
    }

    Model.DrawMeIn3D();

    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    ctx->extraFunctions()->glBindFramebuffer(GL_READ_FRAMEBUFFER, this->defaultFramebufferObject());
    ctx->extraFunctions()->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MyFBO->handle());
    ctx->extraFunctions()->glBlitFramebuffer(0, 0, width(), height(), 0, 0, MyFBO->width(), MyFBO->height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glGetIntegerv(GL_VIEWPORT, vport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modl);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);

    //qDebug() << __PRETTY_FUNCTION__;
}
//-------------------------------------------------------------

void MainGLWidget::keyPressEvent(QKeyEvent *pe)
{
    if (pe->isAutoRepeat())
        return;

//    qDebug() << __PRETTY_FUNCTION__;
    Model.OnKeyPress(pe);
    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::keyReleaseEvent(QKeyEvent *pe)
{
    if (pe->isAutoRepeat())
        return;

//    qDebug() << __PRETTY_FUNCTION__;
    Model.OnKeyRelease(pe);
    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::mousePressEvent(QMouseEvent *pe)
{
    Model.OnMousePress(pe, wExists, wx, wy);
    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::mouseMoveEvent(QMouseEvent *pe)
{
    if (pe->position().x() < 0 || pe->position().x() >= width())
        return;
    if (pe->position().y() < 0 || pe->position().y() >= height())
        return;

//    auto tStart = ClockType::now();

    if (WorkMode == WorkModeType::Nothing || WorkMode == WorkModeType::EditLine
            || WorkMode == WorkModeType::EditStart)
    {

        wExists = MouseToWorld_v3(pe->position().x(), pe->position().y(), wx, wy, wz);
    //    wExists = Model.MouseToWorld(pe->x(), pe->y(), this->height(), wx, wy, wz);
    //    auto tEnd = ClockType::now();
    //    auto t = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tEnd - tStart).count();
    //    t /= 1.0e6;
    //    qDebug() << "t mouse move get depth (mili) =" << t;
        emit SignalSendWorldCoords(wx, wy, wz, wExists);
    }

    Model.OnMouseMove(pe, wExists, wx, wy);

//    if (WorkMode != WorkModeType::Nothing || pe->buttons() & Qt::RightButton)
//        this->repaint();

    if ( WorkMode == WorkModeType::EditLine
         || WorkMode == WorkModeType::EditStart || pe->buttons() & Qt::RightButton)
        this->repaint();

}
//-------------------------------------------------------------

void MainGLWidget::mouseReleaseEvent(QMouseEvent *pe)
{
    Model.OnMouseRelease(pe, wExists, wx, wy);
    this->repaint();
}
//-------------------------------------------------------------

void MainGLWidget::wheelEvent(QWheelEvent *pe)
{
    Model.OnMouseWheel(pe);
    this->repaint();
}
//-------------------------------------------------------------

bool MainGLWidget::MouseToWorld_v3(int clientX, int clientY,
                             GLdouble &_worldX, GLdouble &_worldY, GLdouble &_worldZ)
{
    int X = clientX;
    int Y = this->height() - 1 - clientY;

    GLfloat depth;

    this->makeCurrent();

    MyFBO->bind();
    glReadPixels(X, Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    MyFBO->release();

    this->doneCurrent();

    //qDebug() << "depth =" << depth;

    if ( qFuzzyCompare(depth, 1) )
    {
        return false;
    }

    if ( gluUnProject(X, Y, depth, modl, proj, vport, &_worldX, &_worldY, &_worldZ) == GL_TRUE )
        return true;
    else
        return false;
}
//-------------------------------------------------------------


