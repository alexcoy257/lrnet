#include "lrchef_connectform.h"
#include <QIntValidator>

ConnectForm::ConnectForm(QWidget *parent) : QWidget(parent)
  , tl_layout(new QVBoxLayout(this))
  , bu_layout(new QHBoxLayout(NULL))
  , key_layout(new QHBoxLayout(NULL))
  , m_hostBox(new QLineEdit("localhost", this))
  , m_portBox(new QLineEdit("4463", this))
  , m_errLabel(new QLabel(this))
  , m_submitButton(new QPushButton("Connect",this))
  , m_settings(REHEARSALCHEF_DOMAIN, REHEARSALCHEF_TITLE)
  , m_lPublicKey(new QPlainTextEdit(this))
  , m_keyCopyGenButton(new QPushButton(this))
{
    tl_layout->addWidget(m_errLabel);
    m_errLabel->hide();
    tl_layout->insertLayout(0, bu_layout);
    tl_layout->insertLayout(1, key_layout);
    bu_layout->addWidget(m_hostBox);
    bu_layout->addWidget(m_portBox);
    bu_layout->addWidget(m_submitButton);
    key_layout->addWidget(m_lPublicKey);
    key_layout->addWidget(m_keyCopyGenButton);
    m_portBox->setPlaceholderText("Port");
    m_hostBox->setPlaceholderText("Server");
    m_portBox->setValidator(new QIntValidator(1024, 65535));
    loadSetup();

    if (m_publicKey.length() == 0){
        RSA * keyPair = RSA_new();
        BIGNUM * e = NULL;
        BN_dec2bn(&e, "3");
        RSA_generate_key_ex(keyPair, 2048,e, NULL);
        m_keyCopyGenButton->setText("Generate Public Key");
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
        RSA_free(keyPair);
    }
    else{
        m_lPublicKey->setPlainText(m_publicKey);
        m_keyCopyGenButton->setText("Copy Public Key");
    }

    BIO * t_pri = BIO_new(BIO_s_mem());
    BIO * t_pub = BIO_new(BIO_s_mem());

    RSA * keyPair = RSA_new();

    qDebug() <<"Public key length " <<m_publicKey.length();
    BIO_write(t_pub, m_publicKey.data(), m_publicKey.length());
    PEM_read_bio_RSAPublicKey(t_pub, &keyPair, NULL, NULL);

    BIO_write(t_pri, m_privateKey.data(), m_privateKey.length());
    PEM_read_bio_RSAPrivateKey(t_pri, &keyPair, NULL, NULL);

    BIO_free(t_pri);
    BIO_free(t_pub);

    if (RSA_check_key(keyPair)){
        qDebug() <<"Key valid";
    }
    else{
        qDebug() <<"Key invalid";
    }



    //QObject::connect(m_portBox, &QLineEdit::inputRejected, this, [=](){qDebug()<<"Input Rejected";});
    QObject::connect(m_portBox, &QLineEdit::textEdited, this,
                     [=](){
                        if(!m_portBox->hasAcceptableInput()){
                        m_errLabel->setText("Hint: port should be between 1024 and 65535");
                        m_errLabel->show();
                        m_submitButton->setDisabled(true);}
                           }
    );
    QObject::connect(m_portBox, &QLineEdit::editingFinished, this, [=](){m_submitButton->setEnabled(true);m_errLabel->hide();});
    QObject::connect(m_submitButton, &QAbstractButton::released, this, [=](){emit tryConnect(m_hostBox->text(), m_portBox->text().toInt());});
}

ConnectForm::~ConnectForm(){
    saveSetup();
}

void ConnectForm::loadSetup(){
    m_settings.beginGroup("/Auth");
    m_publicKey = m_settings.value("PublicKey","").toByteArray();
    m_privateKey = m_settings.value("PrivateKey","").toByteArray();
    qDebug() <<"Public key: " <<m_publicKey;
    m_settings.endGroup();
}

void ConnectForm::saveSetup(){
    m_settings.beginGroup("/Auth");
    m_settings.setValue("PublicKey",m_publicKey);
    m_settings.setValue("PrivateKey",m_privateKey);
    m_settings.endGroup();
    m_settings.sync();
}
