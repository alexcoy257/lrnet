#ifndef CHEFFORM_H
#define CHEFFORM_H

#include <QWidget>
#include <QDebug>
#include <QSettings>
#include <QHBoxLayout>

#include "channelstrip.h"
#include "compressor.h"
#include "chatform.h"
#include "talkbacksettingsform.h"

namespace Ui {
class ChefForm;
}

typedef struct LRMClient{
    ChannelStrip * cs;
    Compressor * comp;
    int64_t id;
}LRMClient;

class ChefForm : public QWidget
{
    Q_OBJECT


    Compressor * m_actComp = NULL;
    QHash<int, LRMClient *> m_clients;
    QHash<int, LRMClient *> m_soloers;
    TalkbackSettingsForm * m_tbSetupForm = NULL;
    QHBoxLayout * m_csAreaLayout;
public:
    explicit ChefForm(QWidget *parent = nullptr);
    virtual ~ChefForm();


public slots:
    void highlightInsert(Compressor * cp);
    void updateAuthCodeStatus(bool enabled, const QString & authCode);
    void handleAuthCodeEnabledUpdated(bool enabled);
    void updateAuthCodeLabel(const QString & authCode);
    void soloRequested(int id, bool checked);
    void handleSoloResponse(int id, bool isSolo);
    void handleJoinMutedResponse(bool joinMuted);
    void addChannelStrip(const QString& mName, const QString& sName, QVector<float> controls, int id);
    void updateChannelStrip(const QString& mName, const QString& sName, int id);
    void updateChannelStripControls(QVector<float> &controls, int id);
    void deleteChannelStrip(int id);

    void loadSetup(QSettings &settings);
    void saveSetup(QSettings &settings);

signals:
    void sendControlUpdate(int id, QVector<float> & controls);
    void sendSoloUpdate(int id, bool isSolo);
    void sendJoinMutedUpdate(bool joinMuted);
    void authCodeUpdated(const QString & nname);
    void authCodeEnabledUpdated(bool enabled);
    void startJackTrip();
    void stopJackTrip();
    void setjtSelfLoopback(bool e);
    void doMute(bool m);
    void startJacktripSec();
    void stopJacktripSec();

private:
    Ui::ChefForm *ui;
    void updateAuthCode();
    void updateAuthCodeEnabled();
    bool muted = true;
    void fstartJacktripSec();
    void fstopJacktripSec();

private slots:
    void newValueHandler(LRMClient * myClient, int type, float value);
    void disableJackForm();
    void enableJackForm();
    void toggleMute();

public:
    ChatForm * m_chatForm;
};

#endif // CHEFFORM_H
