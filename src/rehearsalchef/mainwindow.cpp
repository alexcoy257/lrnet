#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_actComp(NULL)
    , m_connectForm(new ConnectForm(NULL))
    , m_openConnectFormButton(new QPushButton(NULL))
    , m_settings(REHEARSALCHEF_DOMAIN, REHEARSALCHEF_TITLE)
    , m_netClient(new LRNetClient())
    , m_keepAliveTimer(this)
    , m_privateKey({})
    , m_publicKey({})
    , keyPair(NULL)
{
    ui->setupUi(this);
    //ui->m_channelStripArea->addStretch();
    ui->m_channelStripArea->addStretch();
    ui->m_rosterArea->addWidget(m_openConnectFormButton);



    QObject::connect(m_openConnectFormButton, &QAbstractButton::released, m_connectForm, &QWidget::show);
    QObject::connect(m_connectForm, &ConnectForm::tryConnect, m_netClient, &LRNetClient::tryConnect);
    QObject::connect(m_netClient, &LRNetClient::timeout, this, [=](){qDebug()<<"Timeout";});
    QObject::connect(m_netClient, &LRNetClient::connected, this,
                     [=](){
        qDebug()<<"Connected";
    });
    QObject::connect(m_netClient, &LRNetClient::newMember, this, &MainWindow::addChannelStrip);
    QObject::connect(m_netClient, &LRNetClient::lostMember, this, &MainWindow::deleteChannelStrip);

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

}

void MainWindow::highlightInsert(Compressor * cp){
    if(m_actComp){
        m_actComp->hide();
        ui->m_actCompArea->removeWidget(m_actComp);
    }


    ui->m_actCompArea->addWidget(cp);
    m_actComp = cp;
    m_actComp->show();
}

void MainWindow::addChannelStrip(const QString& mName, const QString& sName, int id){

    if (! m_clients.contains(id)){
        LRMClient * cStruct = (LRMClient *)malloc(sizeof(LRMClient));
        cStruct->cs = new ChannelStrip(this, mName);

        cStruct->comp = new Compressor(this);
        cStruct->comp->hide();
        qDebug() <<"Widgets present: " << ui->m_channelStripArea->count() <<"\n";
        ui->m_channelStripArea->insertWidget(ui->m_channelStripArea->count()-1,cStruct->cs);
        //ui->m_channelStripArea->addWidget(cStruct->cs, 0, Qt::AlignLeft);
        //cStruct->cs->show();
        QObject::connect(cStruct->cs, &ChannelStrip::setActive, this, [=](){highlightInsert(cStruct->comp);}, Qt::QueuedConnection);
        m_clients[id] = cStruct;
    }
}

void MainWindow::deleteChannelStrip(int id){

    if (m_clients.contains(id)){
        delete m_clients[id]->cs;
        delete m_clients[id]->comp;
        m_clients.remove(id);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_connectForm;
    RSA_free(keyPair);
    saveSetup();
}

void MainWindow::keyInit(){
    loadSetup();

    if (m_publicKey.length() == 0){
        RSA * t_keyPair = RSA_new();
        BIGNUM * e = NULL;
        BN_dec2bn(&e, "3");
        RSA_generate_key_ex(keyPair, 2048,e, NULL);
        //m_keyCopyGenButton->setText("Generate Public Key");
        BN_free(e);

        BIO * t_pri = BIO_new(BIO_s_mem());
        BIO * t_pub = BIO_new(BIO_s_mem());
        PEM_write_bio_RSAPrivateKey(t_pri, keyPair, NULL, NULL, 0, NULL, NULL);
        PEM_write_bio_RSA_PUBKEY(t_pub, keyPair);
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
        m_connectForm->m_keyCopyGenButton->setText("Copy Public Key");
    }

    BIO * t_pri = BIO_new(BIO_s_mem());
    BIO * t_pub = BIO_new(BIO_s_mem());

    keyPair = RSA_new();

    qDebug() <<"Public key length " <<m_publicKey.length();
    BIO_write(t_pub, m_publicKey.data(), m_publicKey.length());
    RSA * trsa = PEM_read_bio_RSA_PUBKEY(t_pub, &keyPair, NULL, NULL);

    if (!trsa){
        qDebug() <<"RSA read failed somehow";
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

}

void MainWindow::loadSetup(){
    m_settings.beginGroup("/Auth");
    m_publicKey = m_settings.value("PublicKey","").toByteArray();
    m_privateKey = m_settings.value("PrivateKey","").toByteArray();
    qDebug() <<"Public key: " <<m_publicKey;
    m_settings.endGroup();
}

void MainWindow::saveSetup(){
    m_settings.beginGroup("/Auth");
    m_settings.setValue("PublicKey",m_publicKey);
    m_settings.setValue("PrivateKey",m_privateKey);
    m_settings.endGroup();
    m_settings.sync();
}

