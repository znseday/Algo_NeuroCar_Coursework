#ifndef CAR_H
#define CAR_H

#include <QPointF>
#include <QLineF>
#include <QVector2D>
#include <QColor>
#include <QJsonObject>
#include <QPolygonF>

#include <GL/glu.h>

#include <array>

#include "Types.h"
#include "NeuroNet.h"

constexpr double MAX_ACCEL = 0.15;
constexpr double MAX_WAS = 1.5;
constexpr double MAX_SIMPLE_SPEED = 0.16;
constexpr double MAX_SIMPLE_ANGLE = 30;

struct Detector
{
    QLineF Line;
    QPointF WallPoint;
    double Angle = 0;
    double Length = 0.3;
    double CurVal = 1;
    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject &_jsonObject);
    static std::vector<double> ParseStringToDoubles(QString _str);
    static QString DoublesToString(const std::vector<double> &_vals);
};

class Car : public IDrawableIn3D
{
private:

    QPolygonF CarPolygon;

    QPointF Pos;
    QVector2D V{1, 0};

    double Speed = 0;
    double Accel = 0;
    double WheelAngle = 0;
    double WheelAngleSpeed = 0;

    QVector2D Old_V;

    std::vector<Detector> Detectors;

    bool IsAlive = true;
    bool IsLastBest = false;
    bool IsCurBest = false;

    bool IsAdvanced = true;
    bool IsDrift = false;

    double Ssum = 0;
    int tInternal = 0;
    double Vaver = 0;
    double MAVelocity = -1;

    double OldSsumForMAV = 0;

    NeuroNet Brain;

    void DrawWheel(double zOffset) const;
    void DrawWheels(double thetaCar, double zOffset) const; 

public:

    static int MaxLifeTime;
    static double MinMAVelocity;
    static bool IsDrawEvenDead;
    static double MarginLevel;

    static GLUquadric* Quadric();

    Car();

    NeuroNet & BrainAccess() {return Brain;} // Стоит ли делать подобные методы или просто делать Brain в public?
    const NeuroNet & BrainAccess() const {return Brain;}

    QString GetDetectorsLengths() const;
    QString GetDetectorsAngles() const;

    virtual void DrawMeIn3D() const override;

    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject &_jsonObject);

    void CreateDetectors(const QString &_detectorLenghts, const QString &_detectorAngles);
    void CalcCarRectAndDetectors();

    void SetPos(const QPointF &_pos);
    void SetStartVByEndPos(const QPointF &_posTo);
    void SetStartV(const QVector2D &_startV);

    const QPointF & GetPos() const {return Pos;}
    const QVector2D & GetV() const {return V;}

    void SetSpeed(double _speed) {Speed = _speed;}
    void SetAccel(double _accel) {Accel = _accel;}
    void SetWheelAngle(double _wheelAngle) {WheelAngle = _wheelAngle;}
    void SetWheelAngleSpeed(double _wheelAngleSpeed) {WheelAngleSpeed = _wheelAngleSpeed;}

    double GetSpeed() const {return Speed;}
    double GetAccel() const {return Accel;}
    double GetWheelAngle() const {return WheelAngle;}
    double GetWheelAngleSpeed() const {return WheelAngleSpeed;}

    double GetIsAlive() const {return IsAlive;}

    bool GetIsAdvanced() const {return IsAdvanced;}
    void SetIsAdvanced(bool _isAdvanced) {IsAdvanced = _isAdvanced;}

    void GoManual(double dt);
    void GoNeuro(double dt);

    void CalcIntersects(const std::vector<std::vector<QLineF>> & _lines, bool _isML);

    void Reset(const QPointF &_startPoint, const QVector2D &_startVector);

    double GetSsum()    const {return Ssum;}
    int Get_tInternal() const {return tInternal;}
    double GetVaver()   const {return Vaver;}

    void MarkAsLastBest() {IsLastBest = true;}
    void MarkAsCurBest()  {IsCurBest = true;}
    void MarkAsPlain()    {IsCurBest = false;}
    bool GetIsLastBest() const {return IsLastBest;}
    bool GetIsCurBest() const {return IsCurBest;}

    size_t GetDetectorsCount() const {return Detectors.size();}

};

#endif // CAR_H
