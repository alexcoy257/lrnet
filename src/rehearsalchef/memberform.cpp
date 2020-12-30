#include "memberform.h"
#include "ui_memberform.h"
#include <QDebug>

MemberForm::MemberForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MemberForm)
  ,m_chatForm(new ChatForm(this))
  //,m_jackForm(new JackParameterForm(this))
{
    ui->setupUi(this);
    ui->chatArea->addWidget(m_chatForm);
    //ui->mainGridLayout->addWidget();
    QObject::connect(ui->nameChoice, &QLineEdit::editingFinished, this, &MemberForm::updateName);
    QObject::connect(ui->sectionChoice, &QComboBox::currentTextChanged, this, [=](){emit sectionUpdated(ui->sectionChoice->currentText());});
}

void MemberForm::updateName(){
    ui->nameChoice->clearFocus();
    emit nameUpdated(ui->nameChoice->text());
}

MemberForm::~MemberForm()
{
    QObject::disconnect(ui->nameChoice, &QLineEdit::editingFinished, this, &MemberForm::updateName);
    delete ui;
    delete m_chatForm;
}

void MemberForm::setName(const QString & nname){
    ui->nameChoice->setText(nname);
}

void MemberForm::setSection(const QString & nsection){
    ui->sectionChoice->setCurrentIndex(0);
    ui->sectionChoice->setCurrentText(nsection);
}
