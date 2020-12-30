#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chefForm(NULL)
    , m_connectForm(new ConnectForm(this))
    , m_settings(REHEARSALCHEF_DOMAIN, REHEARSALCHEF_TITLE)
    , m_netClient(new LRNetClient())
    , m_keepAliveTimer(this)
    , m_privateKey({})
    , m_publicKey({})
    , keyPair(NULL)
    , m_jacktrip( new RCJTWorker(this, 10, JackTrip::ZEROS, ""))
{
    ui->setupUi(this);

    setCentralWidget(m_connectForm);


    QObject::connect(m_connectForm, &ConnectForm::tryConnect, this, &MainWindow::tryConnect);

    QObject::connect(m_connectForm, &ConnectForm::updateCode, this, [=](const QString & code){
        m_netClient->setTempCode(code);
        QString base = "Updated code to: %1";
        statusBar()->showMessage(base.arg(code));
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
    QObject::connect(m_netClient, &LRNetClient::authenticated, this, &MainWindow::handleAuth);
    QObject::connect(m_netClient, &LRNetClient::authFailed, this, [=](){statusBar()->showMessage("Login failed");});
    QObject::connect(m_connectForm, &ConnectForm::netidUpdated, this, [&](const QString & nnetid){m_netid = nnetid; m_netClient->setNetid(nnetid);});




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
}





MainWindow::~MainWindow()
{
    m_jacktrip->stopThread();
    delete ui;
    delete m_netClient;
    //delete m_connectForm;
    RSA_free(keyPair);
    saveSetup();
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

     m_settings.beginGroup("/Names");
     m_name = m_settings.value("name","").toString();
     m_section = m_settings.value("section","").toString();
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

    m_settings.beginGroup("/Names");
    m_settings.setValue("name",m_name);
    m_settings.setValue("section",m_section);
    m_settings.endGroup();

    m_settings.sync();
}

void MainWindow::handleAuth(AuthTypeE type){
    if (type != NONE){
        setCentralWidget(new Launcher(type, this));
        connect((Launcher *)centralWidget(), &Launcher::choseSuperChef, this, &MainWindow::launchSuperChef);
        connect((Launcher *)centralWidget(), &Launcher::choseChef, this, &MainWindow::launchChef);
        connect((Launcher *)centralWidget(), &Launcher::choseMember, this, &MainWindow::launchMember);
    }
}

void MainWindow::launchSuperChef(){
qDebug()<<"Chose superchef";
m_netClient->subSuperchef();
}

void MainWindow::launchChef(){
setCentralWidget(new ChefForm(this));
QObject::connect(m_netClient, &LRNetClient::newMember, (ChefForm *)centralWidget(), &ChefForm::addChannelStrip);
QObject::connect(m_netClient, &LRNetClient::updateMember, (ChefForm *)centralWidget(), &ChefForm::updateChannelStrip);
QObject::connect(m_netClient, &LRNetClient::lostMember, (ChefForm *)centralWidget(), &ChefForm::deleteChannelStrip);
m_netClient->subChef();
}

void MainWindow::launchMember(){
setCentralWidget(new MemberForm(this));
((MemberForm *)centralWidget())->setName(m_name);
((MemberForm *)centralWidget())->setSection(m_section);
QObject::connect((MemberForm *)centralWidget(), &MemberForm::nameUpdated, this, [=](const QString nname){
    m_name = nname;
    m_netClient->updateName(nname);
});
QObject::connect((MemberForm *)centralWidget(), &MemberForm::sectionUpdated, this, [=](const QString nsection){
    m_section = nsection;
    m_netClient->updateSection(nsection);
});
m_netClient->subMember();

QObject::connect((MemberForm *)centralWidget(), &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);

QObject::connect(m_netClient, &LRNetClient::gotUdpPort, this, &MainWindow::setUdpPort);

}

void MainWindow::setUdpPort(int port){
    m_jacktrip->setJackTrip(m_hostname, port, port, 2, true);
}



void MainWindow::startJackTrip(){
    QObject::disconnect((MemberForm *)centralWidget(), &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);
    qDebug() << "Want to start JackTrip";
    m_netClient->startJackTrip();
    QObject::connect(m_netClient, &LRNetClient::serverJTReady, this, &MainWindow::startJackTripThread);

}

void MainWindow::startJackTripThread(){
    m_jacktripthreadpool.start(m_jacktrip, QThread::TimeCriticalPriority);
}


void MainWindow::releaseThread(int n){
    QObject::connect((MemberForm *)centralWidget(), &MemberForm::startJackTrip, this, &MainWindow::startJackTrip);
}
