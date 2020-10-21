#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), Model(WorkMode)
{
    ui->setupUi(this);

    lblWorldX = new QLabel("n/a");
    lblWorldY = new QLabel("n/a");
    lblFps = new QLabel("n/a");
    lblCarSpeed = new QLabel("n/a");
    lblCar_tInternal = new QLabel("n/a");
    lblCarSsum = new QLabel("n/a");
    lblCarVaver = new QLabel("n/a");
    lblTimerStatus = new QLabel("n/a");
    lblCarAccell = new QLabel("n/a");
    lblCarWheelSpeed = new QLabel("n/a");

    lblWorldX->setFixedWidth(80);
    lblWorldY->setFixedWidth(80);
    lblFps->setFixedWidth(80);
    lblCarSpeed->setFixedWidth(80);
    lblCar_tInternal->setFixedWidth(80);
    lblCarSsum->setFixedWidth(80);
    lblCarVaver->setFixedWidth(100);
    lblTimerStatus->setFixedWidth(80);
    lblCarAccell->setFixedWidth(100);
    lblCarWheelSpeed->setFixedWidth(100);

    ui->statusbar->addWidget(lblWorldX);
    ui->statusbar->addWidget(lblWorldY);
    ui->statusbar->addWidget(lblFps);
    ui->statusbar->addWidget(lblCarSpeed);
    ui->statusbar->addWidget(lblCarSsum);
    ui->statusbar->addWidget(lblCar_tInternal);
    ui->statusbar->addWidget(lblCarVaver);
    ui->statusbar->addWidget(lblTimerStatus);
    ui->statusbar->addWidget(lblCarAccell);
    ui->statusbar->addWidget(lblCarWheelSpeed);

    this->setWindowTitle(QApplication::applicationName());

    wgtMainGL = new MainGLWidget(WorkMode, Model);

    connect(wgtMainGL, SIGNAL(SignalSendWorldCoords(double, double, double, bool)),
            this, SLOT(SlotReceiveWorldCoords(double, double, double, bool)));

    connect(&Model, SIGNAL(SignalSendPutStartFinished()),
            this, SLOT(SlotReceivePutStartFinished()));

    connect(wgtMainGL, SIGNAL(SignalSendCarInfo(double, double, int, double, double, double)),
            this, SLOT(SlotReceiveCarInfo(double, double, int, double, double, double)));

    connect(&Model, SIGNAL(SignalSendCarInfo(double, double, int, double, double, double)),
            this, SLOT(SlotReceiveCarInfo(double, double, int, double, double, double)));

    connect(wgtMainGL, SIGNAL(SignalSendTimerStatus(bool)),
            this, SLOT(SlotReceiveTimerStatus(bool)));

    connect(wgtMainGL, SIGNAL(SignalSendCurrentFps(int)),
            this,   SLOT(SlotReceiveCurrentFps(int)));

    setCentralWidget(wgtMainGL);

    dlgAbout.setWindowTitle("About");
    dlgSettings.setWindowTitle("Settings");

    wgtMainGL->setMouseTracking(true);
    wgtMainGL->setFocus();

    on_actionFile_New_triggered();
}
//-------------------------------------------------------------

MainWindow::~MainWindow()
{
    //WorkMode = WorkModeType::Nothing;
    delete ui;
}
//-------------------------------------------------------------

void MainWindow::AfterShow()
{
    // OnShow
}
//-------------------------------------------------------------

void MainWindow::SlotReceiveWorldCoords(double wx, double wy, [[maybe_unused]] double wz, bool wExists)
{
    lblWorldX->setText("X = " + (wExists?QString().setNum(wx):"n/a"));
    lblWorldY->setText("Y = " + (wExists?QString().setNum(wy):"n/a"));
}
//-------------------------------------------------------------

void MainWindow::SlotReceivePutStartFinished()
{
    WorkMode = WorkModeType::Nothing;
    ui->actionFile_Open->setEnabled(true);
    ui->actionFile_Save->setEnabled(true);
    ui->actionFile_Save_As->setEnabled(true);
    ui->actionEdit_Put_Start->setEnabled(true);
    ui->actionEdit_Start_Line->setEnabled(true);
    ui->actionEdit_End_Line->setEnabled(true);
    ui->actionRun_Manual->setEnabled(true);
    ui->actionRun_Neuro_Car->setEnabled(true);
    ui->actionRun_Run_ML->setEnabled(true);
}
//-------------------------------------------------------------

void MainWindow::SlotReceiveCurrentFps(int fps)
{
    lblFps->setText("fps = " + QString().setNum(fps));
}
//-------------------------------------------------------------

void MainWindow::SlotReceiveTimerStatus(bool isOn)
{
    lblTimerStatus->setText(QString("Timer is ") + (isOn?"On":"Off"));
}
//-------------------------------------------------------------

void MainWindow::SlotReceiveCarInfo(double v, double s, int t, double vaver, double accel, double wheelSpeed)
{
    lblCarSpeed->setText("V = " + QString().setNum(v));
    lblCarSsum->setText("S = " + QString().setNum(s));
    lblCar_tInternal->setText("t = " + QString().setNum(t));
    lblCarVaver->setText("Vaver = " + QString().setNum(vaver));
    lblCarAccell->setText("Accel = " + QString().setNum(accel));
    lblCarWheelSpeed->setText("Wspeed = " + QString().setNum(wheelSpeed));
}
//-------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    wgtMainGL->StopMainTimer();
    WorkMode = WorkModeType::Nothing;
    QMainWindow::closeEvent(event);
}
//-------------------------------------------------------------

void MainWindow::on_actionFile_New_triggered()
{
    wgtMainGL->StopMainTimer();

    FileName.clear();
    this->setWindowTitle(QApplication::applicationName() + " - New");

    WorkMode = WorkModeType::Nothing;
    Model.New();
    Model.OnResize(wgtMainGL->GetWidth(), wgtMainGL->GetHeight());

    ui->actionFile_Open->setEnabled(true);
    ui->actionFile_Save->setEnabled(true);
    ui->actionFile_Save_As->setEnabled(true);
    ui->actionEdit_Put_Start->setEnabled(true);
    ui->actionEdit_Start_Line->setEnabled(true);
    ui->actionEdit_End_Line->setEnabled(true);
    ui->actionRun_Manual->setEnabled(true);
    ui->actionRun_Neuro_Car->setEnabled(true);
    ui->actionRun_Run_ML->setEnabled(true);

    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionFile_Open_triggered()
{
    WorkMode = WorkModeType::Nothing;
    wgtMainGL->StopMainTimer();

    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Race file", ".", "Race Files (*.race)");

    if (fileName.isEmpty())
        return;

    FileName = fileName;
    this->setWindowTitle(QApplication::applicationName() + " - " + FileName);
    Model.LoadFromFile(fileName);
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionFile_Save_triggered()
{
    if (FileName.isEmpty())
        on_actionFile_Save_As_triggered();
    else
        Model.SaveToFile(FileName);
}
//-------------------------------------------------------------

void MainWindow::on_actionFile_Save_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Race file", ".", "Race Files (*.race)");

    if (fileName.isEmpty())
        return;

    FileName = fileName;

    Model.SaveToFile(FileName);
    this->setWindowTitle(QApplication::applicationName() + " - " + FileName);
}
//-------------------------------------------------------------

void MainWindow::on_actionFile_Exit_triggered()
{
    wgtMainGL->StopMainTimer();
    QApplication::closeAllWindows();
}
//-------------------------------------------------------------

void MainWindow::on_actionEdit_Start_Line_triggered()
{
    WorkMode = WorkModeType::EditLine;
    wgtMainGL->StopMainTimer();

    Model.AddNewLine();
    ui->actionFile_Open->setEnabled(false);
    ui->actionFile_Save->setEnabled(false);
    ui->actionFile_Save_As->setEnabled(false);
    ui->actionEdit_Put_Start->setEnabled(false);
    ui->actionEdit_Start_Line->setEnabled(false);
//    ui->actionEdit_End_Line->setEnabled(false);
    ui->actionRun_Manual->setEnabled(false);
    ui->actionRun_Neuro_Car->setEnabled(false);
    ui->actionRun_Run_ML->setEnabled(false);

}
//-------------------------------------------------------------

void MainWindow::on_actionEdit_End_Line_triggered()
{
    WorkMode = WorkModeType::Nothing;
    wgtMainGL->StopMainTimer();

    ui->actionFile_Open->setEnabled(true);
    ui->actionFile_Save->setEnabled(true);
    ui->actionFile_Save_As->setEnabled(true);
    ui->actionEdit_Put_Start->setEnabled(true);
    ui->actionEdit_Start_Line->setEnabled(true);
    ui->actionEdit_End_Line->setEnabled(true);
    ui->actionRun_Manual->setEnabled(true);
    ui->actionRun_Neuro_Car->setEnabled(true);
    ui->actionRun_Run_ML->setEnabled(true);
}
//-------------------------------------------------------------

void MainWindow::on_actionEdit_Put_Start_triggered()
{
    WorkMode = WorkModeType::EditStart;
    wgtMainGL->StopMainTimer();

    ui->actionFile_Open->setEnabled(false);
    ui->actionFile_Save->setEnabled(false);
    ui->actionFile_Save_As->setEnabled(false);
    ui->actionEdit_Put_Start->setEnabled(false);
    ui->actionEdit_Start_Line->setEnabled(false);
    ui->actionEdit_End_Line->setEnabled(false);
    ui->actionRun_Manual->setEnabled(false);
    ui->actionRun_Neuro_Car->setEnabled(false);
    ui->actionRun_Run_ML->setEnabled(false);
}
//-------------------------------------------------------------

void MainWindow::on_actionTools_Settings_triggered()
{
    wgtMainGL->StopMainTimer();

    dlgSettings.InitDialog(Model);

    if (dlgSettings.exec() == QDialog::Accepted)
    {
        dlgSettings.ReInitModel(Model);
    }
    else
    {
        // Rejected
    }
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionRun_Run_ML_triggered()
{
//    if (WorkMode == WorkModeType::RunningML)
//        return;

    wgtMainGL->StopMainTimer();
    WorkMode = WorkModeType::RunningML;
    Model.StartML();
}
//-------------------------------------------------------------

void MainWindow::on_actionRun_Neuro_Car_triggered()
{
    WorkMode = WorkModeType::RunningNeuroCar;
    Model.ClearButLastBestCar();

    wgtMainGL->StartMainTimer();
}
//-------------------------------------------------------------

void MainWindow::on_actionRun_Manual_triggered()
{
    WorkMode = WorkModeType::RunningManual;
    Model.ClearButLastBestCar();
    wgtMainGL->StartMainTimer();
}
//-------------------------------------------------------------

void MainWindow::on_actionHelp_About_triggered()
{
    dlgAbout.exec();
}
//-------------------------------------------------------------

void MainWindow::on_actionView_Switch_Perspective_triggered()
{
    Model.SwitchPerspective();
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionView_Reset_View_Point_triggered()
{
    Model.ResetViewPoint();
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionView_Camera_on_Car_triggered()
{
    Model.SwitchCamera();
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionContinue_ML_based_on_first_car_triggered()
{
//    if (WorkMode == WorkModeType::RunningML)
//        return;

    wgtMainGL->StopMainTimer();
    WorkMode = WorkModeType::RunningML;
    Model.ContinueMLBasedOnFirstCar();  
}
//-------------------------------------------------------------

void MainWindow::on_actionTools_Save_Brain_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save brain file", ".", "Brain Files (*.brain)");

    if (fileName.isEmpty())
        return;

    Model.SaveLastBestBrainToFile(fileName);
}
//-------------------------------------------------------------

void MainWindow::on_actionTools_Load_Brain_triggered()
{
    wgtMainGL->StopMainTimer();

    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Brain file", ".", "Brain Files (*.brain)");

    if (fileName.isEmpty())
        return;

    if (!Model.LoadBrainFormFile(fileName))
        throw std::runtime_error("Brain could not be loaded");

    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionRun_Competition_triggered()
{
    WorkMode = WorkModeType::Competition;
    wgtMainGL->StopMainTimer();
    Model.CreateCompetition();
    wgtMainGL->StartMainTimer();
    wgtMainGL->repaint();
}
//-------------------------------------------------------------

void MainWindow::on_actionEdit_Clear_Lines_triggered()
{
    WorkMode = WorkModeType::Nothing;
    wgtMainGL->StopMainTimer();
    Model.ClearAllLines();
}
//-------------------------------------------------------------

void MainWindow::on_actionView_Hile_Lines_triggered()
{
    Model.SwitchHideLines();
    wgtMainGL->repaint();
}
//-------------------------------------------------------------




