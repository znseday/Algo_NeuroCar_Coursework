#include "MainModel.h"

#include <cmath>
#include <thread>
#include <future>

#include <QDebug>
#include <QApplication>

#include <QVector3D>
#include <QJsonArray>

constexpr float RotSpeed = 0.12f;
constexpr float TransSpeed = 0.0025f;

#include <chrono>
using ClockType = std::chrono::steady_clock;

//#include "MainGLWidget.h"

//-------------------------------------------------------------

MainModel::MainModel(WorkModeType &_workMode) : WorkMode(_workMode)
{

}
//-------------------------------------------------------------
//-------------------------------------------------------------

void MainModel::New()
{
    OldX = CurrentX = 0;
    OldY = CurrentY = 0;

    Settings3d = Settings3dType();

    FileName = "Unknown";
    IsLoaded = false;

    Race.ClearAll();
}
//-------------------------------------------------------------

void MainModel::DrawMeIn3D() const
{
//    double minSize = std::min(Width, Height);
//    glViewport(5, 5, minSize-5, minSize-5);
    glViewport(10, 10, Width-20, Height-20);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Settings3d.IsPerspective)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(45.0, Width/(double)Height, 0.01, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (IsCameraOnCar)
        {
//            Car car;
//            if (WorkMode == WorkModeType::RunningML)
//                car = Race.GetCurBestCar();
//            else
//                car = Race.GetFirstCar();

            const Car & car =
             (WorkMode == WorkModeType::RunningML) ?
                Race.GetCurBestCar()
            :
                Race.GetFirstCar();

            QPointF carPos = car.GetPos();
            QVector2D carV = car.GetV();

            QVector3D eye(carPos.x(), carPos.y(), 0);
            QVector3D ve(carV.x(), carV.y(), 0);
            ve.normalize();
            eye = eye - 0.18*ve;
            eye.setZ(0.05);

            gluLookAt(eye.x(), eye.y(), eye.z(),
                      carPos.x(), carPos.y(), 0,
                      0, 0, 1);
        }
        else
        {
            glTranslatef(Settings3d.TrX, Settings3d.TrY, Settings3d.TrZ);
            glRotatef(Settings3d.RotX, 1, 0, 0);
            glRotatef(Settings3d.RotY, 0, 1, 0);
            glRotatef(Settings3d.RotZ, 0, 0, 1);
        }
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        //gluOrtho2D(-1.04, 1.04, -1.04, 1.04);

        glOrtho(-1.0, 1.0, -Height/double(Width), Height/double(Width), -10, 10);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        glTranslatef(Settings3d.TrX, Settings3d.TrY, 0);
        float ks = fabs(1.0/(Settings3d.TrZ+2.5));
        glScalef(ks, ks, 1.0);
    }

    glPointSize(3.0f);
    glLineWidth(1.0f);


    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -0.75f);
        glVertex2f(-1.0f,  0.75f);
        glVertex2f( 1.0f,  0.75f);
        glVertex2f( 1.0f, -0.75f);
    glEnd();


    Race.DrawMeIn3D();

    if (WorkMode == WorkModeType::EditStart)
    {
        glLineWidth(1.0f);
        glColor3f(0.9f, 0.1f, 0.1f);
        glBegin(GL_LINES);
            glVertex3f(LineStartVector.p1().x(), LineStartVector.p1().y(), 3.5*zOffset);
            glVertex3f(LineStartVector.p2().x(), LineStartVector.p2().y(), 3.5*zOffset);
        glEnd();
    }
}
//-------------------------------------------------------------

void MainModel::SwitchPerspective()
{
    Settings3d.IsPerspective = !Settings3d.IsPerspective;
    //DrawMeIn3D();
}
//-------------------------------------------------------------

void MainModel::ResetViewPoint()
{
    Settings3d = Settings3dType();
    //DrawMeIn3D();
}
//-------------------------------------------------------------

void MainModel::OnResize(int _width, int _height)
{
    SetWidthAndHeight(_width, _height);
    //DepthMap.resize(Width*Height);
}
//-------------------------------------------------------------

void MainModel::OnKeyPress(QKeyEvent *pe)
{
    if (WorkMode != WorkModeType::RunningManual &&
        WorkMode != WorkModeType::Competition )
            return;

    Car &car = Race.Cars.front();

    switch (pe->key())
    {
    case Qt::Key_Up:
        if (car.GetIsAdvanced())
            car.SetAccel(MAX_ACCEL);
        else
            car.SetSpeed(MAX_SIMPLE_SPEED);
        break;
    case Qt::Key_Down:
        if (car.GetIsAdvanced())
            car.SetAccel(-MAX_ACCEL);
        else
            car.SetSpeed(0);
        break;
    case Qt::Key_Left:
        if (car.GetIsAdvanced())
            car.SetWheelAngleSpeed(MAX_WAS);
        else
            car.SetWheelAngle(MAX_SIMPLE_ANGLE);
        break;
    case Qt::Key_Right:
        if (car.GetIsAdvanced())
            car.SetWheelAngleSpeed(-MAX_WAS);
        else
            car.SetWheelAngle(-MAX_SIMPLE_ANGLE);
        break;
    }
}
//-------------------------------------------------------------

void MainModel::OnKeyRelease(QKeyEvent *pe)
{
    if (WorkMode != WorkModeType::RunningManual &&
        WorkMode != WorkModeType::Competition )
            return;

    Car &car = Race.Cars.front();

    switch (pe->key())
    {
    case Qt::Key_Up:
        if (car.GetIsAdvanced())
            car.SetAccel(0);
        else
            car.SetSpeed(0);
        break;
    case Qt::Key_Down:
        if (car.GetIsAdvanced())
            car.SetAccel(0);
        else
            car.SetSpeed(0);
        break;
    case Qt::Key_Left:
        if (car.GetIsAdvanced())
            car.SetWheelAngleSpeed(0);
        else
            car.SetWheelAngle(0);
        break;
    case Qt::Key_Right:
        if (car.GetIsAdvanced())
            car.SetWheelAngleSpeed(0);
        else
            car.SetWheelAngle(0);
        break;
    }
}
//-------------------------------------------------------------

void MainModel::OnMousePress(QMouseEvent *pe, bool wExists, double wx, double wy)
{
    OldX = pe->x();
    OldY = pe->y();

    if (pe->button() == Qt::LeftButton)
    {
        if (WorkMode == WorkModeType::EditStart && wExists)
        {
            LineStartVector.setP1({wx, wy});
            LineStartVector.setP2({wx, wy});
            Race.SetStartPoint(wx, wy);
            Race.ResetFirstCar();
            Race.CalcIntersectsForFirstCar();
        }
        else if (WorkMode == WorkModeType::EditLine && wExists)
        {
            Race.AddNewPointToLine(wx, wy);
            Race.ResetFirstCar();
            Race.CalcIntersectsForFirstCar();
        }
    }
    else if (pe->button() == Qt::RightButton)
    {
//        OldX = pe->x();
//        OldY = pe->y();
    }

    if (pe->type() == QMouseEvent::MouseButtonDblClick)
    {
    }
}
//-------------------------------------------------------------

void MainModel::OnMouseMove(QMouseEvent *pe, bool wExists, double wx, double wy)
{
    if (pe->buttons() & Qt::LeftButton)
    {
        if (WorkMode == WorkModeType::EditStart && wExists)
        {
            LineStartVector.setP2({wx, wy});
            Race.SetStartVectorByEndPos({wx, wy});
            Race.ResetFirstCar();
            Race.CalcIntersectsForFirstCar();
        }

        return;
    }
    if (pe->buttons() & Qt::RightButton)
    {
        CurrentX = pe->x();
        CurrentY = pe->y();

        int dx = CurrentX - OldX;
        int dy = CurrentY - OldY;

        if (QApplication::keyboardModifiers() == Qt::ControlModifier)
        {
            if (Settings3d.IsPerspective)
                Settings3d.RotZ += RotSpeed*dx;
        }
        else if (QApplication::keyboardModifiers() == Qt::AltModifier)
        {
            Settings3d.TrX += TransSpeed*dx;
            Settings3d.TrY -= TransSpeed*dy;
        }
        else
        {
            if (Settings3d.IsPerspective)
            {
                Settings3d.RotX += RotSpeed*dy;
                Settings3d.RotY += RotSpeed*dx;
            }
        }

        OldX = CurrentX;
        OldY = CurrentY;
    }
}
//-------------------------------------------------------------

void MainModel::OnMouseRelease([[maybe_unused]] QMouseEvent *pe, bool wExists, double wx, double wy)
{
    if (WorkMode == WorkModeType::EditStart && wExists)
    {
        Race.SetStartVectorByEndPos({wx, wy});
        Race.ResetFirstCar();
        Race.CalcIntersectsForFirstCar();
        emit SignalSendPutStartFinished();

        return;
    }
}
//-------------------------------------------------------------

void MainModel::OnMouseWheel(QWheelEvent *pe)
{
    //if (Settings3d.IsPerpective)
        Settings3d.TrZ += pe->angleDelta().y() / 1650.0f;
}
//-------------------------------------------------------------

bool MainModel::ParseJson(const QJsonObject &_jsonObject)
{
    qDebug() << "Model:";
    qDebug() << "FileName = " << FileName;

    const QJsonObject &raceObject = _jsonObject["Race"].toObject();
    Race.LoadFromJsonObject(raceObject);

    Car::MaxLifeTime = _jsonObject["MaxLifeTime"].toInt();
    Car::MinMAVelocity = _jsonObject["MinMAVelocity"].toDouble();
    Car::MarginLevel = _jsonObject["MarginLevel"].toDouble();

    ThreadCount = _jsonObject["ThreadCount"].toInt(2);

    return true;
}
//-------------------------------------------------------------

bool MainModel::LoadFromFile(const QString &_fileName)
{
    New();

    QFile json(_fileName);
    if (json.open(QIODevice::ReadOnly))
    {
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            if (jsonDoc.isObject())
            {
                FileName = _fileName;

                if (!ParseJson(jsonDoc.object()) )
                    return false;
            }
        }
        else
        {
            qDebug() << parseError.errorString();
            return false;
        }
    }
    else
    {
        qDebug() << "json file not open";
        return false;
    }

    Race.CalcIntersectsForFirstCar();

    IsLoaded = true;
    return true;
}
//-------------------------------------------------------------

void MainModel::SaveToFile(const QString &_fileName)
{
    QJsonDocument jsonDoc;
    QJsonObject mainObject;

    mainObject.insert("Race", Race.RepresentAsJsonObject());
    mainObject.insert("MaxLifeTime", Car::MaxLifeTime);
    mainObject.insert("MinMAVelocity", Car::MinMAVelocity);
    mainObject.insert("MarginLevel", Car::MarginLevel);

    mainObject.insert("ThreadCount", (int)ThreadCount);

    jsonDoc.setObject(mainObject);

    QFile jsonFile(_fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(jsonDoc.toJson());
}
//-------------------------------------------------------------

void MainModel::AddNewLine()
{
    Race.AddNewLine();
}
//-------------------------------------------------------------

void MainModel::MainLearningLoop()
{
    if (ThreadCount > 1)
        myThreadPool.Init(ThreadCount);

    while (WorkMode == WorkModeType::RunningML)
    {
        Race.NextStepML(dt_60fps, myThreadPool, ThreadCount);

        emit SignalSendRepaint();

        const auto & car = Race.GetCurBestCar();
        emit SignalSendCarInfo(car.GetSpeed(), car.GetSsum(), car.Get_tInternal(), car.GetVaver(), car.GetAccel(), car.GetWheelAngleSpeed());

        QApplication::processEvents();
    }

    myThreadPool.FinishAndRelease();
}
//-------------------------------------------------------------

void MainModel::StartML()
{
    Race.CreateFirstRandomPopulation();
    MainLearningLoop();
}
//-------------------------------------------------------------

void MainModel::ContinueMLBasedOnFirstCar()
{
    Race.CreateFirstPopulationBasedOnFirstCar();
    MainLearningLoop();
}
//-------------------------------------------------------------

void MainModel::NextStep(double dt)
{
    if (WorkMode == WorkModeType::RunningManual)
    {
        Race.Cars.front().GoManual(dt);
        Race.CalcIntersectsForFirstCar();
    }
    else if (WorkMode == WorkModeType::RunningNeuroCar)
    {
        Race.Cars.front().GoNeuro(dt);
        Race.CalcIntersectsForFirstCar();
    }
    else if (WorkMode == WorkModeType::Competition)
    {
        Race.Cars.front().GoManual(dt);
        Race.Cars.at(1).GoNeuro(dt);
        Race.CalcIntersectsForTwoCars();
    }
}
//-------------------------------------------------------------

bool MainModel::LoadBrainFormFile(const QString &_fileName)
{
    return Race.LoadBrainFromFile(_fileName);
}
//-------------------------------------------------------------

void MainModel::SaveLastBestBrainToFile(const QString &_fileName)
{
    Race.SaveLastBestBrainToFile(_fileName);
}
//-------------------------------------------------------------

void MainModel::CreateCompetition()
{
    Race.CreateCompetition();
}
//-------------------------------------------------------------
