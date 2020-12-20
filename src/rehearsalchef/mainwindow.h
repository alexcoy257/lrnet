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
#include "lrchef_connectform.h"
#include "../lrnetclient/lrnetclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; class ChannelStrip;}
QT_END_NAMESPACE

typedef struct{
    ChannelStrip * cs;
    Compressor * comp;
}LRMClient;

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
    Compressor * m_actComp;
    QHash<int, LRMClient *> m_clients;
    ConnectForm * m_connectForm;
    QPushButton * m_openConnectFormButton;
    QSettings m_settings;
    LRNetClient * m_netClient;
    QTimer m_keepAliveTimer;
    QByteArray m_privateKey;
    QByteArray m_publicKey;
    RSA * keyPair;

    void loadSetup();
    void keyInit();
    void saveSetup();


signals:
    void deleteChannel(int id);

private slots:
    void highlightInsert(Compressor * cp);
    void addChannelStrip(const QString& mName, const QString& sName, int id);
    void deleteChannelStrip(int id);
};
#endif // MAINWINDOW_H
