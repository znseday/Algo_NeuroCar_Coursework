#include "Neuron.h"

#include <random>
#include <cmath>
#include <ctime>

#include <QDebug>
#include <QJsonArray>

double MySigmoidFunc(double x)
{
    return 1.0/(1.0 + exp(-x));
}

double MyReLU(double x)
{
    return std::max(0.0, x);
}
//-------------------------------------------------------------
//-------------------------------------------------------------

std::mt19937 Neuron::gen;

/*static*/ void Neuron::InitRandGenByTime()
{
    gen.seed(time(nullptr));
}
//-------------------------------------------------------------
//-------------------------------------------------------------

QJsonObject LayerStruct::RepresentAsJsonObject() const
{
    QJsonObject resultObject;
    resultObject.insert("Neurons.size", (int)Neurons.size());

    QJsonArray neuronsArray;
    for (const auto & n : Neurons)
    {
        neuronsArray.append(QJsonValue(n.RepresentAsJsonObject()));
    }

    resultObject.insert("Neurons", neuronsArray);

    return resultObject;
}
//-------------------------------------------------------------

void LayerStruct::LoadFromJsonObject(const QJsonObject &_jsonObject)
{
    if ((int)Neurons.size() != _jsonObject["Neurons.size"].toInt(0))
        throw std::runtime_error("(int)Neurons.size() != _jsonObject[\"Neurons.size\"].toInt(0) in LayerStruct::LoadFromJsonObject");

    size_t i = 0;
    const QJsonArray &neuronArray = _jsonObject["Neurons"].toArray();
    for (auto it = neuronArray.begin(); it != neuronArray.end(); ++it, ++i)
    {
        Neurons.at(i).LoadFromJsonObject( it->toObject() );
    }
    if (Neurons.size() != i)
        throw std::runtime_error("Neurons.size() != i in LayerStruct::LoadFromJsonObject");
}
//-------------------------------------------------------------
//-------------------------------------------------------------

Neuron::Neuron(size_t _index, size_t _outputCount) : Index(_index)
{
    Weights.resize(_outputCount);
}
//-------------------------------------------------------------

void Neuron::InitRandomWeights(size_t _fan_in, size_t _fan_out, bool _isXavier)
{
    //qDebug() << __PRETTY_FUNCTION__;

    if (_isXavier)
    {
        const double x = sqrt(6.0/(_fan_in + _fan_out));
        std::uniform_real_distribution<> distribution(-x, x);

        for (auto & w : Weights)
        {
            w = distribution(gen);
            //qDebug() << "w =" << w;
        }
    }
    else // for ReLU
    {
        const double s = sqrt(2.0/(_fan_in));
        std::normal_distribution<> distribution(0, s);

        for (auto & w : Weights)
        {
            w = distribution(gen);
            //qDebug() << "w =" << w;
        }
    }
}
//-------------------------------------------------------------

void Neuron::Mutate(double _weightPercent, double _maxWeightChange, double _maxDeltaBias)
{
    //qDebug() << __PRETTY_FUNCTION__;
    std::uniform_real_distribution<> distribution(-_maxWeightChange, _maxWeightChange);
    for (auto & w : Weights)
    {
        if (rand()/double(RAND_MAX) < _weightPercent)
        {
            w += distribution(gen);
        }
    }

    double r = -_maxDeltaBias + rand()/double(RAND_MAX)*2.0*_maxDeltaBias;
    InputBias += r;
}
//-------------------------------------------------------------

//void Neuron::MutateInputBias(double _maxDeltaBias)
//{
//    double r = -_maxDegree + rand()/double(RAND_MAX)*2.0*_maxDegree;
//    InputBias += r;
//}
//-------------------------------------------------------------

void Neuron::Feed(const LayerStruct &_prevLayer, const ActivationFunc_t &_activationFunc)
{
    double sum = 0;
    for (const auto & neuron : _prevLayer.Neurons)
        sum += neuron.GetValue() * neuron.Weights[Index];

    Value = _activationFunc(sum + InputBias);
}
//-------------------------------------------------------------

void Neuron::PrintMeAsDebugText() const
{
    for (const auto & w : Weights)
    {
        qDebug() << "w =" << w;
    }
    qDebug() << "InputBias =" << InputBias;
}
//-------------------------------------------------------------

QJsonObject Neuron::RepresentAsJsonObject() const
{
    QJsonObject resultObject;
    resultObject.insert("Weights.size", (int)Weights.size());

    QJsonArray weightsArray;
    for (const auto & w : Weights)
    {
        weightsArray.append(w);
    }

    resultObject.insert("Weights", weightsArray);

    resultObject.insert("InputBias", InputBias);

    return resultObject;
}
//-------------------------------------------------------------

void Neuron::LoadFromJsonObject(const QJsonObject &_jsonObject)
{
    if ((int)Weights.size() != _jsonObject["Weights.size"].toInt())
        throw std::runtime_error("(int)Weights.size() != _jsonObject[\"Weights.size\"].toInt() in Neuron::LoadFromJsonObject");

    const QJsonArray &weightsArray = _jsonObject["Weights"].toArray();
    size_t i = 0;
    for (auto it = weightsArray.begin(); it != weightsArray.end(); ++it, ++i)
    {
        Weights.at(i) = it->toDouble(0);
    }
    if (Weights.size() != i)
        throw std::runtime_error("Weights.size() != i in Neuron::LoadFromJsonObject");

    InputBias = _jsonObject["InputBias"].toDouble(0);

    qDebug() << "InputBias = " << InputBias;
}
//-------------------------------------------------------------
