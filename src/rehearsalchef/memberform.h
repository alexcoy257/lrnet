#ifndef MEMBERFORM_H
#define MEMBERFORM_H

#include <QWidget>
#include <QSettings>

#include "chatform.h"
#include <jackparameterform.h>

namespace Ui {
class MemberForm;
}

class MemberForm : public QWidget
{
    Q_OBJECT


public:
    explicit MemberForm(QWidget *parent = nullptr);
    virtual ~MemberForm();

    ChatForm * m_chatForm;

    void setName(const QString & nname);
    void setSection(const QString & nsection);

    void loadSetup(QSettings &settings);
    void saveSetup(QSettings &settings);

    void enableJackForm();
    void disableJackForm();

public slots:
    void handleJackPortsConnected();

signals:
    void nameUpdated(const QString & nname);
    void sectionUpdated(const QString & nsection);
    void setEncryption(bool e);
    void setjtSelfLoopback(bool e);
    void setLocalLoopback(bool e);
    void setNumChannels(int n);

    void sendClientMute(bool isMuted);

    void startJackTrip();
    void stopJackTrip();

    void changeRedundancy(int newRed);

    void doMute(bool m);

private:
    Ui::MemberForm *ui;
    //JackParameterForm * m_jackForm;

    void updateName();
    void fstartJacktrip();
    void fstopJacktrip();

    void enableJackTripButton();
    void disableJackTripButton();
    void toggleMute();

    bool muted = false;

};

#endif // MEMBERFORM_H
