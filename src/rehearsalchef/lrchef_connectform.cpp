#include "lrchef_connectform.h"
#include <QIntValidator>

ConnectForm::ConnectForm(QWidget *parent) : QWidget(parent)
  , m_ksw(new KeySubwidget(this))
  , m_csw(new CodeSubwidget(this))
  , tl_layout(new QVBoxLayout(this))
  , bu_layout(new QHBoxLayout(NULL))
  , kc_layout(new QHBoxLayout(NULL))
  , m_hostBox(new QLineEdit("localhost", this))
  , m_portBox(new QLineEdit("4463", this))
  , m_netidBox(new QLineEdit(this))

  , m_authByKeyButton(new QRadioButton("Key Authorization", this))
  , m_lPublicKey(m_ksw->m_lPublicKey)

  , m_authByCodeButton(new QRadioButton("Code Authorization", this))

  , m_errLabel(new QLabel(this))
  , m_submitButton(new QPushButton("Connect",this))


{
    m_ksw->show();
    m_csw->hide();
    m_authByKeyButton->setChecked(true);

    tl_layout->addWidget(m_errLabel);
    m_errLabel->hide();
    tl_layout->insertLayout(0, bu_layout);
    tl_layout->insertLayout(1, kc_layout);
    tl_layout->addWidget(m_ksw);
    tl_layout->addWidget(m_csw);

    bu_layout->addWidget(m_hostBox);
    bu_layout->addWidget(m_portBox);
    bu_layout->addWidget(m_netidBox);
    bu_layout->addWidget(m_submitButton);

    kc_layout->addWidget(m_authByKeyButton);
    kc_layout->addWidget(m_authByCodeButton);

    m_portBox->setPlaceholderText("Port");
    m_hostBox->setPlaceholderText("Server");
    m_netidBox->setPlaceholderText("netid");
    m_portBox->setValidator(new QIntValidator(1024, 65535));


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

    QObject::connect(m_authByKeyButton, &QAbstractButton::toggled, this, &ConnectForm::setKeyAuth);
    QObject::connect(m_authByCodeButton, &QAbstractButton::toggled, this, &ConnectForm::setCodeAuth);

    QObject::connect(m_csw->m_authCodeBox, &QLineEdit::editingFinished, this, [=](){emit updateCode(m_csw->m_authCodeBox->text());});
    QObject::connect(m_netidBox, &QLineEdit::editingFinished, this, [=](){emit netidUpdated( m_netidBox->text());});

}

ConnectForm::~ConnectForm(){
}

void ConnectForm::setKeyAuth(bool checked){
    if (!checked)
        return;
    m_ksw->show();
    m_csw->hide();
    emit setToKeyAuth();
}

void ConnectForm::setCodeAuth(bool checked){
   if (!checked)
       return;
   m_ksw->hide();
   m_csw->show();
   emit setToCodeAuth();
}


