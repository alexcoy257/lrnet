#include "memberform.h"
#include "ui_memberform.h"

MemberForm::MemberForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MemberForm)
  ,m_chatForm(new ChatForm(this))
{
    ui->setupUi(this);
    ui->chatArea->addWidget(m_chatForm);
}

MemberForm::~MemberForm()
{
    delete ui;
    delete m_chatForm;
}
