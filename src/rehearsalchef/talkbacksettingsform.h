#ifndef TALKBACKSETTINGSFORM_H
#define TALKBACKSETTINGSFORM_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class TalkbackSettingsForm;
}

class TalkbackSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit TalkbackSettingsForm(QWidget *parent = nullptr);
    ~TalkbackSettingsForm();
    void enableJackForm();
    void disableJackForm();
    void loadSetup(QSettings &settings);
    void saveSetup(QSettings &settings);
    bool getjtSelfLoopback();

signals:
    void jackStarted();
    void jackStopped();
    void startJackTrip();
    void stopJackTrip();
    void setjtSelfLoopback(bool e);

private:
    Ui::TalkbackSettingsForm *ui;

private slots:
    void fStartJacktrip();
    void fStopJacktrip();
    void enableJackTripButton();
    void disableJackTripButton();

};

#endif // TALKBACKSETTINGSFORM_H
