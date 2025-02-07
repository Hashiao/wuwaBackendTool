#ifndef GENERALPANEL_H
#define GENERALPANEL_H

#include <QWidget>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QDateTime>

#include "settingparams.h"
#include "utils.h"

namespace Ui {
class GeneralPanel;
}

class GeneralPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralPanel(QWidget *parent = nullptr);
    ~GeneralPanel();

    RebootGameSetting getRebootGameSetting() const;

    void setActivateStartTime(const QDateTime& dt);
    void setExpirationTime(const QDateTime& dt);
    void setCurrentTime(const QDateTime& dt);

private:
    Ui::GeneralPanel *ui;


};

#endif // GENERALPANEL_H
