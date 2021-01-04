#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QObject>
#include <QPushButton>
#include <QTimer>

#include "channelstrip.h"
#include "compressor.h"
#include "channeltester.h"
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
    ChannelStrip * m_channelStrip;
    Compressor * m_comp;

    QStackedWidget * m_stackedWidget;
    ConnectForm * m_connectForm;
    Launcher * m_launcherForm;
    QWidget * m_roleForm;

    ChefForm * m_chefForm;


    AuthTypeE m_authType;
    QSettings m_settings;
    LRNetClient * m_netClient;
    QTimer m_keepAliveTimer;
    QByteArray m_privateKey;
    QByteArray m_publicKey;
    RSA * keyPair;
    QString m_netid;
    QString m_name;
    QString m_section;
    QString m_hostname;
    int m_port;

    RCJTWorker * m_jacktrip;
    QThreadPool m_jacktripthreadpool;

    QAction *m_disconnectAction;
    QAction *m_changeRoleAction;

    void loadSetup();
    void keyInit();
    void saveSetup();


signals:
    void deleteChannel(int id);

private slots:
    void tryConnect(const QString & host, int port);
    void disconnected();
    void handleAuth(AuthTypeE type);
    void launchLauncher();
    void launchSuperChef();
    void launchChef();
    void launchMember();
    void changeRole();
    void setUdpPort(int port);
    void startJackTrip();
    void stopJackTrip();
    void startJackTripThread();
    void stopJackTripThread();

};
#endif // MAINWINDOW_H
