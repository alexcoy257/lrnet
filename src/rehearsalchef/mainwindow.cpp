#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_connectFormSize(528,225)
    , m_stackedWidget(new QStackedWidget())
    , m_connectForm(new ConnectForm())
    , m_launcherForm(NULL)
    , m_role(NONE)
    , m_roleForm(NULL)
    , m_authType(NONE)
    , m_settings(REHEARSALCHEF_DOMAIN, REHEARSALCHEF_TITLE)
    , m_netClient(new LRNetClient())
    , m_keepAliveTimer(this)
    , m_netStatusTimer(new QTimer(this))
    , m_connectionLabel(new QLabel(""))
    , m_privateKey({})
    , m_publicKey({})
    , keyPair(NULL)
    , m_jacktrip( new RCJTWorker(this, 10, JackTrip::ZEROS, ""))
    , m_sJacktrip(NULL)
{
    ui->setupUi(this);
    m_connectionLabel->hide();
    //m_connectionLabel->setStyleSheet("QLabel {color:yellow;}");

    m_jacktrip->setPortCBAreas(pri_fromPorts, pri_toPorts, pri_broadcastPorts, 2);

    m_stackedWidget->addWidget(m_connectForm);

    ui->mainLayout->addWidget(m_stackedWidget);
    ui->mainLayout->addWidget(m_connectionLabel);


    // Actions menu
    QMenu *actionsMenu = menuBar()->addMenu(tr("&Actions"));

    m_disconnectAction = new QAction("&Disconnect", this);
    m_disconnectAction->setShortcut(QKeySequence(tr("Ctrl+D")));
    m_disconnectAction->setEnabled(false);
    actionsMenu->addAction(m_disconnectAction);
    QObject::connect(m_disconnectAction, &QAction::triggered, m_netClient, &LRNetClient::disconnectFromHost);

    m_changeRoleAction = new QAction("&Change role", this);
    m_changeRoleAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    m_changeRoleAction->setEnabled(false);
    actionsMenu->addAction(m_changeRoleAction);
    QObject::connect(m_changeRoleAction, &QAction::triggered, this, &MainWindow::changeRole);

    m_netStatusTimer->callOnTimeout(this, &MainWindow::updateConnectionLabel);
    m_netStatusTimer->start(100);

    QObject::connect(m_connectForm, &ConnectForm::tryConnect, this, &MainWindow::tryConnect);

    QObject::connect(m_connectForm, &ConnectForm::updateCode, this, [=](const QString & code){
        m_netClient->setTempCode(code);
    });
    QObject::connect(m_connectForm, &ConnectForm::setToKeyAuth, m_netClient, &LRNetClient::setKeyAuthMethod);
    QObject::connect(m_connectForm, &ConnectForm::setToCodeAuth, m_netClient, &LRNetClient::setCodeAuthMethod);
    QObject::connect(m_connectForm, &ConnectForm::updateHost, this, [=](const QString & host){m_hostname = host;});
    QObject::connect(m_connectForm, &ConnectForm::updatePort, this, [=](int port){m_port = port;});

    QObject::connect(m_netClient, &LRNetClient::timeout, this, [=](){statusBar()->showMessage("Timed out");});
    QObject::connect(m_netClient, &LRNetClient::connected, this,
                     [=](){
        qDebug()<<"Connected";
        QObject::disconnect(m_connectForm, &ConnectForm::tryConnect, this, &MainWindow::tryConnect);
        m_connectForm->m_submitButton->setText("Login");
        QObject::connect(m_connectForm, &ConnectForm::tryConnect, m_netClient, &LRNetClient::tryToAuthenticate);
    });
    QObject::connect(m_netClient, &LRNetClient::disconnected, this, &MainWindow::disconnected);
    QObject::connect(m_netClient, &LRNetClient::authenticated, this, &MainWindow::handleAuth);
    QObject::connect(this, &MainWindow::sendPublicKey, m_netClient, &LRNetClient::sendPublicKey);
    QObject::connect(m_netClient, &LRNetClient::storeKeyResultReceived, this, &MainWindow::storeKeyResultReceived);
    QObject::connect(m_netClient, &LRNetClient::authFailed, this, [=](){statusBar()->showMessage("Login failed");});
    QObject::connect(m_netClient, &LRNetClient::authCodeIncorrect, this, [=](){statusBar()->showMessage("Incorrect login code");});
    QObject::connect(m_netClient, &LRNetClient::authCodeDisabled, this, [=](){statusBar()->showMessage("Guest login disabled.  Ask a leader to enable it");});
    QObject::connect(m_connectForm, &ConnectForm::netidUpdated, this, [&](const QString & nnetid){m_netid = nnetid; m_netClient->setNetid(nnetid);});
    QObject::connect(m_netClient, &LRNetClient::gotEncryptionKey, this, &MainWindow::setEncryptionKey);


    /*
    m_channelStrip = new ChannelStrip(this);
    ui->m_channelStripArea->addWidget(m_channelStrip);
    m_comp = new Compressor(this);
    m_comp->hide();

*/

    //ui->m_channelStripArea->addWidget(m_channelStrip);
    //m_channelStrip->setCompressorZone(ui->m_actCompArea);

    loadSetup();
    keyInit();
    m_netClient->setRSAKey( keyPair );
    m_connectForm->m_netidBox->setText(m_netid);
    m_netClient->setNetid(m_netid);
    m_connectForm->setHost(m_hostname);
    m_connectForm->setPort(m_port);

    setMinimumSize(m_connectFormSize);
    resize(m_connectFormSize);
}





MainWindow::~MainWindow()
{
//    m_jacktrip->stopThread();

    stopJackTrip();
    saveSetup();

    delete ui;
    delete m_netClient;
    delete m_roleForm;
    //delete m_connectForm;
    RSA_free(keyPair);

}

void MainWindow::updateConnectionLabel(){
    QString label = "";
    for(const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.type() == QNetworkInterface::Wifi && iface.flags().testFlag(QNetworkInterface::IsUp)) {
            label = "Active wifi interface detected. Consider using ethernet only for enhanced reliability.";
        }
    }
    if (label.isEmpty())
        m_connectionLabel->hide();
    else {
        m_connectionLabel->setText(label);
        m_connectionLabel->show();
    }
}

void MainWindow::keyInit(){
    //loadSetup();

    if (m_publicKey.length() == 0){
        RSA * t_keyPair = RSA_new();
        BIGNUM * e = NULL;
        BN_dec2bn(&e, "3");
        RSA_generate_key_ex(t_keyPair, 2048,e, NULL);
        //m_keyCopyGenButton->setText("Generate Public Key");
        BN_free(e);

        BIO * t_pri = BIO_new(BIO_s_mem());
        BIO * t_pub = BIO_new(BIO_s_mem());
        PEM_write_bio_RSAPrivateKey(t_pri, t_keyPair, NULL, NULL, 0, NULL, NULL);
        PEM_write_bio_RSA_PUBKEY(t_pub, t_keyPair);
        size_t len = BIO_pending(t_pri);

        void *pri_key = malloc(len);
        BIO_read(t_pri, pri_key, len);

        m_privateKey = QByteArray::fromRawData((const char *)pri_key, len);

        len = BIO_pending(t_pub);
        void *pub_key = malloc(len);
        BIO_read(t_pub, pub_key, len);
        m_publicKey = QByteArray::fromRawData((const char *)pub_key, len);
        qDebug() <<"Made new public key" <<m_publicKey;

        BIO_free(t_pri);
        BIO_free(t_pub);
        RSA_free(t_keyPair);
    }
    else{
        m_connectForm->m_lPublicKey->setPlainText(m_publicKey);
    }

    BIO * t_pri = BIO_new(BIO_s_mem());
    BIO * t_pub = BIO_new(BIO_s_mem());

    keyPair = RSA_new();

    qDebug() <<"Public key length " <<m_publicKey.length();
    BIO_write(t_pub, m_publicKey.data(), m_publicKey.length());
    RSA * trsa = PEM_read_bio_RSA_PUBKEY(t_pub, &keyPair, NULL, NULL);

    if (!trsa){
        qDebug() <<"RSA read failed somehow. Quitting";
        std::exit(1);
    }

    BIO_write(t_pri, m_privateKey.data(), m_privateKey.length());
    PEM_read_bio_RSAPrivateKey(t_pri, &keyPair, NULL, NULL);

    BIO_free(t_pri);
    BIO_free(t_pub);

    qDebug() << "Key Size: " <<RSA_size(keyPair);


    int numBytes = RSA_size(keyPair) - 43;

    QByteArray loca(numBytes, 0);
    QByteArray eloca(numBytes + 43, 0);
    QByteArray dloca(numBytes, 0);
    unsigned int retlen;


    unsigned char * loc = (unsigned char *) loca.data();
    unsigned char * eloc = (unsigned char *) eloca.data();
    unsigned char * dloc = (unsigned char *) dloca.data();

    auth_packet_t pck;

    if(loc && eloc && dloc){
        RAND_bytes(pck.challenge, 214);

        RSA_sign(NID_sha256, pck.challenge, 214, pck.sig, &retlen, keyPair);
        if (RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, retlen, keyPair))
            qDebug() <<"Verified! Len:" <<retlen;
        else
            qDebug() <<"Failed to verify.";

    }
    if (RSA_check_key(keyPair)){
        qDebug() <<"Key valid";
    }
    else{
        qDebug() <<"Key invalid";
    }

    statusBar()->showMessage("SSL Okay");
}


void MainWindow::tryConnect(const QString & host, int port){
    m_netClient->tryConnect(host, port);
    QString base= "Connecting to %1 on %2";
    statusBar()->showMessage(base.arg(host).arg(port));
}

void MainWindow::disconnected(){
    m_stackedWidget->setCurrentWidget(m_connectForm);
    setMinimumSize(m_connectFormSize);
    resize(m_connectFormSize);
    delete m_launcherForm;
    m_launcherForm = NULL;
    if (m_role == MEMBER){
        ((MemberForm *)m_roleForm)->saveSetup(m_settings);
    }
    if(m_roleForm)
    delete m_roleForm;
    m_role = NONE;
    m_roleForm = NULL;
    m_authType = NONE;

    m_disconnectAction->setEnabled(false);

    QObject::disconnect(m_connectForm, &ConnectForm::tryConnect, m_netClient, &LRNetClient::tryToAuthenticate);
    m_connectForm->m_submitButton->setText("Connect");
    QObject::connect(m_connectForm, &ConnectForm::tryConnect, this, &MainWindow::tryConnect);
}

void MainWindow::loadSetup(){
    m_settings.beginGroup("/Auth");
    m_hostname = m_settings.value("Host","localhost").toString();
    m_port = m_settings.value("Port",4463).toInt();
    m_netid = m_settings.value("Netid","").toString();
    //qDebug()  <<"Netid: " <<m_netid;
    m_publicKey = m_settings.value("PublicKey","").toByteArray();
    m_privateKey = m_settings.value("PrivateKey","").toByteArray();
    //qDebug() <<"Public key: " <<m_publicKey;
    m_settings.endGroup();
}

void MainWindow::saveSetup(){
    m_settings.beginGroup("/Auth");
    m_settings.setValue("Host", m_hostname);
    m_settings.setValue("Port", m_port);
    m_settings.setValue("Netid", m_netid);
    m_settings.setValue("PublicKey",m_publicKey);
    m_settings.setValue("PrivateKey",m_privateKey);
    m_settings.endGroup();

    if (m_role == MEMBER){
        ((MemberForm *)m_roleForm)->saveSetup(m_settings);
    }

    m_settings.sync();
}

void MainWindow::handleAuth(AuthTypeE type){
    m_authType = type;
    if (!m_connectForm->m_usingKey && m_connectForm->m_csw->m_saveKeyCheckbox->isChecked())
        emit sendPublicKey();

    launchLauncher();
}

void MainWindow::changeRole(){
    m_stackedWidget->setCurrentWidget(m_launcherForm);
    setMinimumSize(m_connectFormSize);
    resize(m_connectFormSize);
    if (m_role == MEMBER){
        ((MemberForm *)m_roleForm)->saveSetup(m_settings);
    }
     QObject::disconnect(m_netClient, &LRNetClient::gotUdpPort, this, &MainWindow::setUdpPort);
    delete m_roleForm;
    m_role = NONE;
    m_roleForm = NULL;
    m_netClient->unsubscribe();
    m_changeRoleAction->setDisabled(true);
}

void MainWindow::launchLauncher(){

    if (m_authType != NONE){
        m_launcherForm = new Launcher(m_authType, this);
        m_stackedWidget->addWidget(m_launcherForm);
        m_stackedWidget->setCurrentWidget(m_launcherForm);
        connect(m_launcherForm, &Launcher::choseSuperChef, this, &MainWindow::launchSuperChef);
        connect(m_launcherForm, &Launcher::choseChef, this, &MainWindow::launchChef);
        connect(m_launcherForm, &Launcher::choseMember, this, &MainWindow::launchMember);

        m_disconnectAction->setEnabled(true);
    }
}

void MainWindow::launchSuperChef(){
qDebug()<<"Chose superchef";
m_roleForm = new SuperChefForm(this);
m_role = SUPERCHEF;
m_netClient->subSuperChef();
m_stackedWidget->addWidget(m_roleForm);
m_stackedWidget->setCurrentWidget(m_roleForm);
m_changeRoleAction->setEnabled(true);

QObject::connect(((SuperChefForm *)m_roleForm), &SuperChefForm::requestRoles, m_netClient, &LRNetClient::requestRoles);
QObject::connect(((SuperChefForm *)m_roleForm), &SuperChefForm::updatePermissions, m_netClient, &LRNetClient::updatePermissions);
QObject::connect(((SuperChefForm *)m_roleForm), &SuperChefForm::removeUsers, m_netClient, &LRNetClient::removeUsers);
QObject::connect(m_netClient, &LRNetClient::rolesReceived, (SuperChefForm *)m_roleForm, &SuperChefForm::updateLists);

resize(m_roleForm->sizeHint());
}

void MainWindow::launchChef(){
    m_roleForm = new ChefForm(this);
    m_role = CHEF;
    m_stackedWidget->addWidget(m_roleForm);
    m_stackedWidget->setCurrentWidget(m_roleForm);
    QObject::connect(m_netClient, &LRNetClient::newMember, (ChefForm *)m_roleForm, &ChefForm::addChannelStrip);
    QObject::connect(m_netClient, &LRNetClient::updateMember, (ChefForm *)m_roleForm, &ChefForm::updateChannelStrip);
    QObject::connect(m_netClient, &LRNetClient::updateMemberControls, (ChefForm *)m_roleForm, &ChefForm::updateChannelStripControls);
    QObject::connect(m_netClient, &LRNetClient::lostMember, (ChefForm *)m_roleForm, &ChefForm::deleteChannelStrip);
    QObject::connect(m_netClient, &LRNetClient::chatReceived, ((ChefForm *)m_roleForm)->m_chatForm, &ChatForm::appendMessage);
    QObject::connect(m_netClient, &LRNetClient::updateAuthCodeStatus, (ChefForm *)m_roleForm, &ChefForm::updateAuthCodeStatus);
    QObject::connect(m_netClient, &LRNetClient::handleSoloResponse, (ChefForm *)m_roleForm, &ChefForm::handleSoloResponse);
    QObject::connect(m_netClient, &LRNetClient::handleJoinMutedResponse, (ChefForm *)m_roleForm, &ChefForm::handleJoinMutedResponse);
    QObject::connect(m_netClient, &LRNetClient::serverUpdatedAuthCodeEnabled, (ChefForm *)m_roleForm, &ChefForm::handleAuthCodeEnabledUpdated);
    QObject::connect(m_netClient, &LRNetClient::serverUpdatedAuthCode, (ChefForm *)m_roleForm, &ChefForm::updateAuthCodeLabel);
    QObject::connect(((ChefForm *)m_roleForm)->m_chatForm, &ChatForm::sendChat, m_netClient, &LRNetClient::sendChat);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::authCodeUpdated, m_netClient, &LRNetClient::sendAuthCode);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::authCodeEnabledUpdated, m_netClient, &LRNetClient::updateAuthCodeEnabled);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::sendControlUpdate, m_netClient, &LRNetClient::sendControlUpdate);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::sendSoloUpdate, m_netClient, &LRNetClient::sendSoloUpdate);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::sendJoinMutedUpdate, m_netClient, &LRNetClient::sendJoinMutedUpdate);
    QObject::connect(((ChefForm *)m_roleForm), &ChefForm::authCodeEnabledUpdated, m_netClient, &LRNetClient::updateAuthCodeEnabled);
    QObject::connect((ChefForm *)m_roleForm, &ChefForm::setjtSelfLoopback, m_netClient, &LRNetClient::setjtSelfLoopback);
    QObject::disconnect(m_jacktrip, &RCJTWorker::jackPortsReady, this, &MainWindow::connectPrimary);
    QObject::connect(m_jacktrip, &RCJTWorker::jackPortsReady, this, &MainWindow::connectPrimaryReceive, Qt::QueuedConnection);



    m_sJacktrip = new RCJTWorker(this, 10, JackTrip::ZEROS, "");
    m_sJacktrip->setPortCBAreas(sec_fromPorts, sec_toPorts, sec_broadcastPorts, 2);
    QObject::connect(m_sJacktrip, &RCJTWorker::jackPortsReady, this, &MainWindow::connectSecondary, Qt::QueuedConnection);

    QObject::connect((ChefForm *)m_roleForm, &ChefForm::doMute, this, [=](bool m){
        if (m) disconnectPrimarySend();
        else connectPrimarySend();
    });


    QObject::connect((ChefForm *)m_roleForm, &ChefForm::startJackTrip, this, &MainWindow::startJackTrip);
    QObject::connect((ChefForm *)m_roleForm, &ChefForm::startJacktripSec, this, &MainWindow::startJackTripSecondary);
    QObject::connect(m_netClient, &LRNetClient::gotUdpPort, this, &MainWindow::setUdpPort);
    m_netClient->subChef();
    ((ChefForm *)m_roleForm)->loadSetup(m_settings);
    m_changeRoleAction->setEnabled(true);

    resize(800,725);
}


//---------------------------------------------------------------------------------------
void MainWindow::launchMember(){
    m_roleForm = new MemberForm(this);
    m_role = MEMBER;
    m_stackedWidget->addWidget(m_roleForm);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::nameUpdated, m_netClient, &LRNetClient::updateName);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::sectionUpdated, m_netClient, &LRNetClient::updateSection);
    QObject::connect(m_netClient, &LRNetClient::chatReceived, ((MemberForm *)m_roleForm)->m_chatForm, &ChatForm::appendMessage);
    QObject::connect(((MemberForm *)m_roleForm)->m_chatForm, &ChatForm::sendChat, m_netClient, &LRNetClient::sendChat);
    m_netClient->subMember();

    QObject::connect((MemberForm *)m_roleForm, &MemberForm::changeRedundancy, this, &MainWindow::setRedundancy);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::setNumChannels, this, &MainWindow::setNumChannels);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::setEncryption, this, &MainWindow::setEncryption);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::setjtSelfLoopback, m_netClient, &LRNetClient::setjtSelfLoopback);
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::setLocalLoopback, this, &MainWindow::setLocalLoopback);


    QObject::connect(m_jacktrip, &RCJTWorker::jackPortsReady, this,
            &MainWindow::connectPrimary, Qt::QueuedConnection);

    QObject::connect((MemberForm *)m_roleForm, &MemberForm::doMute, this, [=](bool m){
        if (m) disconnectPrimarySend();
        else connectPrimarySend();
    });

    QObject::connect((MemberForm *)m_roleForm, &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);

    QObject::connect(m_netClient, &LRNetClient::gotUdpPort, this, &MainWindow::setUdpPort);

    ((MemberForm *)m_roleForm)->loadSetup(m_settings);
    m_changeRoleAction->setEnabled(true);
    m_stackedWidget->setCurrentWidget(m_roleForm);
    resize(600,575);
}


//---------------------------------------------------------------------------------------
void MainWindow::setUdpPort(int port){
    if(m_expectSecondary){
        qDebug() << "Set secondary udp port to" <<port;
        m_sJacktrip->setJackTrip(m_hostname, 4464, port, 2, false);
        m_jacktripthreadpool.start(m_sJacktrip, QThread::TimeCriticalPriority);
    }
    else{
    m_jacktrip->setJackTrip(m_hostname, 4463, port, 2, false);
    }
}


//---------------------------------------------------------------------------------------
void MainWindow::startJackTrip(){
    switch (m_role){
    case MEMBER:
        QObject::disconnect((MemberForm *)m_roleForm, &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);
        QObject::connect((MemberForm *)m_roleForm, &MemberForm::stopJackTrip, this, &MainWindow::stopJackTrip);
        qDebug() << "Want to start JackTrip";
        m_netClient->startJackTrip(MEMBER);
        ((MemberForm *)m_roleForm)->disableJackForm();
        QObject::connect(m_netClient, &LRNetClient::serverJTReady, this, &MainWindow::startJackTripThread);
        break;

    case CHEF:
        QObject::disconnect((ChefForm *)m_roleForm, &ChefForm::startJackTrip, this, &MainWindow::startJackTrip);
        QObject::connect((ChefForm *)m_roleForm, &ChefForm::stopJackTrip, this, &MainWindow::stopJackTrip);
        qDebug() << "Want to start JackTrip for talkback";
        m_netClient->startJackTrip(CHEF);
        QObject::connect(m_netClient, &LRNetClient::serverJTReady, this, &MainWindow::startJackTripThread);
        break;

    default:
        break;

    }
}

void MainWindow::startJackTripThread(){
    jack_status_t status;
    if (!m_jackClient)
    m_jackClient = jack_client_open ("rcclient", JackNoStartServer, &status);
    //QThread::msleep(500);
    QObject::disconnect(m_netClient, &LRNetClient::serverJTReady, this, &MainWindow::startJackTripThread);

    m_jacktripthreadpool.start(m_jacktrip, QThread::TimeCriticalPriority);

}

void MainWindow::stopJackTrip(){
    switch (m_role){
    case MEMBER:
        QObject::disconnect((MemberForm *)m_roleForm, &MemberForm::stopJackTrip, this, &MainWindow::stopJackTrip);
        QObject::connect((MemberForm *)m_roleForm, &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);
        qDebug() << "Want to stop JackTrip";
        m_netClient->stopJackTrip(MEMBER);
        ((MemberForm *)m_roleForm)->enableJackForm();
        break;
    case CHEF:
        QObject::disconnect((ChefForm *)m_roleForm, &ChefForm::stopJackTrip, this, &MainWindow::stopJackTrip);
        QObject::connect((ChefForm *)m_roleForm, &ChefForm::startJackTrip, this, &MainWindow::startJackTrip);
        qDebug() << "Want to stop JackTrip";
        m_netClient->stopJackTrip(CHEF);
        break;
    default:
        break;
    }
    if(m_jackClient){
        jack_deactivate(m_jackClient);
        jack_client_close(m_jackClient);
        m_jackClient=NULL;
    }
    stopJackTripThread();
}

void MainWindow::stopJackTripThread(){
    //QThread::msleep(500);
    m_jacktrip->stopThread();
}


void MainWindow::storeKeyResultReceived(bool success){
    if (success){
        QString base = "Success!  'User Login' now available for %1 on this computer";
        statusBar()->showMessage(base.arg(m_connectForm->m_netidBox->text()));

    } else {
        QString base = "Failed to remember you...  Try again?";
        statusBar()->showMessage(base);
    }
}

void MainWindow::releaseThread(int n){
    if (m_role != MEMBER && m_role != CHEF)
        return;
    QObject::connect((MemberForm *)m_roleForm, &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);
}

void MainWindow::setRedundancy(int n){
    m_netClient->setRedundancy(n);
    ui->statusbar->showMessage(QString("Set redundancy to %1").arg(n), 1000);
    m_jacktrip->setRedundancy(n);
}

void MainWindow::setEncryption(bool e){
    qDebug() <<"Todo: enable encryption";
    m_netClient->setEncryption(e);
}

void MainWindow::setEncryptionKey(char * key){
    qDebug() <<"Consuming encryption key" << QByteArray(key, 32);
    m_jacktrip->setEncryptionKey(key);

    startJackTripThread();
    //delete[] key;
}

void MainWindow::connectPrimary(){
    connectPrimarySend();
    connectPrimaryReceive();
}

void MainWindow::connectPrimarySend(){
    if (m_jackClient){
        const char** ports;

            // Get physical output (capture) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsOutput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical capture ports";
            }
            else
            {
                // Connect capure ports to jacktrip send
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        qDebug() <<"Primary: connect" <<ports[i] <<"to"<<jack_port_name(pri_toPorts[i]);
                        jack_connect(m_jackClient, ports[i], jack_port_name(pri_toPorts[i]));
                    }
                }
                std::free(ports);
            }
    }}

void MainWindow::connectPrimaryReceive(){
if (m_jackClient){

        const char** ports;

            // Get physical input (playback) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsInput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical playback ports";
            }
            else
            {
                // Connect playback ports to jacktrip receive
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        qDebug() <<"Primary: connect" <<ports[i] <<"to"<<jack_port_name(pri_fromPorts[i]);
                        jack_connect(m_jackClient, jack_port_name(pri_fromPorts[i]), ports[i]);
                    }
                }
                std::free(ports);
            }
    }
}

void MainWindow::disconnectPrimary(){
    disconnectPrimarySend();
    disconnectPrimaryReceive();
}

void MainWindow::disconnectPrimarySend(){
    if (m_jackClient){
        const char** ports;

            // Get physical output (capture) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsOutput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical capture ports";
            }
            else
            {
                // Connect capure ports to jacktrip send
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        qDebug() <<"Primary: connect" <<ports[i] <<"to"<<jack_port_name(pri_toPorts[i]);
                        jack_disconnect(m_jackClient, ports[i], jack_port_name(pri_toPorts[i]));
                    }
                }
                std::free(ports);
            }
    }}

void MainWindow::disconnectPrimaryReceive(){
if (m_jackClient){

        const char** ports;

            // Get physical input (playback) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsInput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical playback ports";
            }
            else
            {
                // Connect playback ports to jacktrip receive
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        qDebug() <<"Primary: connect" <<ports[i] <<"to"<<jack_port_name(pri_fromPorts[i]);
                        jack_disconnect(m_jackClient, jack_port_name(pri_fromPorts[i]), ports[i]);
                    }
                }
                std::free(ports);
            }
    }
}

void MainWindow::connectSecondary(){
    if (m_jackClient){
        qDebug() <<"Connect secondary";
        const char** ports;

            // Get physical output (capture) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsOutput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical capture ports";
            }
            else
            {
                // Connect capure ports to jacktrip send
                for (int i = 0; i < 4; i++)
                {

                    if (!ports[i])
                        break;
                    // Check that we don't run out of capture ports
                    if ( (i==2 || i==3) && ports[i] ) {
                        qDebug() <<"Secondary: connect" <<ports[i] <<"to"<<jack_port_name(sec_toPorts[i-2]);
                        jack_connect(m_jackClient, ports[i], jack_port_name(sec_toPorts[i-2]));
                    }
                }
                std::free(ports);
            }
    }
}

void MainWindow::disconnectSecondary(){
    if (m_jackClient){
        const char** ports;

            // Get physical output (capture) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsOutput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical capture ports";
            }
            else
            {
                // Connect capure ports to jacktrip send
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        jack_disconnect(m_jackClient, ports[i], jack_port_name(pri_toPorts[i]));
                    }
                }
                std::free(ports);
            }

            // Get physical input (playback) ports
            if ( (ports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsInput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical playback ports";
            }
            else
            {
                // Connect playback ports to jacktrip receive
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( ports[i] != NULL ) {
                        jack_disconnect(m_jackClient, jack_port_name(pri_fromPorts[i]), ports[i]);
                    }
                }
                std::free(ports);
            }
    }
}

void MainWindow::setNumChannels(int n){

    m_netClient->setNumChannels(n);
}

void MainWindow::startJackTripSecondary(){
    switch (m_role){
    case CHEF:
        QObject::disconnect((ChefForm *)m_roleForm, &ChefForm::startJacktripSec, this, &MainWindow::startJackTripSecondary);
        QObject::connect((ChefForm *)m_roleForm, &ChefForm::stopJacktripSec, this, &MainWindow::stopJackTripSecondary);
        qDebug() << "Want to start JackTrip secondary";
        m_expectSecondary = true;

        m_netClient->startJackTripSec();
        //QObject::connect(m_netClient, &LRNetClient::serverJTReady, this, &MainWindow::startJackTripThread);
        break;

    default:
        break;

    }
}

void MainWindow::stopJackTripSecondary(){
    switch (m_role){
    case CHEF:
        QObject::disconnect((ChefForm *)m_roleForm, &ChefForm::stopJacktripSec, this, &MainWindow::stopJackTripSecondary);
        QObject::connect((ChefForm *)m_roleForm, &ChefForm::startJacktripSec, this, &MainWindow::startJackTripSecondary);
        qDebug() << "Want to stop JackTrip secondary";
        m_netClient->stopJackTripSec();
        break;
    default:
        break;
    }
    m_sJacktrip->stopThread();
}

void MainWindow::setLocalLoopback(bool l){
    jack_status_t status;
    if (!m_jackClient)
        m_jackClient = jack_client_open ("rcclient", JackNoStartServer, &status);
    if(m_jackClient){
        const char** inports;
        const char** outports;

            // Get physical input (playback) ports
            if ( (inports =
                  jack_get_ports (m_jackClient, NULL, NULL,
                                  JackPortIsPhysical | JackPortIsInput)) == NULL
                 ||
                 (outports =
                                   jack_get_ports (m_jackClient, NULL, NULL,
                                                   JackPortIsPhysical | JackPortIsOutput)) == NULL)
            {
                qDebug() << "WARNING: Cannot find any physical playback ports";
            }
            else
            {
                // Connect playback ports to jacktrip receive
                for (int i = 0; i < 2; i++)
                {
                    // Check that we don't run out of capture ports
                    if ( inports[i] != NULL && outports[i]!=NULL) {
                        if(l){
                        qDebug() <<"Primary: connect" <<inports[i] <<"to"<<outports[i];
                        jack_connect(m_jackClient, outports[i], inports[i]);
                        }
                        else{
                            qDebug() <<"Primary: disconnect" <<inports[i] <<"to"<<outports[i];
                            jack_disconnect(m_jackClient, outports[i], inports[i]);
                        }
                    }
                }
                std::free(inports);
                std::free(outports);
            }
    }

    }

