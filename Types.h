#ifndef TYPES_H
#define TYPES_H

#include <vector>

constexpr float zOffset = 0.0001f;

constexpr int dt_60fps = 17;

enum class WorkModeType
{
    Nothing,
    EditLine,
    EditStart,
    RunningML,
    RunningNeuroCar,
    RunningManual,
    Competition
};

struct Settings3dType
{
    float TrX = 0;
    float TrY = 0;
    float TrZ = -1.8f;
    float RotX = 0;
    float RotY = 0;
    float RotZ = 0;
    //float OrthoZoom = 1.0f;
    bool IsPerspective = true;
};

//enum class ActivationFuncType
//{
//    Sigmoid,
//    Tanh,
//    ReLU
//};

struct GenStatsType
{
    double BestSsum = -1;
    double BestVaver = -1;
    int    Best_tInternal = -1;
    double AverSsum = -1;
    double AverVaver = -1;
    int    Aver_tInternal = -1;
};


class IDrawableIn3D
{
public:
    virtual void DrawMeIn3D() const = 0;
};

#endif // TYPES_H
