#ifndef CONNECTFORM_H
#define CONNECTFORM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSettings>
#include <QPlainTextEdit>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>


#include "RehearsalChefabout.h"

#include "../lrnetclient/lrnetclient.h"

class ConnectForm : public QWidget
{
    Q_OBJECT

    QBoxLayout * tl_layout;
    QBoxLayout * bu_layout;
    QBoxLayout * key_layout;
    QLineEdit * m_hostBox;
    QLineEdit * m_portBox;
    QLabel * m_errLabel;
    QPushButton * m_submitButton;
    QSettings m_settings;


    QByteArray m_privateKey;
    QByteArray m_publicKey;
    QPlainTextEdit * m_lPublicKey;
    QPushButton * m_keyCopyGenButton;

    void loadSetup();
    void saveSetup();

public:
    explicit ConnectForm(QWidget *parent = nullptr);
    ~ConnectForm();

signals:
    void tryConnect(const QString host, int port);

};

#endif // CONNECTFORM_H
