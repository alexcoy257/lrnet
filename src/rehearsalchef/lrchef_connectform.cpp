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
}


