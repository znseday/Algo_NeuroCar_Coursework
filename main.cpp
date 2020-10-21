#include "MainWindow.h"

#include <QApplication>
//#include <QDebug>

#include <random>

#include "NeuroNet.h"


int main(int argc, char *argv[])
{
    Neuron::InitRandGenByTime();
    NeuroNet::InitRandGenByTime();

    QSurfaceFormat sfmt;
    sfmt.setSamples(4);
    QSurfaceFormat::setDefaultFormat(sfmt);

    srand(time(nullptr));

    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
