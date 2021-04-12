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
    QObject::connect(ui->sentChannelsChoice, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int nv){emit setNumChannels(nv);});
    //QObject::connect(ui->redundancyChoice, &QSpinBox::textChanged, this, [=](QString s){emit changeRedundancy(s.toInt());});

    QObject::connect(ui->jackServer, &JackParameterForm::jackStarted, this, &MemberForm::enableJackTripButton);
    QObject::connect(ui->jackServer, &JackParameterForm::jackStopped, this, &MemberForm::disableJackTripButton);
    QObject::connect(ui->encryptEnabledBox, &QCheckBox::stateChanged, this, [=](int e){emit setEncryption(e);});
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
    QObject::connect(ui->muteButton, &QAbstractButton::released, this, &MemberForm::toggleMute);
    QObject::connect(ui->jtSelfLoopbackBox, &QCheckBox::stateChanged, this, &MemberForm::setjtSelfLoopback);
    QObject::connect(ui->localLoopbackBox, &QCheckBox::stateChanged, this, [=](int e){emit setLocalLoopback(e);});

    ui->jtSelfLoopbackBox->hide();
    ui->jtSelfLoopbackInfoLabel->hide();

    // This section should be removed as functionality is implemented
    ui->sectionLabel->hide();
    ui->sectionChoice->hide();
    ui->tcpAudioButton->hide();
    ui->udpAudioButton->hide();
    //
}

void MemberForm::updateName(){
    ui->nameChoice->clearFocus();
    emit nameUpdated(ui->nameChoice->text());
}

void MemberForm::enableJackTripButton(){
    ui->startJackTripButton->setEnabled(true);
}

void MemberForm::disableJackTripButton(){
    ui->startJackTripButton->setDisabled(true);
}

void MemberForm::fstartJacktrip(){
    emit startJackTrip();
    ui->startJackTripButton->setText("Stop JackTrip");
    ui->redundancyChoice->setDisabled(true);
    ui->sentChannelsChoice->setDisabled(true);
    ui->jtSelfLoopbackBox->show();
    ui->jtSelfLoopbackInfoLabel->show();
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstopJacktrip);
}

void MemberForm::fstopJacktrip(){
    emit stopJackTrip();
    ui->startJackTripButton->setText("Start JackTrip");
    ui->redundancyChoice->setEnabled(true);
    ui->sentChannelsChoice->setEnabled(true);
    QObject::disconnect(ui->jtSelfLoopbackBox, &QCheckBox::stateChanged, this, &MemberForm::setjtSelfLoopback);
    ui->jtSelfLoopbackBox->setChecked(false);
    ui->jtSelfLoopbackBox->hide();
    ui->jtSelfLoopbackInfoLabel->hide();
    QObject::connect(ui->jtSelfLoopbackBox, &QCheckBox::stateChanged, this, &MemberForm::setjtSelfLoopback);
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstopJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &MemberForm::fstartJacktrip);
}

void MemberForm::handleJackPortsConnected(){
    emit doMute(muted);
}

MemberForm::~MemberForm()
{
    QObject::disconnect(ui->nameChoice, &QLineEdit::editingFinished, this, &MemberForm::updateName);
    QObject::disconnect(ui->jackServer, &JackParameterForm::jackStarted, this, &MemberForm::enableJackTripButton);
    QObject::disconnect(ui->jackServer, &JackParameterForm::jackStopped, this, &MemberForm::disableJackTripButton);
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

    emit setNumChannels(1);

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
    if (ui->jackServer)
    ui->jackServer->setDisabled(true);
}

void MemberForm::enableJackForm(){
    if (ui->jackServer)
    ui->jackServer->setEnabled(true);
}

void MemberForm::toggleMute(){
    muted = !muted;
    if(muted){
        ui->muteButton->setText("Unmute");
        emit doMute(true);
    }
    else{
        ui->muteButton->setText("Mute");
        emit doMute(false);
    }
    emit sendClientMute(muted);
}
