#ifndef RACEMAP_H
#define RACEMAP_H

#include <vector>
#include <chrono>

#include <QLineF>
#include <QJsonObject>

#include "Types.h"
#include "Car.h"

#include "MyThreadPool.h"

using ClockType = std::chrono::steady_clock;

class RaceMap : public IDrawableIn3D
{
private: 
    std::vector<std::vector<QLineF>> Lines;
    QPointF StartPoint;
    QVector2D StartVector = QVector2D(1, 0);

    bool IsHideLines = false;

    BrainSettingsType BrainSettings;

    size_t PopulationSize = 300;
    double MutationCoeff = 0.5;
    bool IsCrossover = false;

    void ClearStatistics();
    void AddStatisticsForCurGeneration();

    std::vector<GenStatsType> GenStats;

    void CreateNextGeneration();
    void FindBestAndMark();
    void MutateAllButFirst();

    QString FirstReportFileName;
    QString ContinueReportFileName;
    bool IsContinueML = false;
    void CreateReportFile();

    decltype(ClockType::now()) tStartLearning;

public:

    friend class DialogSettings;

    std::vector<Car> Cars;

    RaceMap();

    void ClearAll();
    void ClearButLastBestCar();

    void CreateCompetition();

    void ResetFirstCar();

    void CreateFirstRandomPopulation();
    void CreateFirstPopulationBasedOnFirstCar();

    void SetStartPoint(double x, double y);
    void SetStartVector(const QVector2D &_startVector);
    void SetStartVectorByEndPos(const QPointF &_posTo);

    void AddNewLine();
    void AddNewPointToLine(double x, double y);

    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject _jsonObject);

    virtual void DrawMeIn3D() const override;

    void CalcIntersectsForFirstCar();
    void CalcIntersectsForTwoCars();

    void NextStepML(double dt, MyThreadPool &_myThreadPool, size_t _threadCount);

    bool LoadBrainFromFile(const QString &_fileName);
    void SaveLastBestBrainToFile(const QString &_fileName);

    const Car & GetFirstCar() const {return Cars.front();}
    const Car & GetCurBestCar() const;
    const Car & GetLastBestCar() const;

    void ClearAllLines() {Lines.clear(); CalcIntersectsForFirstCar();};
    void SwitchHideLines() {IsHideLines = !IsHideLines;}
};

#endif // RACEMAP_H
