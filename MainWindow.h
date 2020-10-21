#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "MainGLWidget.h"
#include "MainModel.h"
#include "Types.h"

#include "DialogSettings.h"
#include "DialogAbout.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void AfterShow();

private slots:
    void on_actionFile_New_triggered();
    void on_actionFile_Open_triggered();
    void on_actionFile_Save_triggered();
    void on_actionFile_Save_As_triggered();
    void on_actionFile_Exit_triggered();

    void on_actionEdit_Start_Line_triggered();
    void on_actionEdit_End_Line_triggered();
    void on_actionEdit_Put_Start_triggered();

    void on_actionTools_Settings_triggered();

    void on_actionRun_Run_ML_triggered();
    void on_actionRun_Neuro_Car_triggered();
    void on_actionRun_Manual_triggered();

    void on_actionHelp_About_triggered();

    void on_actionView_Switch_Perspective_triggered();
    void on_actionView_Reset_View_Point_triggered();
    void on_actionView_Camera_on_Car_triggered();

    void on_actionContinue_ML_based_on_first_car_triggered();

    void on_actionTools_Save_Brain_triggered();
    void on_actionTools_Load_Brain_triggered();

    void on_actionRun_Competition_triggered();

    void on_actionEdit_Clear_Lines_triggered();

    void on_actionView_Hile_Lines_triggered();

public slots:
    void SlotReceiveWorldCoords(double wx, double wy, double wz, bool wExists);
    void SlotReceivePutStartFinished();
    void SlotReceiveCurrentFps(int fps);
    void SlotReceiveTimerStatus(bool isOn);
    void SlotReceiveCarInfo(double v, double s, int t, double vaver, double accel, double wheelSpeed);

private:
    Ui::MainWindow *ui;

    QLabel *lblWorldX, *lblWorldY;
    QLabel *lblFps;
    QLabel *lblCarSpeed, *lblCar_tInternal, *lblCarSsum, *lblCarVaver;
    QLabel *lblCarAccell, *lblCarWheelSpeed;
    QLabel *lblTimerStatus;

    MainGLWidget *wgtMainGL = nullptr;

    WorkModeType WorkMode = WorkModeType::Nothing;

    MainModel Model;

    QString FileName;

    DialogSettings dlgSettings;

    DialogAbout dlgAbout;

protected:
    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
