#ifndef CONNECTFORM_H
#define CONNECTFORM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QTimer>
#include <QSettings>
#include <QPlainTextEdit>


#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/err.h>


#include "RehearsalChefabout.h"

#include "../lrnetclient/lrnetclient.h"

class ConnectForm : public QWidget
{
    Q_OBJECT
    friend class MainWindow;

    class KeySubwidget : public QWidget{
        friend class ConnectForm;
        QBoxLayout * key_layout;
        QPlainTextEdit * m_lPublicKey;
        QPushButton * m_keyCopyGenButton;
    public:
        explicit KeySubwidget(QWidget * parent): QWidget(parent)
          , key_layout(new QHBoxLayout(this))
          , m_lPublicKey(new QPlainTextEdit(this))
          , m_keyCopyGenButton(new QPushButton(this))
        {
            setLayout(key_layout);
            m_keyCopyGenButton->setText("Copy Public Key");
            key_layout->addWidget(m_lPublicKey);
            key_layout->addWidget(m_keyCopyGenButton);
        };

    };

    class CodeSubwidget : public QWidget{
        friend class ConnectForm;
        QBoxLayout * code_layout;
        QLineEdit * m_authCodeBox;
    public:
        explicit CodeSubwidget(QWidget * parent): QWidget(parent)
        , code_layout(new QHBoxLayout(this))
        , m_authCodeBox(new QLineEdit(NULL))
        {
            setLayout(code_layout);
            code_layout->addWidget(m_authCodeBox);

        }

    };

    KeySubwidget * m_ksw;
    CodeSubwidget * m_csw;


    QBoxLayout * tl_layout;
    QBoxLayout * bu_layout;
    QBoxLayout * kc_layout;

    QLineEdit * m_hostBox;
    QLineEdit * m_portBox;
    QLineEdit * m_netidBox;

    QRadioButton * m_authByKeyButton;
    QPlainTextEdit * m_lPublicKey;

    QRadioButton * m_authByCodeButton;

    QLabel * m_errLabel;
    QPushButton * m_submitButton;

private slots:
    void setKeyAuth(bool checked);
    void setCodeAuth(bool checked);

public:
    explicit ConnectForm(QWidget *parent = nullptr);
    ~ConnectForm();

signals:
    void tryConnect(const QString host, int port);
    void setToKeyAuth();
    void setToCodeAuth();
    void updateCode(const QString & code);
    void netidUpdated(const QString & netid);

};

#endif // CONNECTFORM_H
