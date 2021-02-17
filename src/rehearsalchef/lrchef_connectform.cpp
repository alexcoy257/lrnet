#include "lrchef_connectform.h"
#include <QIntValidator>

ConnectForm::ConnectForm(QWidget *parent) : QWidget(parent)
  , m_ksw(new KeySubwidget(this))
  , m_csw(new CodeSubwidget(this))
  , tl_layout(new QVBoxLayout(this))
  , bu_layout(new QHBoxLayout(NULL))
  , kc_layout(new QHBoxLayout(NULL))
  , m_hostBox(new QLineEdit(this))
  , m_portBox(new QLineEdit("4463", this))
  , m_netidBox(new QLineEdit(this))

  , m_authByKeyButton(new QRadioButton("User Login", this))
  , m_lPublicKey(m_ksw->m_lPublicKey)

  , m_authByCodeButton(new QRadioButton("Guest Login", this))

  , m_errLabel(new QLabel(this))
  , m_submitButton(new QPushButton("Connect",this))


{
    m_ksw->hide();
    m_csw->hide();
    m_authByKeyButton->setChecked(true);

    tl_layout->addWidget(m_errLabel);
    m_errLabel->hide();
    tl_layout->insertLayout(0, bu_layout);
    tl_layout->insertLayout(1, kc_layout);
    tl_layout->addWidget(m_ksw);
    tl_layout->addWidget(m_csw);

    bu_layout->addWidget(m_hostBox,0,Qt::AlignLeft);
    bu_layout->addWidget(m_portBox,0,Qt::AlignCenter);
    bu_layout->addWidget(m_netidBox,0,Qt::AlignCenter);
    bu_layout->addWidget(m_submitButton,0,Qt::AlignRight);

    kc_layout->addWidget(m_authByKeyButton,0,Qt::AlignLeft);
    kc_layout->addWidget(m_authByCodeButton,0,Qt::AlignLeft);

    m_portBox->setPlaceholderText("Port");
    m_portBox->setFixedWidth(80);

    m_hostBox->setPlaceholderText("Host");
    m_hostBox->setFixedWidth(225);

    m_netidBox->setPlaceholderText("NetID");
    m_netidBox->setFixedWidth(80);
    m_netidBox->setMaxLength(30);

    m_portBox->setValidator(new QIntValidator(1024, 65535));

    m_submitButton->setFixedWidth(100);


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
//    QObject::connect(m_submitButton, &QAbstractButton::released, this, [=](){if (validInputs()){
//                                                                                 m_usingKey = m_authByKeyButton->isChecked();
//                                                                                 emit tryConnect(m_hostBox->text(), m_portBox->text().toInt());
//                                                                            }});

    QObject::connect(m_submitButton, &QAbstractButton::released, this, [=](){m_usingKey = m_authByKeyButton->isChecked();
                                                                             emit tryConnect(m_hostBox->text(), m_portBox->text().toInt());});

    QObject::connect(m_authByKeyButton, &QAbstractButton::toggled, this, &ConnectForm::setKeyAuth);
    QObject::connect(m_authByCodeButton, &QAbstractButton::toggled, this, &ConnectForm::setCodeAuth);

    QObject::connect(m_csw->m_authCodeBox, &QLineEdit::editingFinished, this, [=](){emit updateCode(m_csw->m_authCodeBox->text());});
    QObject::connect(m_csw->m_authCodeBox, &QLineEdit::returnPressed, this, [=](){emit m_csw->m_authCodeBox->editingFinished();
                                                                                  m_submitButton->click();});

    QObject::connect(m_netidBox, &QLineEdit::editingFinished, this, [=](){emit netidUpdated( m_netidBox->text());});
    QObject::connect(m_netidBox, &QLineEdit::returnPressed, this, [=](){emit m_netidBox->editingFinished();
                                                                        m_submitButton->click();});

    QObject::connect(m_hostBox, &QLineEdit::editingFinished, this, [=](){emit updateHost( m_hostBox->text());});
    QObject::connect(m_hostBox, &QLineEdit::returnPressed, this, [=](){emit m_hostBox->editingFinished();
                                                                       m_submitButton->click();});

    QObject::connect(m_portBox, &QLineEdit::editingFinished, this, [=](){emit updatePort( m_portBox->text().toInt());});
    QObject::connect(m_portBox, &QLineEdit::returnPressed, this, [=](){emit m_portBox->editingFinished();
                                                                       m_submitButton->click();});

}

ConnectForm::~ConnectForm(){
}

void ConnectForm::setKeyAuth(bool checked){
    if (!checked)
        return;
//    m_ksw->show();
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

bool ConnectForm::validInputs(){
    bool valid = true;
    if (m_hostBox->text().isEmpty())
        valid = false;

    if (m_portBox->text().isEmpty())
        valid = false;

    if (m_netidBox->text().isEmpty())
        valid = false;

    return valid;
}
