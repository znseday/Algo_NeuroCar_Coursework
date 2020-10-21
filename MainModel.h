#ifndef MAINMODEL_H
#define MAINMODEL_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

#include "Types.h"
#include "RaceMap.h"
#include "MyThreadPool.h"

//class MainGLWidget;

class MainModel : public QObject, public IDrawableIn3D
{
    Q_OBJECT

protected:

    int OldX = 0, CurrentX = 0;
    int OldY = 0, CurrentY = 0;

    int Width = 200, Height = 200;

    Settings3dType Settings3d;

    QString FileName = "Unknown";
    bool IsLoaded = false;

    bool ParseJson(const QJsonObject &_jsonObject);

    WorkModeType &WorkMode;

    QLineF LineStartVector;

    //std::vector<float> DepthMap;

    bool IsCameraOnCar = false;

    void MainLearningLoop();

    RaceMap Race;

    size_t ThreadCount = 1;
    MyThreadPool myThreadPool;

public:

    friend class DialogSettings;

    MainModel() = delete;
    MainModel(WorkModeType &_workMode);

    //void SetInfiniteMaxLifeTimeAndMinMAV() {Car::MaxLifeTime = -1; Car::MinMAVelocity = -1;}

    void SetWidthAndHeight(int _w, int _h) {Width = _w; Height = _h;}

    void New();
    void ClearButLastBestCar() {Race.ClearButLastBestCar();}

    bool LoadFromFile(const QString &_fileName);
    void SaveToFile(const QString &_fileName);

    void DrawMeIn3D() const override;

    void SwitchPerspective();

    void ResetViewPoint();

    void SwitchHideLines() {Race.SwitchHideLines();}

    void OnResize(int _width, int _height);

    void OnKeyPress(QKeyEvent *pe);
    void OnKeyRelease(QKeyEvent *pe);

    void OnMousePress(QMouseEvent *pe, bool wExists, double wx, double wy);
    void OnMouseMove(QMouseEvent *pe, bool wExists, double wx, double wy);
    void OnMouseRelease(QMouseEvent *pe, bool wExists, double wx, double wy);
    void OnMouseWheel(QWheelEvent *pe);

//    bool MouseToWorld(int ClientX, int ClientY, int ClientHeight,
//                       GLdouble &_worldX, GLdouble &_worldY, GLdouble &_worldZ);

//    bool MouseToWorld_v2(int clientX, int clientY,
//                       GLdouble &_worldX, GLdouble &_worldY, GLdouble &_worldZ);

    void AddNewLine();

    void StartML();
    void ContinueMLBasedOnFirstCar();

    void NextStep(double dt);

    void SwitchCamera() {IsCameraOnCar = !IsCameraOnCar;}

    bool LoadBrainFormFile(const QString &_fileName);
    void SaveLastBestBrainToFile(const QString &_fileName);

    const Car & GetFirstCar() const {return Race.Cars.front();}
    const Car & GetCurBestCar() const {return Race.GetCurBestCar();}
    const Car & GetLastBestCar() const {return Race.GetLastBestCar();}

    void CreateCompetition();

    void ClearAllLines() {Race.ClearAllLines();}

signals:

    void SignalSendPutStartFinished();
    void SignalSendRepaint();
    void SignalSendCarInfo(double v, double s, int t, double vaver, double accel, double wheelSpeed);
};

#endif // MAINMODEL_H
