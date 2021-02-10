#include "launcher.h"
#include <QDebug>

Launcher::Launcher(AuthTypeE level, bool usingKey, QWidget *parent) : QWidget(parent)
  ,m_mainLayout(new QVBoxLayout(this))
  ,m_userLayout(new QHBoxLayout)
  ,m_sendKeyArea(new QHBoxLayout)
  ,m_superChefButton(new QPushButton("Super Chef", this))
  ,m_chefButton(new QPushButton("Chef", this))
  ,m_memberButton(new QPushButton("Member", this))
{
    setLayout(m_mainLayout);
    m_mainLayout->addLayout(m_userLayout);

    m_userLayout->addWidget(m_superChefButton);
    m_userLayout->addWidget(m_chefButton);
    m_userLayout->addWidget(m_memberButton);

    if (!usingKey){
        m_sendKeyButton = new QPushButton("Remember Me", this);
        m_mainLayout->addLayout(m_sendKeyArea);
        m_sendKeyArea->addSpacerItem(new QSpacerItem(40,20,QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_sendKeyArea->addWidget(m_sendKeyButton);
        m_sendKeyArea->addSpacerItem(new QSpacerItem(40,20,QSizePolicy::Expanding, QSizePolicy::Expanding));
        connect(m_sendKeyButton, &QAbstractButton::released, this, [=](){emit sendPublicKey();});
    }

    switch(level){
    case NONE:
    m_memberButton->hide();
    case MEMBER:
    m_chefButton->hide();
    case CHEF:
    m_superChefButton->hide();
    case SUPERCHEF:
    break;
    }

    connect(m_superChefButton, &QAbstractButton::released, this, [=](){emit choseSuperChef();});
    connect(m_chefButton, &QAbstractButton::released, this, [=](){emit choseChef();});
    connect(m_memberButton, &QAbstractButton::released, this, [=](){emit choseMember();});
}

void Launcher::storeKeyResultReceived(bool success){
    m_sendKeyButton->setDisabled(success);
}
