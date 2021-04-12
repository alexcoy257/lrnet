#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QObject>
#include <QPushButton>
#include <QTimer>
#include <QNetworkInterface>

#include "channelstrip.h"
#include "compressor.h"
#include "channeltester.h"
#include "superchefform.h"
#include "chefform.h"
#include "memberform.h"
#include "lrchef_connectform.h"
#include "launcher.h"

#ifdef LIBLRNET_LIBRARY
#undef LIBLRNET_LIBRARY
#endif
#include "../liblrnet_globals.h"



#include "rcjtworker.h"


#include "../lrnetserver/auth_types.h"
#include "../lrnetclient/lrnetclient.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; class ChannelStrip;}
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void releaseThread(int n);


private:
    Ui::MainWindow *ui;
    QSize m_connectFormSize;
    ChannelStrip * m_channelStrip;
    Compressor * m_comp;

    QStackedWidget * m_stackedWidget;
    ConnectForm * m_connectForm;
    Launcher * m_launcherForm;
    AuthTypeE m_role;
    QWidget * m_roleForm;
    QLabel * m_connectionLabel;

    AuthTypeE m_authType;
    QSettings m_settings;
    LRNetClient * m_netClient;
    QTimer m_keepAliveTimer;
    QTimer * m_netStatusTimer;
    QByteArray m_privateKey;
    QByteArray m_publicKey;
    RSA * keyPair;
    QString m_netid;
    QString m_name;
    QString m_section;
    QString m_hostname;
    int m_port;

    RCJTWorker * m_jacktrip;
    RCJTWorker * m_sJacktrip;
    jack_client_t * m_jackClient = NULL;
    QThreadPool m_jacktripthreadpool;
    bool m_expectSecondary = false;
    int m_secPort = 0;

    jack_port_t * pri_fromPorts[2] = {NULL, NULL};
    jack_port_t * pri_toPorts[2] = {NULL, NULL};
    jack_port_t * pri_broadcastPorts[2] = {NULL, NULL};
    jack_port_t * sec_fromPorts[2] = {NULL, NULL};
    jack_port_t * sec_toPorts[2] = {NULL, NULL};
    jack_port_t * sec_broadcastPorts[2] = {NULL, NULL};

    QAction *m_disconnectAction;
    QAction *m_changeRoleAction;

    void loadSetup();
    void keyInit();
    void saveSetup();

    enum {
        CH_1_2,
        CH_3_4
    } muteGroup_e;


signals:
    void deleteChannel(int id);
    void sendPublicKey();

private slots:
    void tryConnect(const QString & host, int port);
    void disconnected();
    void handleAuth(AuthTypeE type);
    void storeKeyResultReceived(bool success);
    void launchLauncher();
    void launchSuperChef();
    void launchChef();
    void launchMember();
    void changeRole();
    void updateConnectionLabel();
    void setUdpPort(int port);
    void startJackTrip();
    void stopJackTrip();
    void startJackTripSecondary();
    void stopJackTripSecondary();
    void startJackTripThread();
    void stopJackTripThread();
    void setRedundancy(int n);
    void setEncryption(bool e);
    void setEncryptionKey(char * key);
    void setNumChannels(int n);
    void connectPrimary();
    void connectPrimarySend();
    void connectPrimaryReceive();
    void disconnectPrimary();
    void disconnectPrimarySend();
    void disconnectPrimaryReceive();
    void connectSecondary();
    void disconnectSecondary();
    void doPrimaryMute(bool m);
    void setLocalLoopback(bool l);

};
#endif // MAINWINDOW_H
