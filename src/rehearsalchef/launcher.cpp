#include "launcher.h"

Launcher::Launcher(AuthTypeE level, QWidget *parent) : QWidget(parent)
  ,m_layout(new QHBoxLayout(this))
  ,m_superChefButton(new QPushButton("Super Chef", this))
  ,m_chefButton(new QPushButton("Chef", this))
  ,m_memberButton(new QPushButton("Member", this))
{
    setLayout(m_layout);
    m_layout->addWidget(m_superChefButton);
    m_layout->addWidget(m_chefButton);
    m_layout->addWidget(m_memberButton);

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
