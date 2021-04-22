#include "NeuroNet.h"

#include <QDebug>
#include <QJsonDocument>
#include <QFile>
#include <QJsonArray>


std::mt19937 NeuroNet::gen;

/*static*/ void NeuroNet::InitRandGenByTime()
{
    gen.seed(time(nullptr));
}
//-------------------------------------------------------------

NeuroNet::NeuroNet(const std::vector<size_t> _topology,
                   ActivationFunc_t _generalActivationFunc,
                   ActivationFunc_t _finalActivationFunc, bool _isXavier)
{
    InitTopology(_topology, _generalActivationFunc, _finalActivationFunc, _isXavier);
    InitRandomWeights();
}
//-------------------------------------------------------------

void NeuroNet::InitTopology(const std::vector<size_t> _topology,
                            ActivationFunc_t _generalActivationFunc,
                            ActivationFunc_t _finalActivationFunc,
                            bool _isXavier)
{
    BrainSettings.IsXavier = _isXavier;
    BrainSettings.Topology = _topology;
    GeneCount = 0;
    BrainSettings.ActivationFuncGeneral = _generalActivationFunc;
    BrainSettings.ActivationFuncFinal = _finalActivationFunc;

    Layers.clear();

    for (size_t i = 0; i < _topology.size(); ++i)
    {
        Layers.emplace_back(LayerStruct());
        if (i > 0)
            Layers.back().ActivationFunc = (i != _topology.size()-1) ? _generalActivationFunc : _finalActivationFunc;

        size_t outputsCount = (i != _topology.size()-1) ? _topology[i+1] : 0;
        for (size_t j = 0; j < _topology[i]; ++j)
        {
            Layers.back().Neurons.emplace_back(j, outputsCount);
            if (outputsCount)
            {
                GeneCount++;
            }
        }
    }

    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << "GeneCount =" << GeneCount;
}
//-------------------------------------------------------------

void NeuroNet::InitRandomWeights()
{
    for (size_t i = 0; i < Layers.size()-1; ++i)
    {
        for (auto & neuron : Layers[i].Neurons)
            neuron.InitRandomWeights(Layers[i].Neurons.size(), Layers[i+1].Neurons.size(), BrainSettings.IsXavier);
    }
}
//-------------------------------------------------------------

void NeuroNet::SetInput(size_t _ind, double _value)
{
    Layers.front().Neurons.at(_ind).SetValue(_value);
}
//-------------------------------------------------------------

void NeuroNet::CalcAll()
{
    for (size_t i = 1; i < Layers.size(); ++i)
    {
        for (auto & neuron : Layers[i].Neurons)
            neuron.Feed(Layers[i-1], Layers[i].ActivationFunc);
    }
}
//-------------------------------------------------------------

double NeuroNet::GetOutput(size_t _ind) const
{
    return Layers.back().Neurons.at(_ind).GetValue();
}
//-------------------------------------------------------------

void NeuroNet::MutateOneRandomGene(double _weightPercent, double _maxWeightChange, double _maxDeltaBias)
{
    size_t geneInd = rand()%GeneCount;
    for (auto & layer : Layers)
    {
        if (geneInd < layer.Neurons.size())
        {
            layer.Neurons.at(geneInd).Mutate(_weightPercent, _maxWeightChange, _maxDeltaBias);
            return;
        }
        else
        {
            geneInd -= layer.Neurons.size();
        }
    }
}
//-------------------------------------------------------------

void NeuroNet::MutateWholeNet(double _netPercent, double _amountWeightProb, double _maxWeightChange, double _maxDeltaBias)
{
    for (size_t i = 0; i < Layers.size()-1; ++i)
    {
        for (auto & neuron : Layers[i].Neurons)
            if (rand()/double(RAND_MAX) < _netPercent)
                neuron.Mutate(_amountWeightProb, _maxWeightChange, _maxDeltaBias);
    }
}
//-------------------------------------------------------------

void NeuroNet::PrintMeAsDebugText() const
{
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << "GeneCount =" << GeneCount;
    qDebug() << "IsXavier =" << BrainSettings.IsXavier;
    qDebug() << "BasicActivationFunc:" << QString().fromStdString(ActivationFuncAsText(BrainSettings.ActivationFuncGeneral));
    qDebug() << "FinalActivationFunc:" << QString().fromStdString(ActivationFuncAsText(BrainSettings.ActivationFuncFinal));
    qDebug() << "Topology:";    
    for (size_t i = 0; i < Layers.size(); ++i)
    {
        qDebug() << "Layer" << i << ".Neurons.size() ="<< Layers[i].Neurons.size();
        for (size_t j = 0; j < Layers[i].Neurons.size(); ++j)
        {
            qDebug() << "neuron" << j << "." << Layers[i].Neurons[j].GetValue();
            Layers[i].Neurons[j].PrintMeAsDebugText();
        }
    }
}
//-------------------------------------------------------------

void NeuroNet::SaveToFile(const QString & _fileName) const
{
    QJsonDocument jsonDoc;
    QJsonObject resultObject;
    resultObject.insert("BrainSettings", QJsonValue(BrainSettings.RepresentAsJsonObject()));
    resultObject.insert("Layers.size", (int)Layers.size());

    QJsonArray layersArray;
    for (const auto & l : Layers)
    {
        layersArray.append(QJsonValue(l.RepresentAsJsonObject()));
    }

    resultObject.insert("Layers", layersArray);

    jsonDoc.setObject(resultObject);

    QFile jsonFile(_fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(jsonDoc.toJson());
}
//-------------------------------------------------------------

bool NeuroNet::LoadFromFile(const QString &_fileName)
{
    QFile json(_fileName);
    if (json.open(QIODevice::ReadOnly))
    {
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json.readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError)
        {
            if (jsonDoc.isObject())
            {
                ParseJsonObject(jsonDoc.object());
                PrintMeAsDebugText();
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

    return true;
}
//-------------------------------------------------------------

void NeuroNet::ParseJsonObject(const QJsonObject &_jsonObject)
{
    BrainSettings.LoadFromJsonObject(_jsonObject["BrainSettings"].toObject());
    InitTopology(BrainSettings.Topology, BrainSettings.ActivationFuncGeneral, BrainSettings.ActivationFuncFinal, BrainSettings.IsXavier);

    if ((int)Layers.size() != _jsonObject["Layers.size"].toInt())
        throw std::runtime_error("(int)Layers.size() != _jsonObject[\"Layers.size\"].toInt() in NeuroNet::LoadFromJsonObject");

    size_t i = 0;
    const QJsonArray &layersArray = _jsonObject["Layers"].toArray();
    for (auto it = layersArray.begin(); it != layersArray.end(); ++it, ++i)
    {
        Layers.at(i).LoadFromJsonObject( it->toObject() );
    }
    if (Layers.size() != i)
        throw std::runtime_error("Layers.size() != i in NeuroNet::LoadFromJsonObject");
}
//-------------------------------------------------------------

QJsonObject BrainSettingsType::RepresentAsJsonObject() const
{
    QJsonObject brainSettingsObject;
    QJsonArray topologyArray;
    for (const auto t : Topology)
        topologyArray.append((int)t);
    brainSettingsObject.insert("Topology", topologyArray);
    brainSettingsObject.insert("IsXavier", IsXavier);
    brainSettingsObject.insert("ActivationFuncGeneral", QString().fromStdString(ActivationFuncAsText(ActivationFuncGeneral)));
    brainSettingsObject.insert("ActivationFuncFinal", QString().fromStdString(ActivationFuncAsText(ActivationFuncFinal)));
    return brainSettingsObject;
}
//-------------------------------------------------------------

void BrainSettingsType::LoadFromJsonObject(const QJsonObject &_jsonObject)
{
    Topology.clear();
    const QJsonArray &topologyArray = _jsonObject["Topology"].toArray();
    for (auto it = topologyArray.begin(); it != topologyArray.end(); ++it)
    {
         Topology.emplace_back(it->toInt());
    }
    IsXavier = _jsonObject["IsXavier"].toBool(true);
    ActivationFuncGeneral = GetActivationFuncByText(_jsonObject["ActivationFuncGeneral"].toString().toStdString());
    ActivationFuncFinal = GetActivationFuncByText(_jsonObject["ActivationFuncFinal"].toString().toStdString());

}
//-------------------------------------------------------------

void NeuroNet::CreateByCrossover(const NeuroNet &_A, double rateAtoB, const NeuroNet &_B)
{
    std::uniform_real_distribution<> distribution(0.0, 1.0);

    for (size_t i = 0; i < Layers.size(); ++i)
    {
        for (size_t j = 0; j < Layers[i].Neurons.size(); ++j)
        {
            for (size_t k = 0; k < Layers[i].Neurons[j].WeightsAccess().size(); ++k)
            {
                if (distribution(gen) < rateAtoB)
                    Layers[i].Neurons[j].WeightsAccess()[k] = _A.Layers[i].Neurons[j].GetWeights()[k];
                else
                    Layers[i].Neurons[j].WeightsAccess()[k] = _B.Layers[i].Neurons[j].GetWeights()[k];
            }

            if (distribution(gen) < rateAtoB)
                Layers[i].Neurons[j].SetInputBias( _A.Layers[i].Neurons[j].GetInputBias() );
            else
                Layers[i].Neurons[j].SetInputBias( _B.Layers[i].Neurons[j].GetInputBias() );
        }
    }
}
//-------------------------------------------------------------

std::string ActivationFuncAsText(const ActivationFunc_t &_func)
{
    qDebug() << "sizeof(_func)" << sizeof(_func);

    auto ptr = _func.target<double(*)(double)>();
    if (ptr && *ptr == MySigmoidFunc)
        return "MySigmoidFunc";
    else if (ptr && *ptr == tanh)
        return "tanh";
    else if (ptr && *ptr == MyReLU)
        return "MyReLU";
    else
        return "UNKNOWN";
}
//-------------------------------------------------------------

ActivationFunc_t GetActivationFuncByText(const std::string &_funcName)
{
    if (_funcName == "MySigmoidFunc")
        return MySigmoidFunc;
    else if (_funcName == "tanh")
        return tanh;
    else if (_funcName == "MyReLU")
        return MyReLU;
    else
        return nullptr; // std::function<double(double)>();
}
//-------------------------------------------------------------


