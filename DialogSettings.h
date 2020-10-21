#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include "MainModel.h"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = nullptr);
    ~DialogSettings();

    void InitDialog(const MainModel &_model);
    void ReInitModel(MainModel &_model);

private:
    Ui::DialogSettings *ui;
};

#endif // DIALOGSETTINGS_H
