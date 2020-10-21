#include "Car.h"

#include <GL/gl.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#include <cmath>
#include <algorithm>

constexpr double Car_dx = 0.016;
constexpr double Car_dy = 0.008;

int Car::MaxLifeTime = 1400;
double Car::MinMAVelocity = 0.02;
bool Car::IsDrawEvenDead = true;
double Car::MarginLevel = 0.03;


QJsonObject Detector::RepresentAsJsonObject() const
{
    QJsonObject resultObject;
    resultObject.insert("Length", Length);
    resultObject.insert("Angle", Angle);
    return resultObject;
}
//-------------------------------------------------------------

void Detector::LoadFromJsonObject(const QJsonObject &_jsonObject)
{
    Length = _jsonObject["Length"].toDouble(0.333);
    Angle = _jsonObject["Angle"].toDouble(123);
}
//-------------------------------------------------------------

std::vector<double> Detector::ParseStringToDoubles(QString _str)
{
    std::vector<double> res;
    int pos;
    do
    {
        pos = _str.indexOf('|');
        if (pos > 0)
        {
            res.emplace_back(_str.left(pos).toDouble(0));
            _str = _str.right(_str.size()-pos-1);
        }
        else
            res.emplace_back(_str.toDouble(0));
    } while (pos >= 0);
    res.shrink_to_fit();
    return res;
}
//-------------------------------------------------------------

QString Detector::DoublesToString(const std::vector<double> &_vals)
{
    QString res;
    for (size_t i = 0; i < _vals.size(); ++i)
    {
        res += _vals[i];
        if (i < _vals.size() - 1)
            res += "|";
    }
    return res;
}
//-------------------------------------------------------------
//-------------------------------------------------------------

static QPointF RotatePointF(const QPointF &p, double theta)
{
    return {p.x()*cos(theta) - p.y()*sin(theta), p.x()*sin(theta) + p.y()*cos(theta)};
}

static QLineF RotateLineF(const QLineF &line, double theta)
{
    return {RotatePointF(line.p1(), theta), RotatePointF(line.p2(), theta)};
}
//-------------------------------------------------------------
//-------------------------------------------------------------

GLUquadric* Car::Quadric()
{
    static GLUquadric* q = nullptr;
    if (!q)
        q = gluNewQuadric();
    return q;
}
//-------------------------------------------------------------

Car::Car()
{
    CarPolygon << QPointF(0, 0) << QPointF(0, 0) << QPointF(0, 0) << QPointF(0, 0);
    CreateDetectors("0.8|0.4|0.4|0.2|0.2", "0|-20|20|-45|45");
    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------

QString Car::GetDetectorsLengths() const
{
    QString res;
    for (size_t i = 0; i < Detectors.size(); ++i)
    {
        res += QString().setNum(Detectors[i].Length);
        if (i < Detectors.size() - 1)
            res += "|";
    }
    return res;
}
//-------------------------------------------------------------

QString Car::GetDetectorsAngles() const
{
    QString res;
    for (size_t i = 0; i < Detectors.size(); ++i)
    {
        res += QString().setNum(Detectors[i].Angle);
        if (i < Detectors.size() - 1)
            res += "|";
    }
    return res;
}
//-------------------------------------------------------------

void Car::DrawMeIn3D() const
{
    double theta = QLineF(0, 0, V.x(), V.y()).angle();
    theta = -theta;
    //double thetaCar = theta;

    if (IsLastBest)
        glColor3f(0.1f, 0.1f, 0.9f);
    else if (IsCurBest)
        glColor3f(0.9f, 0.1f, 0.1f);
    else
    {
        if (IsAlive)
            glColor3f(0.1f, 0.9f, 0.1f);
        else
            glColor3f(0, 0, 0);
    }

    glBegin(GL_QUADS);
        for (const auto & p : CarPolygon)
            glVertex3f(p.x(), p.y(), 2*zOffset);
    glEnd();

    if (IsAlive || IsDrawEvenDead)
    {
        glLineWidth(1.0f);
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_LINES);
            for (const auto & d : Detectors)
            {
                glVertex3f(d.Line.x1(), d.Line.y1(), 2.5*zOffset);
                if (d.CurVal < d.Length/*+0.000000001*/)
                    glVertex3f(d.WallPoint.x(), d.WallPoint.y(), 2.5*zOffset);
                else
                    glVertex3f(d.Line.x2(), d.Line.y2(), 2.5*zOffset);
            }
        glEnd();

        for (const auto & d : Detectors)
        {
            glColor3f(0.2f, 0.2f, 0.2f);
            glPushMatrix();
                glTranslatef(d.Line.x1(), d.Line.y1(), 2.2*zOffset);
                gluDisk(Quadric(), 0, 0.0015, 6, 2);
            glPopMatrix();

            glPushMatrix();

                if (d.CurVal < d.Length)
                {
//                    glColor3f(0.9f - (d.CurVal / d.Length)*0.9f, 0.1f + (d.CurVal / d.Length)*0.9f, 0.0f);
                    glColor3f(0.9f, 0.2f, 0.2f);
                    glTranslatef(d.WallPoint.x(), d.WallPoint.y(), 2.2*zOffset);
                }
                else
                {
                    glColor3f(0.2f, 0.2f, 0.2f);
                    glTranslatef(d.Line.x2(), d.Line.y2(), 2.2*zOffset);
                }

                gluDisk(Quadric(), 0, 0.003, 6, 2);
            glPopMatrix();
        }
    }

    DrawWheels(theta /*thetaCar*/, 3.5*zOffset);
}
//-------------------------------------------------------------

void Car::DrawWheel(double zOffset) const
{
    glBegin(GL_LINES);
        glVertex3f(-Car_dx/5.0, 0, zOffset);
        glVertex3f( Car_dx/5.0, 0, zOffset);
    glEnd();
}
//-------------------------------------------------------------

void Car::DrawWheels(double thetaCar, double zOffset) const
{
    glColor3f(0.9f, 0.9f, 0.1f);
    glLineWidth(3.0f);

    glPushMatrix();

        glTranslatef(Pos.x(), Pos.y(), 0);
        glRotatef(thetaCar, 0, 0, 1);

        glPushMatrix();
            glTranslatef(Car_dx*0.75, Car_dy, 0);
            glRotatef(WheelAngle, 0, 0, 1);
            DrawWheel(zOffset);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(Car_dx*0.75, -Car_dy, 0);
            glRotatef(WheelAngle, 0, 0, 1);
            DrawWheel(zOffset);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-Car_dx*0.75, Car_dy, 0);
            DrawWheel(zOffset);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-Car_dx*0.75, -Car_dy, 0);
            DrawWheel(zOffset);
        glPopMatrix();

    glPopMatrix();
}
//-------------------------------------------------------------

QJsonObject Car::RepresentAsJsonObject() const
{
    QJsonObject resultObject;

    resultObject.insert("IsAdvanced", IsAdvanced);

    QJsonArray detectorsArray;
    for (const auto & d : Detectors)
    {
        detectorsArray.append(QJsonValue(d.RepresentAsJsonObject()));
    }

    resultObject.insert("Detectors", detectorsArray);

    return resultObject;
}
//-------------------------------------------------------------

void Car::LoadFromJsonObject(const QJsonObject &_jsonObject)
{
    IsAdvanced = _jsonObject["IsAdvanced"].toBool(true);

    const QJsonArray &detectorsArray = _jsonObject["Detectors"].toArray();

    Detectors.clear();
    for (auto it = detectorsArray.begin(); it != detectorsArray.end(); ++it)
    {
        Detectors.push_back(Detector());
        Detectors.back().LoadFromJsonObject(it->toObject());
    }
    Detectors.shrink_to_fit();
}
//-------------------------------------------------------------

void Car::CreateDetectors(const QString &_detectorLenghts, const QString &_detectorAngles)
{
    auto lenghts = Detector::ParseStringToDoubles(_detectorLenghts);
    auto angles  = Detector::ParseStringToDoubles(_detectorAngles);
    if (lenghts.size() != angles.size())
        throw std::runtime_error("lenghts.size() != angles.size()");

    Detectors.clear();
    Detectors.resize(lenghts.size());
    for (size_t i = 0; i < Detectors.size(); ++i)
    {
        Detectors[i].Length = lenghts[i];
        Detectors[i].CurVal = Detectors[i].Length;
        Detectors[i].Angle = angles[i];
    }
}
//-------------------------------------------------------------

void Car::CalcCarRectAndDetectors()
{
    double theta = QLineF(0, 0, V.x(), V.y()).angle();

    theta = -theta;
    theta *= M_PI/180.0;

    CarPolygon[0] = RotatePointF({-Car_dx, -Car_dy}, theta);
    CarPolygon[1] = RotatePointF({-Car_dx,  Car_dy}, theta);
    CarPolygon[2] = RotatePointF({ Car_dx,  Car_dy}, theta);
    CarPolygon[3] = RotatePointF({ Car_dx, -Car_dy}, theta);

    CarPolygon[0] += Pos;
    CarPolygon[1] += Pos;
    CarPolygon[2] += Pos;
    CarPolygon[3] += Pos;

    double ddx;

    if (Detectors.size() > 4)
        ddx = Car_dx/((Detectors.size()-1)/2 - 1);
    else
        ddx = Car_dx;

    for (size_t i = 0; i < Detectors.size(); ++i)
    {
        if (i == 0)
            Detectors[i].Line = {Car_dx, 0, 1, 0};
        else
        {
            if (i%2)
                Detectors[i].Line = {Car_dx-ddx*(i/2), Car_dy, 1, Car_dy};
            else
                Detectors[i].Line = {Car_dx-ddx*((i-1)/2), -Car_dy, 1, -Car_dy};
        }


        Detectors[i].Line.setAngle(Detectors[i].Angle);
        Detectors[i].Line.setLength(Detectors[i].Length);
        Detectors[i].Line = RotateLineF(Detectors[i].Line, theta);
        Detectors[i].Line.translate(Pos);
    }
}
//-------------------------------------------------------------

void Car::SetPos(const QPointF &_pos)
{
    Pos = _pos;
    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------

void Car::SetStartVByEndPos(const QPointF &_posTo)
{
    if (_posTo == Pos)
        V = {1, 0};
    else
    {
        V = {float(_posTo.x() - Pos.x()), float(_posTo.y() - Pos.y())};
        V.normalize();
    }
    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------

void Car::SetStartV(const QVector2D &_startV)
{
    if (_startV.isNull())
        V = {1, 0};
    else
        V = _startV.normalized();

    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------

void Car::GoManual(double dt)
{
    dt /= 1.0e3;
    constexpr double kDF = 0.7;
    constexpr double kDF2 = 0.5;
    constexpr double kDFw = 0.0005; //0.001;
    constexpr double kDFd = 0.003;

    if (IsAdvanced)
    {
        constexpr double driftTrashHold = 0.7;

        double driftRate = Speed*Speed*fabs(WheelAngle)/(0.9*MAX_SIMPLE_SPEED*MAX_SIMPLE_SPEED*MAX_SIMPLE_ANGLE);
        driftRate *= 0.5*(1 + tanh((driftRate-driftTrashHold)*10.0));
        if (driftRate < 0.1)
            driftRate = 0;
        if (driftRate > 1)
            driftRate = 1;
        //qDebug() << "driftRate =" << driftRate;


        double dampingForce = Speed*kDF + Speed*Speed*kDF2  + kDFw*fabs(WheelAngle) - kDFd*sqrt(driftRate);
        double Feff = Accel - dampingForce;
        Speed += Feff*dt;
        if (Speed < 0)
            Speed = 0;

        WheelAngle = (WheelAngle + WheelAngleSpeed*30*dt)/pow(1 + Speed*dt*10.0, 1.5);

        if (WheelAngle > MAX_SIMPLE_ANGLE)
            WheelAngle = MAX_SIMPLE_ANGLE;
        if (WheelAngle < -MAX_SIMPLE_ANGLE)
            WheelAngle = -MAX_SIMPLE_ANGLE;

//        driftRate = sqrt(driftRate);
//        Pos.setX(Pos.x()+V.x()*Speed*dt*(pow(driftRate, 0.05)));
//        Pos.setY(Pos.y()+V.y()*Speed*dt*(pow(driftRate, 0.05)));
////        V = QVector2D(RotatePointF(V.toPointF(), WheelAngle*Speed*dt*0.4*(1-sqrt(driftRate))));
//        V = QVector2D(RotatePointF(V.toPointF(), WheelAngle*Speed*dt*0.4*(1-driftRate*driftRate*driftRate)));

        if (!IsDrift && driftRate > driftTrashHold)
        {
            IsDrift = true;
            Old_V = V;
        }

        if (driftRate <= driftTrashHold)
        {
            IsDrift = false;
        }

        V = QVector2D(RotatePointF(V.toPointF(), WheelAngle*Speed*dt*0.4*(1-driftRate)));

        if (IsDrift)
        {
            //constexpr double kddd = 0.9;

            Pos.setX(Pos.x()+Old_V.x()*Speed*dt*driftRate);
            Pos.setY(Pos.y()+Old_V.y()*Speed*dt*driftRate);
            Pos.setX(Pos.x()+V.x()*Speed*dt*(1-driftRate));
            Pos.setY(Pos.y()+V.y()*Speed*dt*(1-driftRate));
        }
        else
        {
            Pos.setX(Pos.x()+V.x()*Speed*dt);
            Pos.setY(Pos.y()+V.y()*Speed*dt);
        }
    }
    else
    {
        V = QVector2D(RotatePointF(V.toPointF(), WheelAngle*Speed*dt*0.4));
        Pos.setX(Pos.x()+V.x()*Speed*dt);
        Pos.setY(Pos.y()+V.y()*Speed*dt);
    }

    tInternal++;
    Ssum += Speed*1.0;
    Vaver = Ssum/double(tInternal);

    constexpr int dtMAV = 50;
    if (tInternal % dtMAV == 0)
    {
        MAVelocity = (Ssum-OldSsumForMAV)/dtMAV;
        OldSsumForMAV = Ssum;
    }

    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------

void Car::GoNeuro(double dt)
{
    if (IsAdvanced)
    {
        if (Detectors.size()+2 != Brain.GetInputCount())
        {
            throw std::runtime_error("Detectors.size()+2 != Brain.GetInputCount()");
            return;
        }

        if (Brain.GetOutputCount() != 2)
        {
            throw std::runtime_error("Brain.GetOutputCount() != 2");
            return;
        }

        for (size_t i = 0; i < Detectors.size(); ++i)
            Brain.SetInput(i, 1 - sqrt(Detectors[i].CurVal/Detectors[i].Length));

       // Brain.SetInput(0, pow(Detectors[0].CurVal/Detectors[0].Length, 0.5)); // ?


        Brain.SetInput(Detectors.size(),   Speed/MAX_SIMPLE_SPEED);
        Brain.SetInput(Detectors.size()+1, WheelAngle/MAX_SIMPLE_ANGLE);

//        Brain.SetInput(Detectors.size(), 0);
//        Brain.SetInput(Detectors.size()+1, 0);

        Brain.CalcAll();

        double accel;
        double wheelSpeed;
        auto ptr = Brain.GetActivationFinal().target<double(*)(double)>();
        if (ptr && *ptr == MySigmoidFunc)
        {
            accel =    -MAX_ACCEL + 2.0*MAX_ACCEL*Brain.GetOutput(0);
            wheelSpeed = -MAX_WAS + 2.0*MAX_WAS*Brain.GetOutput(1);
        }
        else if (ptr && *ptr == tanh)
        {
            accel = Brain.GetOutput(0)*MAX_ACCEL;
            wheelSpeed = Brain.GetOutput(1)*MAX_WAS;
        }
        else if (ptr && *ptr == MyReLU)
        {
            double accelOutput = Brain.GetOutput(0);
//            if (accelOutput > 2) // ?
//                IsAlive = false;

            accel =    -MAX_ACCEL + 2.0*MAX_ACCEL*std::min(accelOutput, 1.0);
            wheelSpeed = -MAX_WAS + 2.0*MAX_WAS*std::min(Brain.GetOutput(1), 1.0);

        }
        else
            throw std::runtime_error("UNKNOWN Brain.GetActivationFinal().target<double(*)(double)>()");


        SetAccel(accel);
        SetWheelAngleSpeed(wheelSpeed);

    }
    else // Simple physics
    {
        if (Detectors.size() != Brain.GetInputCount())
        {
            throw std::runtime_error("Detectors.size() != Brain.GetInputCount()");
            return;
        }

        if (Brain.GetOutputCount() != 2)
        {
            throw std::runtime_error("Brain.GetOutputCount() != 2");
            return;
        }

        for (size_t i = 0; i < Detectors.size(); ++i)
            Brain.SetInput(i, 1 - sqrt(Detectors[i].CurVal/Detectors[i].Length));

        Brain.CalcAll();

        double speed;
        double wheelAngle;
        auto ptr = Brain.GetActivationFinal().target<double(*)(double)>();
        if (ptr && *ptr == MySigmoidFunc)
        {
            speed =      MAX_SIMPLE_SPEED*Brain.GetOutput(0);
            wheelAngle = -MAX_SIMPLE_ANGLE + 2.0*MAX_SIMPLE_ANGLE*Brain.GetOutput(1);
        }
        else if (ptr && *ptr == tanh)
        {
            speed      = MAX_SIMPLE_SPEED/2.0 + MAX_SIMPLE_SPEED/2.0*Brain.GetOutput(0);
            wheelAngle = MAX_SIMPLE_ANGLE*Brain.GetOutput(1);
        }
        else if (ptr && *ptr == MyReLU)
        {
            speed =      MAX_SIMPLE_SPEED*std::min(Brain.GetOutput(0), 1.0);
            wheelAngle = -MAX_SIMPLE_ANGLE + 2.0*MAX_SIMPLE_ANGLE*std::min(Brain.GetOutput(1), 1.0);
        }
        else
            throw std::runtime_error("UNKNOWN Brain.GetActivationFinal().target<double(*)(double)>()");

        if (speed < 0 || speed > MAX_SIMPLE_ANGLE)
        {
            qDebug() << "!!!!!!!! speed < 0 || speed > MAX_SIMPLE_ANGLE";
        }
        if (wheelAngle < -MAX_SIMPLE_ANGLE || wheelAngle > MAX_SIMPLE_ANGLE)
        {
            qDebug() << "!!!!!!!! wheelAngle < -MAX_SIMPLE_ANGLE || wheelAngle > MAX_SIMPLE_ANGLE";
        }

        SetSpeed(speed);
        SetWheelAngle(wheelAngle);
    }

    GoManual(dt);

    if (/*MinMAVelocity > 0 &&*/ MAVelocity >= 0 && MAVelocity < MinMAVelocity)
    {
        //qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        //qDebug() << "too slow, MAVelocity =" << MAVelocity;
        IsAlive = false;
    }

    if (/*MaxLifeTime > 0 &&*/ tInternal >= MaxLifeTime)
    {
        //qDebug() << "!!!!!! too long, tInternal =" << tInternal;
        IsAlive = false;
    }
}
//-------------------------------------------------------------

void Car::CalcIntersects(const std::vector<std::vector<QLineF> > &_lines, bool _isML)
{
    QPointF interPoint;
    std::vector<std::pair<QPointF, double>> interPoints;
    interPoints.reserve(7);
    for (auto & d : Detectors)
    {
        interPoints.clear();
        for (const auto & line : _lines)
        {
            for (const auto & l : line)
            {
                if (d.Line.intersect(l, &interPoint) == QLineF::BoundedIntersection)
                {
                    double len = QLineF(d.Line.p1(), interPoint).length();
                    interPoints.emplace_back(interPoint, len);
                }
            }
        }

        if (interPoints.empty())
        {
            d.WallPoint = d.Line.p2();
            d.CurVal = d.Length;
        }
        else
        {
            auto itMin = std::min_element(interPoints.begin(), interPoints.end(),[]
                                                (const std::pair<QPointF, double> &a, const std::pair<QPointF, double> &b)
                                                {return a.second < b.second;});
            d.WallPoint = itMin->first;
            d.CurVal = itMin->second;
        }
    }

    std::array<QLineF, 4> carLines;
    carLines[0] = {CarPolygon[0], CarPolygon[1]};
    carLines[1] = {CarPolygon[1], CarPolygon[2]};
    carLines[2] = {CarPolygon[2], CarPolygon[3]};
    carLines[3] = {CarPolygon[3], CarPolygon[0]};

    for (const auto & cl : carLines)
    {
        for (const auto & line : _lines)
        {
            if ( std::any_of(line.begin(), line.end(), [cl]
                                                (const QLineF &l)
                                                { return cl.intersect(l, nullptr) == QLineF::BoundedIntersection; }) )
            {
                IsAlive = false;
            }
        }
    }

    if (_isML)
    {
        if (std::any_of(Detectors.begin(), Detectors.end(), []
                                                (const Detector &d)
                                                {return d.CurVal/d.Length < MarginLevel;}))
        {
            IsAlive = false;
        }
    }
}
//-------------------------------------------------------------

void Car::Reset(const QPointF &_startPoint, const QVector2D &_startVector)
{
    Pos = _startPoint;
    V = _startVector;
    Speed = 0;
    Accel = 0;
    WheelAngle = 0;
    WheelAngleSpeed = 0;
    IsAlive = true;
    Ssum = 0;
    tInternal = 0;
    Vaver = 0;
    MAVelocity = -1;
    OldSsumForMAV = 0;
    IsLastBest = false;
    IsCurBest = false;

    CalcCarRectAndDetectors();
}
//-------------------------------------------------------------







