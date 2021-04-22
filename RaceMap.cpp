#include "RaceMap.h"

#include <QDebug>
#include <QJsonArray>
#include <QDateTime>

//#include <thread>
//#include <future>
#include <fstream>

RaceMap::RaceMap()
{
    BrainSettings.ActivationFuncGeneral = MyReLU;
    BrainSettings.ActivationFuncFinal = MyReLU;
    BrainSettings.IsXavier = false;

    BrainSettings.Topology.emplace_back(7);
    //BrainSettings.Topology.emplace_back(14);
    BrainSettings.Topology.emplace_back(28);
    //BrainSettings.Topology.emplace_back(14);
    BrainSettings.Topology.emplace_back(2);

    //ClearAll();
}
//-------------------------------------------------------------

void RaceMap::ClearAll()
{
    Lines.clear();
    StartPoint = QPointF();
    StartVector = QVector2D(1, 0);

    Cars.clear();
    Cars.emplace_back(Car());
    //Cars.front().CalcCarRectAndDetectors();
    Cars.front().BrainAccess().InitTopology(BrainSettings.Topology,
                                            BrainSettings.ActivationFuncGeneral,
                                            BrainSettings.ActivationFuncFinal, BrainSettings.IsXavier);

    CalcIntersectsForFirstCar();
}
//-------------------------------------------------------------

void RaceMap::ClearButLastBestCar()
{
    ResetFirstCar();
    Car tempCar = GetLastBestCar();
    Cars.clear();

    Cars.emplace_back(tempCar);
    CalcIntersectsForFirstCar();
}
//-------------------------------------------------------------

void RaceMap::CreateCompetition()
{
    ResetFirstCar();
    Car tempCar = GetLastBestCar();
    Cars.clear();

    Cars.emplace_back(tempCar);
    Cars.emplace_back(tempCar);

    Cars.back().MarkAsCurBest();

    CalcIntersectsForFirstCar();
}
//-------------------------------------------------------------

void RaceMap::ResetFirstCar()
{
    Cars.front().Reset(StartPoint, StartVector);
}
//-------------------------------------------------------------

void RaceMap::ClearStatistics()
{
    tStartLearning = ClockType::now();
    GenStats.clear();
}
//-------------------------------------------------------------

void RaceMap::AddStatisticsForCurGeneration()
{
    auto tEnd = ClockType::now();
    auto t = (double)std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStartLearning).count();
    t /= (1.0e3 * 60.0);

    GenStatsType genStats;
    auto it = std::max_element(Cars.begin(), Cars.end(), [](const Car & carA, const Car & carB){return carA.GetSsum() < carB.GetSsum();});
    genStats.BestSsum = it->GetSsum();

    it = std::max_element(Cars.begin(), Cars.end(), [](const Car & carA, const Car & carB){return carA.GetVaver() < carB.GetVaver();});
    genStats.BestVaver = it->GetVaver();

    it = std::max_element(Cars.begin(), Cars.end(), [](const Car & carA, const Car & carB){return carA.Get_tInternal() < carB.Get_tInternal();});
    genStats.Best_tInternal = it->Get_tInternal();

    genStats.AverSsum  = std::accumulate(Cars.begin(), Cars.end(), 0.0, [](double &sum, const Car & car){return sum + car.GetSsum();}) / Cars.size();
    genStats.AverVaver = std::accumulate(Cars.begin(), Cars.end(), 0.0, [](double &sum, const Car & car){return sum + car.GetVaver();}) / Cars.size();
    genStats.Aver_tInternal = std::accumulate(Cars.begin(), Cars.end(), 0.0, [](double &sum, const Car & car){return sum + car.Get_tInternal();}) / Cars.size();

    GenStats.emplace_back(genStats);

    qDebug() << "genStats.BestSsum =" << genStats.BestSsum;
    qDebug() << "genStats.BestVaver =" << genStats.BestVaver;
    qDebug() << "genStats.Best_tInternal =" << genStats.Best_tInternal;
    qDebug() << "genStats.AverSsum =" << genStats.AverSsum;
    qDebug() << "genStats.AverVaver =" << genStats.AverVaver;
    qDebug() << "genStats.Aver_tInternal =" << genStats.Aver_tInternal;

    std::ofstream file;

    if (!IsContinueML)
        file.open(FirstReportFileName.toStdString(), std::ios::out | std::ios::app);
    else
        file.open(ContinueReportFileName.toStdString(), std::ios::out | std::ios::app);


    if (!file)
    {
        qDebug() << "FILE NOT OPEN";
        return;
    }


    file << GenStats.size()-1 << "\t"
         << t /*QTime::currentTime().toString().toStdString()*/ << "\t"
            << genStats.BestSsum  << "\t"
            << genStats.BestVaver << "\t"
            << genStats.Best_tInternal  << "\t"
            << genStats.AverSsum  << "\t"
            << genStats.AverVaver << "\t"
            << genStats.Aver_tInternal << std::endl;


    file.close();
}
//-------------------------------------------------------------

void RaceMap::CreateFirstRandomPopulation()
{
    FirstReportFileName = "FirstML_" + QDateTime::currentDateTime().toString("dd.MM.yyyy-HH.mm.ss") + ".txt";
    IsContinueML = false;
    ClearStatistics();
    CreateReportFile();

    ResetFirstCar();
    Car tempCar = Cars.front();

    Cars.clear();
    Cars.resize(PopulationSize, tempCar);

    for (auto & car : Cars)
        car.BrainAccess().InitRandomWeights();
}
//-------------------------------------------------------------

void RaceMap::CreateFirstPopulationBasedOnFirstCar()
{
    qDebug() << __PRETTY_FUNCTION__;
    ContinueReportFileName = "ContinueML_" + QDateTime::currentDateTime().toString("dd.MM.yyyy-HH.mm.ss") + ".txt";
    IsContinueML = true;
    ClearStatistics();
    CreateReportFile();

    Cars.front().BrainAccess().PrintMeAsDebugText();

    ResetFirstCar();

    Car tempCar = Cars.front();
    tempCar.CalcIntersects(Lines, true);

    Cars.clear();
    Cars.resize(PopulationSize, tempCar);

    MutateAllButFirst();

    Cars.front().MarkAsLastBest();
}
//-------------------------------------------------------------

void RaceMap::CreateNextGeneration()
{
    qDebug() << __PRETTY_FUNCTION__;
    AddStatisticsForCurGeneration();

    std::sort(Cars.begin(), Cars.end(), [](const Car &carA, const Car &carB){return carA.GetSsum() > carB.GetSsum();});


    if (IsCrossover)
    {
        //size_t countBefore = Cars.size();

        size_t countCrossPack = sqrt(Cars.size()-1);

        std::vector<Car> SortedBySsum(Cars.begin(), Cars.begin() + countCrossPack);
        std::vector<Car> SortedByLifeTime(Cars.begin(), Cars.end());

        std::sort(SortedByLifeTime.begin(), SortedByLifeTime.end(), [](const Car &carA, const Car &carB){return carA.Get_tInternal() > carB.Get_tInternal();});

        SortedByLifeTime.resize(countCrossPack);

        if (countCrossPack != SortedByLifeTime.size())
        {
            throw std::runtime_error("countCrossPack != SortedByLifeTime.size()");
        }

        size_t k = 1;
        for (size_t i = 0; i < countCrossPack; ++i)
        {
            size_t ratingA = countCrossPack - i - 1;

            for (size_t j = 0; j < countCrossPack; ++j)
            {
                size_t ratingB = countCrossPack - j - 1;

                Cars.at(k).BrainAccess().CreateByCrossover(SortedBySsum.at(i).BrainAccess(),
                                                           double(ratingA)/double(ratingA+ratingB),
                                                           SortedByLifeTime.at(j).BrainAccess());
                ++k;
            }
        }

        for (; k < Cars.size(); ++k)
        {
            Cars.at(k).BrainAccess() = Cars.front().BrainAccess();
        }

        qDebug() << "k =" << k;

        if (k != Cars.size())
        {
            throw std::runtime_error("k != Cars.size()");
        }
    }
    else
    {
        for (size_t i = 0; i < 9; ++i)
            std::copy(Cars.begin(), Cars.begin()+Cars.size()/10, Cars.begin()+Cars.size()/10*(i+1));

        constexpr double p = 0.034;
        for (size_t i = 1; i < Cars.size(); ++i)
            if ( i%(int(1.0/p)) == 0 )
                Cars[i] = Cars.front();
    }


    for (auto & car : Cars)
    {
        car.Reset(StartPoint, StartVector);
        car.CalcIntersects(Lines, true);
    }

    MutateAllButFirst();

    std::random_shuffle(Cars.begin()+1, Cars.end());

    Cars.front().MarkAsLastBest();
}
//-------------------------------------------------------------

void RaceMap::FindBestAndMark()
{
//    auto it = std::max_element(Cars.begin(), Cars.end(),
//         [](const Car & carA, const Car & carB)
//            {
//                return (carA.GetIsAlive() && carB.GetIsAlive() && (carA.GetSsum() < carB.GetSsum()));
//            });


//    if (Cars[0].IsLastBest == false)
//        qDebug() << "(Cars[0].IsLastBest == false)";

    int iBest = -1;
    double MaxS = -1;
    for (size_t i = 0; i < Cars.size(); ++i)
    {
        if (!Cars[i].GetIsLastBest())
            Cars[i].MarkAsPlain();

        if (Cars[i].GetIsAlive())
        {

            if (Cars[i].GetSsum() > MaxS)
            {
                MaxS = Cars.at(i).GetSsum();
                iBest = i;
            }
        }
    }
    if (iBest >= 0)
    {
        if ( !Cars[iBest].GetIsLastBest() )
        {
            Cars[iBest].MarkAsCurBest();
            std::swap(Cars[iBest], Cars[0]);
        }
    }
}
//-------------------------------------------------------------

void RaceMap::MutateAllButFirst()
{
    for (size_t i = 1; i < Cars.size(); ++i)
    {
        constexpr double p = 0.05;
        double rate = i/double(Cars.size());
        rate = rate*rate;

        Cars[i].BrainAccess().MutateWholeNet(0.05+rate*0.2,  // brain percent
                                             0.05+rate*0.4,  // weights percent
                                             0.003 + rate*MutationCoeff, // max change for weight
                                             0.001 + rate*MutationCoeff);// max change for bias;

        if ( /*true &&*/ i%(int(1.0/p)) == 0 )
            Cars[i].BrainAccess().MutateWholeNet(0.2,  // brain percent
                                                 0.8,  // weights percent
                                                 0.2,  // max change for weight
                                                 0.2); // max change for bias;
    }
}
//-------------------------------------------------------------

void RaceMap::CreateReportFile()
{
    std::ofstream file;
    if (!IsContinueML)
        file.open(FirstReportFileName.toStdString().c_str());
    else
        file.open(ContinueReportFileName.toStdString().c_str());


    if (!file)
    {
        qDebug() << "FILE NOT OPEN";
        return;
    }

    file << "Generation" << "\t"
         << "Time" << "\t"
            << "BestSsum"  << "\t"
            << "BestVaver" << "\t"
            << "BestLifeTime" << "\t"
            << "AverSsum"  << "\t"
            << "AverLifeTime" << std::endl;


    file.close();
}
//-------------------------------------------------------------

void RaceMap::AddNewLine()
{
    Lines.emplace_back(std::vector<QLineF>());
}
//-------------------------------------------------------------

void RaceMap::SetStartPoint(double x, double y)
{
    StartPoint = {x, y};
    Cars.front().SetPos(StartPoint);
}
//-------------------------------------------------------------

void RaceMap::SetStartVector(const QVector2D &_startVector)
{
    StartVector = _startVector;
    Cars.front().SetStartV(StartVector);
}
//-------------------------------------------------------------

void RaceMap::SetStartVectorByEndPos(const QPointF &_posTo)
{
    if (_posTo == StartPoint)
        StartVector = {1, 0};
    else
    {
        StartVector = {float(_posTo.x() - StartPoint.x()), float(_posTo.y() - StartPoint.y())};
        StartVector.normalize();
    }
    Cars.front().SetStartV(StartVector);
}
//-------------------------------------------------------------

void RaceMap::AddNewPointToLine(double x, double y)
{
    if (!Lines.back().empty())
    {
        Lines.back().back().setP2({x, y});
    }
    Lines.back().emplace_back(x, y, x, y);
}
//-------------------------------------------------------------

QJsonObject RaceMap::RepresentAsJsonObject() const
{
    QJsonObject resultObject;

    QJsonObject startPointObject;
    startPointObject.insert("x", StartPoint.x());
    startPointObject.insert("y", StartPoint.y());
    resultObject.insert("StartPoint", startPointObject);

    QJsonObject startVectorObject;
    startVectorObject.insert("x", StartVector.x());
    startVectorObject.insert("y", StartVector.y());
    resultObject.insert("StartVector", startVectorObject);

    resultObject.insert("PopulationSize", (int)PopulationSize);
    resultObject.insert("MutationCoeff", MutationCoeff);
    resultObject.insert("IsCrossover", IsCrossover);

    resultObject.insert("BrainSettings", BrainSettings.RepresentAsJsonObject());

    resultObject.insert("Car", Cars.front().RepresentAsJsonObject());

    QJsonArray linesArray;
    for (const auto & line : Lines)
    {
        QJsonArray lArray;
        for (const auto & l : line)
        {
            QJsonObject lObject;
            lObject.insert("x1", l.x1());
            lObject.insert("y1", l.y1());
            lObject.insert("x2", l.x2());
            lObject.insert("y2", l.y2());
            lArray.append(lObject);
        }
        linesArray.append(lArray);
    }
    resultObject.insert("Lines", linesArray);

    return resultObject;
}
//-------------------------------------------------------------

void RaceMap::LoadFromJsonObject(const QJsonObject _jsonObject)
{
    const QJsonObject &startPointObject = _jsonObject["StartPoint"].toObject();
    StartPoint.setX(startPointObject["x"].toDouble(0));
    StartPoint.setY(startPointObject["y"].toDouble(0));

    const QJsonObject &startVectorObject = _jsonObject["StartVector"].toObject();
    StartVector.setX(startVectorObject["x"].toDouble(1));
    StartVector.setY(startVectorObject["y"].toDouble(0));


    Cars.front().LoadFromJsonObject(_jsonObject["Car"].toObject());
    Cars.front().SetPos(StartPoint);
    Cars.front().SetStartV(StartVector);

    PopulationSize = _jsonObject["PopulationSize"].toInt(100);
    MutationCoeff = _jsonObject["MutationCoeff"].toDouble(0.7);
    IsCrossover = _jsonObject["IsCrossover"].toBool(false);

    BrainSettings.LoadFromJsonObject(_jsonObject["BrainSettings"].toObject());
    Cars.front().BrainAccess().InitTopology(BrainSettings.Topology,
                                            BrainSettings.ActivationFuncGeneral,
                                            BrainSettings.ActivationFuncFinal,
                                            BrainSettings.IsXavier);

    double x1, y1, x2, y2;
    const QJsonArray &linesArray = _jsonObject["Lines"].toArray();
    for (auto it = linesArray.begin(); it != linesArray.end(); ++it)
    {
        const QJsonArray &lArray = it->toArray();
        Lines.emplace_back(std::vector<QLineF>());
        for (auto itL = lArray.begin(); itL != lArray.end(); ++itL)
        {
            const QJsonObject lObject = itL->toObject();
            x1 = lObject["x1"].toDouble();
            y1 = lObject["y1"].toDouble();
            x2 = lObject["x2"].toDouble();
            y2 = lObject["y2"].toDouble();
            Lines.back().emplace_back(x1, y1, x2, y2);
        }
    }

}
//-------------------------------------------------------------

void RaceMap::DrawMeIn3D() const
{   
    if (!IsHideLines)
    {
        for (const auto & line : Lines)
        {
            for (const auto & l : line)
            {
                glLineWidth(1.0f);
                glColor3f(0.2f, 0.2f, 0.8f);
                glBegin(GL_LINES);
                    glVertex3f(l.x1(), l.y1(), zOffset);
                    glVertex3f(l.x2(), l.y2(), zOffset);
                glEnd();
            }
        }
    }

    Car::IsDrawEvenDead = true;
    size_t countAlive = 0;
    for (size_t i = 0; i < Cars.size(); ++i)
    {
        if (i == 0 || (countAlive < 100 && Cars[i].GetIsAlive())
                   || Cars[i].GetIsLastBest() || Cars[i].GetIsCurBest())
        {
            Cars.at(i).DrawMeIn3D();
            countAlive++;
            Car::IsDrawEvenDead = false;
        }
    }

//    for (const auto & car : Cars)
//        if (car.GetIsLastBest())
//            car.DrawMeIn3D();
}
//-------------------------------------------------------------

void RaceMap::CalcIntersectsForFirstCar()
{ 
    Cars.front().CalcIntersects(Lines, false);
}
//-------------------------------------------------------------

void RaceMap::CalcIntersectsForTwoCars()
{
    Cars.front().CalcIntersects(Lines, false);
    Cars.back().CalcIntersects(Lines, false);
}
//-------------------------------------------------------------

void RaceMap::NextStepML(double dt, MyThreadPool &_myThreadPool, size_t _threadCount)
{
    auto tStart = ClockType::now();
    std::atomic_bool isSomeoneAlive = false;

    const size_t K = _threadCount;

    if (_threadCount > 1)
    {
        _myThreadPool.SetCountTasksToDo(K);

        for (size_t k = 0; k < K; ++k)
        {
            _myThreadPool.EmplaceTask(
                [this, &isSomeoneAlive, dt]
                    (size_t s, size_t e)
                    {
                        for (size_t j = s; j < e; ++j)
                        {
                            if (Cars[j].GetIsAlive())
                            {
                                isSomeoneAlive = true;
                                Cars[j].GoNeuro(dt);
                            }

                            if (Cars[j].GetIsAlive())
                                Cars[j].CalcIntersects(Lines, true);

                        }
                    },
                    (k*Cars.size())/K,
                    ((k+1)*Cars.size())/K);
        }

        _myThreadPool.NotifyToDoAllTasks();
        _myThreadPool.WaitForAllTasksCompleted();
    }
    else
    {
        for (auto & car : Cars)
        {
            if (car.GetIsAlive())
            {
                isSomeoneAlive = true;
                car.GoNeuro(dt);
            }
            if (car.GetIsAlive())
                car.CalcIntersects(Lines, true);
        }
    }

//    if (ThreadCount > 1)
//    {
//        std::vector<std::future<void>> tasks(ThreadCount);

//        std::vector<size_t> inds(Cars.size());
//        for (size_t i = 0; i < inds.size(); ++i)
//            inds[i] = i;
//        std::random_shuffle(inds.begin(), inds.end());

//        for (size_t k = 0; k < ThreadCount; ++k)
//        {
//            tasks[k] = std::async(std::launch::async,
//                                  [this, &inds, &isSomeoneAlive, dt]
//                                  (size_t s, size_t e)
//                {

//                        for (size_t j = s; j < e; ++j)
//                        {
//                            if (Cars[inds[j]].GetIsAlive())
//                            {
//                                isSomeoneAlive = true;
//                                Cars[inds[j]].GoNeuro(dt);
//                            }

//                            if (Cars[inds[j]].GetIsAlive())
//                                Cars[inds[j]].CalcIntersects(Lines, true);
//                        }
//                },
//                k*Cars.size()/ThreadCount,
//                (k+1)*Cars.size()/ThreadCount);
//        }

//        for (auto & t : tasks)
//            t.get();

//    }
//    else
//    {
//        for (auto & car : Cars)
//        {
//            if (car.GetIsAlive())
//            {
//                isSomeoneAlive = true;
//                car.GoNeuro(dt);
//                car.CalcIntersects(Lines, true);
//            }
//        }
//    }

    auto tEnd = ClockType::now();
    auto t = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tEnd - tStart).count();
    t /= 1.0e6;
    qDebug() << "Go neuro and searching intersects (for all) (mili) =" << t;

    if (!isSomeoneAlive)
        CreateNextGeneration();
    else
        FindBestAndMark();
}
//-------------------------------------------------------------

bool RaceMap::LoadBrainFromFile(const QString &_fileName)
{
    bool res = Cars.front().BrainAccess().LoadFromFile(_fileName);
    if (res)
        BrainSettings = Cars.front().BrainAccess().GetBrainSettings();
    return res;
}
//-------------------------------------------------------------

void RaceMap::SaveLastBestBrainToFile(const QString &_fileName)
{
    GetLastBestCar().GetBrain().SaveToFile(_fileName);
}
//-------------------------------------------------------------

const Car & RaceMap::GetCurBestCar() const
{
    if (Cars.size() == 1)
        return Cars.front();
    else
    {
        for (const auto & car : Cars)
            if (car.GetIsCurBest())
                return car;

        return Cars.front();
    }
}
//-------------------------------------------------------------

const Car & RaceMap::GetLastBestCar() const
{
    if (Cars.size() == 1)
        return Cars.front();
    else
    {
        for (const auto & car : Cars)
            if (car.GetIsLastBest())
                return car;

        return Cars.front();
    }
}
//-------------------------------------------------------------


