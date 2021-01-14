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
    QObject::connect(ui->redundancyChoice, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int nv){emit changeRedundancy(nv);});
    //QObject::connect(ui->redundancyChoice, &QSpinBox::textChanged, this, [=](QString s){emit changeRedundancy(s.toInt());});

    QObject::connect(ui->jackServer, &JackParameterForm::jackStarted, this, [=](){ui->startJackTripButton->setEnabled(true);});
    QObject::connect(ui->jackServer, &JackParameterForm::jackStopped, this, [=](){ui->startJackTripButton->setDisabled(true);});
    QObject::connect(ui->encryptEnabledBox, &QCheckBox::stateChanged, this, [=](int e){emit setEncryption(e);});
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
}

void MemberForm::updateName(){
    ui->nameChoice->clearFocus();
    emit nameUpdated(ui->nameChoice->text());
}

void MemberForm::fstartJacktrip(){
    emit startJackTrip();
    ui->startJackTripButton->setText("Stop JackTrip");
    ui->redundancyChoice->setDisabled(true);
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstopJacktrip);
}

void MemberForm::fstopJacktrip(){
    emit stopJackTrip();
    ui->startJackTripButton->setText("Start JackTrip");
    ui->redundancyChoice->setEnabled(true);
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstopJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
}

MemberForm::~MemberForm()
{
    QObject::disconnect(ui->nameChoice, &QLineEdit::editingFinished, this, &MemberForm::updateName);
    delete ui;
    delete m_chatForm;
}

void MemberForm::setName(const QString & nname){
    ui->nameChoice->setText(nname);
    updateName();
}

void MemberForm::setSection(const QString & nsection){
    ui->sectionChoice->setCurrentIndex(0);
    ui->sectionChoice->setCurrentText(nsection);
    emit sectionUpdated(ui->sectionChoice->currentText());
}

void MemberForm::loadSetup(QSettings &settings){
    settings.beginGroup("/Member");
    setName(settings.value("Name","").toString());
    setSection(settings.value("Section","").toString());
    settings.endGroup();

    ui->jackServer->loadSetup(settings);

}

void MemberForm::saveSetup(QSettings &settings){
    settings.beginGroup("/Member");
    settings.setValue("Name", ui->nameChoice->text());
    settings.setValue("Section", ui->sectionChoice->currentText());
    settings.endGroup();

    ui->jackServer->saveSetup(settings);

    settings.sync();
}

void MemberForm::disableJackForm(){
    ui->jackServer->setDisabled(true);
}

void MemberForm::enableJackForm(){
    ui->jackServer->setEnabled(true);
}
