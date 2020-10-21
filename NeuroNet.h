#ifndef NEURONET_H
#define NEURONET_H

#include <vector>
#include <functional>
#include <random>
#include <QJsonObject>

#include "Neuron.h"

struct BrainSettingsType
{
    std::vector<size_t> Topology;
    ActivationFunc_t ActivationFuncGeneral = nullptr;
    ActivationFunc_t ActivationFuncFinal = nullptr;
    bool IsXavier = true;
    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject &_jsonObject);

    bool operator!=(const BrainSettingsType &ob) const
    {
        return (Topology != ob.Topology ||
                *ActivationFuncGeneral.target<double(*)(double)>() != *ob.ActivationFuncGeneral.target<double(*)(double)>() ||
                *ActivationFuncFinal.target<double(*)(double)>()   != *ob.ActivationFuncFinal.target<double(*)(double)>() ||
                IsXavier != ob.IsXavier);
    }
};



class NeuroNet
{
private:

    size_t GeneCount = 0;
    std::vector<LayerStruct> Layers;

    BrainSettingsType BrainSettings;

    void ParseJsonObject(const QJsonObject _jsonObject);

    static std::mt19937 gen;

public:

    static void InitRandGenByTime();

    NeuroNet() = default;
    NeuroNet(const std::vector<size_t> _topology,
             ActivationFunc_t _generalActivationFunc,
             ActivationFunc_t _finalActivationFunc, bool _isXavier);

    void InitTopology(const std::vector<size_t> _topology,
                      ActivationFunc_t _generalActivationFunc,
                      ActivationFunc_t _finalActivationFunc, bool _isXavier);

    void InitRandomWeights();

    void SetInput(size_t _ind, double _value);
    void CalcAll();
    double GetOutput(size_t _ind) const;

    void MutateOneRandomGene(double _weightPercent, double _maxWeightChange, double _maxBiasChange);
    void MutateWholeNet(double _netPercent, double _amountWeightProb, double _maxWeightChange, double _maxBiasChange);

    void PrintMeAsDebugText() const;

    size_t GetInputCount()  const {return Layers.at(0).Neurons.size();}
    size_t GetOutputCount() const {return Layers.at(Layers.size()-1).Neurons.size();}

    size_t GetGeneCount() const {return GeneCount;}

    const BrainSettingsType & GetBrainSettings() const {return BrainSettings;}

    const auto & GetActivationGeneral() const {return BrainSettings.ActivationFuncGeneral;}
    const auto & GetActivationFinal() const   {return BrainSettings.ActivationFuncFinal;}

    void SaveToFile(const QString & _fileName) const;
    bool LoadFromFile(const QString &_fileName);

    void CreateByCrossover(const NeuroNet &_A, double rateAtoB, const NeuroNet &_B);
};

std::string ActivationFuncAsText(const ActivationFunc_t &_func);
ActivationFunc_t GetActivationFuncByText(const std::string &_funcName);

#endif // NEURONET_H
