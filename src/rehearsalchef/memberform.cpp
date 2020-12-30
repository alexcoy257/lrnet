#include "memberform.h"
#include "ui_memberform.h"
#include <QDebug>

MemberForm::MemberForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MemberForm)
  ,m_chatForm(new ChatForm(this))
{
    ui->setupUi(this);
    ui->chatArea->addWidget(m_chatForm);
    ui->chatArea->addWidget(new QLabel("test"));
    m_chatForm->setVisible(true);
    m_chatForm->show();
    QObject::connect(ui->nameChoice, &QLineEdit::editingFinished, this, [=](){emit nameUpdated(ui->nameChoice->text());
    qDebug()<<"Name updated";});
    QObject::connect(ui->sectionChoice, &QComboBox::currentTextChanged, this, [=](){emit sectionUpdated(ui->sectionChoice->currentText());});
}

MemberForm::~MemberForm()
{
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
