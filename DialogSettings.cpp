#include "DialogSettings.h"
#include "ui_DialogSettings.h"

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
}
//-------------------------------------------------------------

DialogSettings::~DialogSettings()
{
    delete ui;
}
//-------------------------------------------------------------

void DialogSettings::InitDialog(const MainModel &_model)
{
    ui->lineEditPopulationSize->setText(QString().setNum(_model.Race.PopulationSize));
    ui->lineEditMutationCoeff->setText(QString().setNum(_model.Race.MutationCoeff));

    ui->cbThreadCount->setCurrentIndex( ui->cbThreadCount->findText( QString().setNum(_model.ThreadCount) ));

    ui->chbIsAdvanced->setChecked( _model.Race.Cars.front().GetIsAdvanced() );
    ui->chbIsCrossover->setChecked( _model.Race.IsCrossover );

    QString strTopology;
    for (size_t i = 0; i < _model.Race.BrainSettings.Topology.size(); ++i)
    {
        strTopology += QString().setNum(_model.Race.BrainSettings.Topology[i]);
        if (i < _model.Race.BrainSettings.Topology.size() - 1)
            strTopology += "-";
    }
    ui->lineEditNetTopology->setText(strTopology);

    if (_model.Race.BrainSettings.IsXavier)
        ui->cbInitWeights->setCurrentIndex(0);
    else
        ui->cbInitWeights->setCurrentIndex(1);

    auto ptr = _model.Race.BrainSettings.ActivationFuncGeneral.target<double(*)(double)>();
    if (ptr && *ptr == MySigmoidFunc)
        ui->cbActivationGeneral->setCurrentIndex(0);
    else if (ptr && *ptr == tanh)
        ui->cbActivationGeneral->setCurrentIndex(1);
    else if (ptr && *ptr == MyReLU)
        ui->cbActivationGeneral->setCurrentIndex(2);
    else
        throw std::runtime_error("Unknown General Activation Function");

    ptr = _model.Race.BrainSettings.ActivationFuncFinal.target<double(*)(double)>();
    if (ptr && *ptr == MySigmoidFunc)
        ui->cbActivationFinal->setCurrentIndex(0);
    else if (ptr && *ptr == tanh)
        ui->cbActivationFinal->setCurrentIndex(1);
    else if (ptr && *ptr == MyReLU)
        ui->cbActivationFinal->setCurrentIndex(2);
    else
        throw std::runtime_error("Unknown Final Activation Function");

    ui->lineEditMax_tInternal->setText(QString().setNum(Car::MaxLifeTime));
    ui->lineEditMinMAVelocity->setText(QString().setNum(Car::MinMAVelocity));
    ui->lineEditMarginLevel->setText(QString().setNum(Car::MarginLevel));

    ui->lineEditDetectorsLenghts->setText( _model.Race.Cars.front().GetDetectorsLengths() );
    ui->lineEditDetectorsAngles->setText( _model.Race.Cars.front().GetDetectorsAngles() );
}
//-------------------------------------------------------------

void DialogSettings::ReInitModel(MainModel &_model)
{
    _model.Race.PopulationSize = ui->lineEditPopulationSize->text().toInt();
    _model.Race.MutationCoeff = ui->lineEditMutationCoeff->text().toDouble();

    _model.ThreadCount = ui->cbThreadCount->currentText().toInt();
    _model.Race.IsCrossover = ui->chbIsCrossover->isChecked();

    _model.Race.Cars.front().SetIsAdvanced(ui->chbIsAdvanced->isChecked());
    _model.Race.Cars.front().CreateDetectors( ui->lineEditDetectorsLenghts->text(),
                                              ui->lineEditDetectorsAngles->text());


    BrainSettingsType brainSettings;
    QString strTopology = ui->lineEditNetTopology->text();
    int pos;
    do
    {
        pos = strTopology.indexOf('-');
        if (pos > 0)
        {
            brainSettings.Topology.emplace_back(strTopology.left(pos).toInt());
            strTopology = strTopology.right(strTopology.size()-pos-1);
        }
        else
            brainSettings.Topology.emplace_back(strTopology.toInt());
    } while (pos >= 0);

    if (_model.Race.Cars.front().GetIsAdvanced())
        brainSettings.Topology.front() = _model.Race.Cars.front().GetDetectorsCount()+2;
    else
        brainSettings.Topology.front() = _model.Race.Cars.front().GetDetectorsCount();


    if (ui->cbInitWeights->currentIndex() == 0)
        brainSettings.IsXavier = true;
    else if (ui->cbInitWeights->currentIndex() == 1)
        brainSettings.IsXavier = false;
    else
        throw std::runtime_error("Unknown Weights Init");

    if (ui->cbActivationGeneral->currentIndex() == 0)
        brainSettings.ActivationFuncGeneral = MySigmoidFunc;
    else if (ui->cbActivationGeneral->currentIndex() == 1)
        brainSettings.ActivationFuncGeneral = tanh;
    else if (ui->cbActivationGeneral->currentIndex() == 2)
        brainSettings.ActivationFuncGeneral = MyReLU;
    else
        throw std::runtime_error("Unknown General Activation Function");

    if (ui->cbActivationFinal->currentIndex() == 0)
        brainSettings.ActivationFuncFinal = MySigmoidFunc;
    else if (ui->cbActivationFinal->currentIndex() == 1)
        brainSettings.ActivationFuncFinal = tanh;
    else if (ui->cbActivationFinal->currentIndex() == 2)
        brainSettings.ActivationFuncFinal = MyReLU;
    else
        throw std::runtime_error("Unknown Final Activation Function");

    _model.Race.BrainSettings = brainSettings;

    Car::MaxLifeTime = ui->lineEditMax_tInternal->text().toInt();
    Car::MinMAVelocity = ui->lineEditMinMAVelocity->text().toDouble();
    Car::MarginLevel = ui->lineEditMarginLevel->text().toDouble();



    if (_model.Race.Cars.front().BrainAccess().GetBrainSettings() != brainSettings)
//    if (_model.Race.Cars.front().BrainAccess().GetBrainSettings().RepresentAsJsonObject() != brainSettings.RepresentAsJsonObject())
    {
        _model.Race.Cars.front().BrainAccess().InitTopology(brainSettings.Topology,
                                                            brainSettings.ActivationFuncGeneral,
                                                            brainSettings.ActivationFuncFinal, brainSettings.IsXavier);
    }

    _model.Race.Cars.front().CalcCarRectAndDetectors();
    _model.Race.CalcIntersectsForFirstCar();
}
//-------------------------------------------------------------











