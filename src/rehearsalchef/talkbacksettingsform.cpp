#include "talkbacksettingsform.h"
#include "ui_talkbacksettingsform.h"

TalkbackSettingsForm::TalkbackSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TalkbackSettingsForm)
{
    ui->setupUi(this);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &TalkbackSettingsForm::fStartJacktrip);
    QObject::connect(ui->jtSelfLoopbackBox, &QCheckBox::stateChanged, this, [=](int e){emit setjtSelfLoopback(e);});

    QObject::connect(ui->jackServer, &JackParameterForm::jackStarted, this, &TalkbackSettingsForm::enableJackTripButton);
    QObject::connect(ui->jackServer, &JackParameterForm::jackStopped, this, &TalkbackSettingsForm::disableJackTripButton);


}

TalkbackSettingsForm::~TalkbackSettingsForm()
{
    QObject::disconnect(ui->jackServer, &JackParameterForm::jackStarted, this, &TalkbackSettingsForm::enableJackTripButton);
    QObject::disconnect(ui->jackServer, &JackParameterForm::jackStopped, this, &TalkbackSettingsForm::disableJackTripButton);
    delete ui;
}

void TalkbackSettingsForm::enableJackTripButton(){
    emit jackStarted();
    ui->startJackTripButton->setEnabled(true);
}

void TalkbackSettingsForm::disableJackTripButton(){
    emit jackStopped();
    ui->startJackTripButton->setDisabled(true);
}

void TalkbackSettingsForm::fStartJacktrip(){
    emit startJackTrip();
    ui->startJackTripButton->setText("Stop JackTrip");
    ui->redundancyChoice->setDisabled(true);
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &TalkbackSettingsForm::fStartJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &TalkbackSettingsForm::fStopJacktrip);
}

void TalkbackSettingsForm::fStopJacktrip(){
    emit stopJackTrip();
    ui->startJackTripButton->setText("Start JackTrip");
    ui->redundancyChoice->setEnabled(true);
    QObject::disconnect(ui->startJackTripButton, &QAbstractButton::released, this, &TalkbackSettingsForm::fStopJacktrip);
    QObject::connect(ui->startJackTripButton, &QAbstractButton::released, this, &TalkbackSettingsForm::fStartJacktrip);
}

void TalkbackSettingsForm::disableJackForm(){
    if (ui->jackServer)
    ui->jackServer->setDisabled(true);
}

void TalkbackSettingsForm::enableJackForm(){
    if (ui->jackServer)
    ui->jackServer->setEnabled(true);
}

void TalkbackSettingsForm::loadSetup(QSettings & settings){
    ui->jackServer->loadSetup(settings);
}

void TalkbackSettingsForm::saveSetup(QSettings & settings){
    ui->jackServer->saveSetup(settings);
}

bool TalkbackSettingsForm::getjtSelfLoopback(){
    return ui->jtSelfLoopbackBox->isChecked();
}
