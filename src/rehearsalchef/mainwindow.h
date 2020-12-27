#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
#include "../liblrnet_globals.h"


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


private:
    Ui::MainWindow *ui;
    ChannelStrip * m_channelStrip;
    Compressor * m_comp;

    ChefForm * m_chefForm;


    ConnectForm * m_connectForm;
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

    void loadSetup();
    void keyInit();
    void saveSetup();


signals:
    void deleteChannel(int id);

private slots:
    void tryConnect(const QString & host, int port);
    void handleAuth(AuthTypeE type);
    void launchSuperChef();
    void launchChef();
    void launchMember();

};
#endif // MAINWINDOW_H
