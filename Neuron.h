#ifndef NEURON_H
#define NEURON_H

#include <vector>
#include <functional>
#include <random>
#include <QJsonObject>


//using ActivationFunc_t = double(*)(double); // Тестировать будет ли работать быстрее
using ActivationFunc_t = std::function<double(double)>;

double MySigmoidFunc(double x);
double MyReLU(double x);

class Neuron;

struct LayerStruct
{
    std::vector<Neuron> Neurons;
//    std::vector<double> Biases;

    ActivationFunc_t ActivationFunc = nullptr;
    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject &_jsonObject);
};


class Neuron
{
private:
    size_t Index;
    double Value;
    std::vector<double> Weights;
    double InputBias = 0;

    static std::mt19937 gen;

public:

    static void InitRandGenByTime();

    auto & WeightsAccess() {return Weights;}
    const auto & GetWeights() const {return Weights;}

    Neuron() = delete;
    Neuron(size_t _index, size_t _outputCount);

    void SetValue(double _value) {Value = _value;}
    double GetValue() const {return Value;}

    void InitRandomWeights(size_t _fan_in, size_t _fan_out, bool _isXavier);

    void Mutate(double _weightPercent, double _maxWeightChange, double _maxDeltaBias);
//    void MutateInputBias(double _maxDegree);

    void Feed(const LayerStruct &_prevLayer, const ActivationFunc_t &_activationFunc);

    void PrintMeAsDebugText() const;

    QJsonObject RepresentAsJsonObject() const;
    void LoadFromJsonObject(const QJsonObject &_jsonObject);

    void SetInputBias(double _inputBias) {InputBias = _inputBias;}
    double GetInputBias() const {return InputBias;}
};
//-------------------------------------------------------------

#endif // NEURON_H
