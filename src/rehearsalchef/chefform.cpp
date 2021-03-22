#include "chefform.h"
#include "ui_chefform.h"

ChefForm::ChefForm(QWidget *parent) :
    QWidget(parent),
    m_tbSetupForm(new TalkbackSettingsForm(NULL)),
    m_csAreaLayout(new QHBoxLayout(NULL)),
    ui(new Ui::ChefForm)

{
    ui->setupUi(this);
    m_chatForm = ui->m_chatForm;
    //ui->m_channelStripArea->addStretch();
    ui->m_channelStripScrollParent->setLayout(m_csAreaLayout);
    m_csAreaLayout->addStretch();
    ui->secondaryConnectButton->setEnabled(false);
    //ui->chatArea->addWidget(m_chatForm);

    // Remove this when implemented
    ui->unmuteAllButton->hide();
    //


    QObject::connect(ui->authCodeEdit, &QLineEdit::editingFinished, this, &ChefForm::updateAuthCode);
    QObject::connect(ui->codeEnabledBox, &QCheckBox::stateChanged, this, &ChefForm::updateAuthCodeEnabled);
    QObject::connect(ui->tbSetupButton, &QAbstractButton::released, m_tbSetupForm, &QWidget::show);
    QObject::connect(ui->joinMutedBox, &QAbstractButton::toggled, this, &ChefForm::sendJoinMutedUpdate);
    QObject::connect(m_tbSetupForm, &TalkbackSettingsForm::startJackTrip, this, [=](){emit startJackTrip();});
    QObject::connect(m_tbSetupForm, &TalkbackSettingsForm::stopJackTrip, this, [=](){emit stopJackTrip();});
    QObject::connect(m_tbSetupForm, &TalkbackSettingsForm::setjtSelfLoopback, this, &ChefForm::setjtSelfLoopback);
    QObject::connect(ui->muteButton, &QAbstractButton::released, this, &ChefForm::toggleMute);
    QObject::connect(ui->muteAllButton, &QAbstractButton::clicked, this, &ChefForm::muteAll);
    QObject::connect(m_tbSetupForm, &TalkbackSettingsForm::jackStarted, this, [=](){ui->secondaryConnectButton->setEnabled(true);});
    QObject::connect(m_tbSetupForm, &TalkbackSettingsForm::jackStopped, this, [=](){ui->secondaryConnectButton->setEnabled(false);});
    QObject::connect(ui->secondaryConnectButton, &QAbstractButton::released, this, &ChefForm::fstartJacktripSec);

}

ChefForm::~ChefForm()
{
    delete ui;
    delete m_tbSetupForm;
    //delete m_chatForm;
}

void ChefForm::addChannelStrip(const QString& mName, const QString& sName, QVector<float> controls, int id){

    if (! m_clients.contains(id)){
        LRMClient * cStruct = (LRMClient *)malloc(sizeof(LRMClient));
        cStruct->cs = new ChannelStrip(cStruct, this, mName);

        cStruct->comp = new Compressor(cStruct, this);
        cStruct->comp->hide();
        cStruct->id = id;
        qDebug() <<"Controls: " <<controls;
        cStruct->cs->newControls(controls);
        //qDebug() <<"Widgets present: " << ui->m_channelStripArea->count();
        //ui->m_channelStripArea->insertWidget(ui->m_channelStripArea->count()-1,cStruct->cs);
        m_csAreaLayout->insertWidget(m_csAreaLayout->count()-1,cStruct->cs);
        //ui->m_channelStripArea->addWidget(cStruct->cs, 0, Qt::AlignLeft);
        //cStruct->cs->show();
        QObject::connect(cStruct->cs, &ChannelStrip::setActive, this, [=](){highlightInsert(cStruct->comp);}, Qt::QueuedConnection);
        QObject::connect(cStruct->comp, &Compressor::valueChanged, this, &ChefForm::newValueHandler);
        QObject::connect(cStruct->cs, &ChannelStrip::valueChanged, this, &ChefForm::newValueHandler);
        QObject::connect(cStruct->cs, &ChannelStrip::requestSolo, this, &ChefForm::soloRequested);
        m_clients[id] = cStruct;
    }
}

void ChefForm::newValueHandler(LRMClient * cStruct, int type, float value){
    cStruct->cs->setControl((int)type, value);
    QVector<float> ftmp = cStruct->cs->getCurrentControls();
    emit sendControlUpdate(cStruct->id, ftmp);
}

void ChefForm::updateChannelStrip(const QString& mName, const QString& sName, int id){
    LRMClient * client = m_clients[id];
    if(client){
        client ->cs->setName(mName);
        client->cs->setSection(sName);
    }
}

void ChefForm::updateChannelStripControls(QVector<float> &controls, int id){
    LRMClient * client = m_clients[id];
    if (client){
        client->cs->newControls(controls);
    }
}

void ChefForm::deleteChannelStrip(int id){
    qDebug() <<"Remove channel strip " <<id;
    if (m_clients.contains(id)){
        delete m_clients[id]->cs;
        delete m_clients[id]->comp;
        m_clients.remove(id);
    }
}

void ChefForm::highlightInsert(Compressor * cp){
    if(m_actComp){
        m_actComp->hide();
        ui->m_actCompArea->removeWidget(m_actComp);
    }

    qDebug()<<"Highlight a compressor";
    ui->m_actCompArea->addWidget(cp);
    m_actComp = cp;
    m_actComp->show();
}

void ChefForm::updateAuthCode(){
    ui->authCodeEdit->clearFocus();
    emit authCodeUpdated(ui->authCodeEdit->text());
}

void ChefForm::updateAuthCodeEnabled(){
    ui->codeEnabledBox->clearFocus();
    emit authCodeEnabledUpdated(ui->codeEnabledBox->isChecked());
}

void ChefForm::updateAuthCodeStatus(bool enabled, const QString & authCode){
    handleAuthCodeEnabledUpdated(enabled);
    updateAuthCodeLabel(authCode);
}

void ChefForm::handleAuthCodeEnabledUpdated(bool enabled){
    ui->codeEnabledBox->setChecked(enabled);
}

void ChefForm::updateAuthCodeLabel(const QString &authCode){
    ui->authCodeLabel->setText(authCode);
}

void ChefForm::muteAll(){
    for (LRMClient * client : m_clients.values()){
        client->cs->setMuted(true);
    }
}

void ChefForm::soloRequested(int id, bool checked){
    if (checked && !m_soloers.contains(id)){
        for (int client_id:m_clients.keys()){
            if (id != client_id && !m_soloers.contains(client_id)){
                m_clients[client_id]->cs->sendMute(true);
            }
        }
        emit sendSoloUpdate(id, checked);
        if (m_clients[id]->cs->getMuted()){
            m_clients[id]->cs->sendMute(false);
        }
    }
    else{
        if (m_soloers.contains(id)){
            if (m_soloers.count() > 1 && !m_soloers[id]->cs->getMuted()){
                m_soloers[id]->cs->sendMute(true);
            }
            emit sendSoloUpdate(id, checked);
        }
    }
}

void ChefForm::handleSoloResponse(int id, bool isSolo){
    if (isSolo)
        m_soloers.insert(id, m_clients[id]);
    else
        m_soloers.remove(id);

    if (m_clients.contains(id))
        m_clients[id]->cs->setSolo(isSolo);
}

void ChefForm::handleJoinMutedResponse(bool joinMuted){
    ui->joinMutedBox->setChecked(joinMuted);
    qDebug() << "set muted box checked to " << joinMuted;
}

void ChefForm::disableJackForm(){
    m_tbSetupForm->disableJackForm();
}

void ChefForm::enableJackForm(){
    m_tbSetupForm->enableJackForm();
}

void ChefForm::loadSetup(QSettings & settings){
    m_tbSetupForm->loadSetup(settings);
}

void ChefForm::saveSetup(QSettings & settings){
    m_tbSetupForm->saveSetup(settings);
}

void ChefForm::toggleMute(){
    muted = !muted;
    if(muted){
        ui->muteButton->setText("Unmute");
        emit doMute(true);
    }
    else{
        ui->muteButton->setText("Mute");
        emit doMute(false);
    }
}

void ChefForm::fstartJacktripSec(){
    emit startJacktripSec();
    ui->secondaryConnectButton->setText("3+4 Disconnect");
    QObject::disconnect(ui->secondaryConnectButton, &QAbstractButton::released, this, &ChefForm::fstartJacktripSec);
    QObject::connect(ui->secondaryConnectButton, &QAbstractButton::released, this, &ChefForm::fstopJacktripSec);
}

void ChefForm::fstopJacktripSec(){
    emit stopJacktripSec();
    ui->secondaryConnectButton->setText("3+4 Connect");
    QObject::disconnect(ui->secondaryConnectButton, &QAbstractButton::released, this, &ChefForm::fstopJacktripSec);
    QObject::connect(ui->secondaryConnectButton, &QAbstractButton::released, this, &ChefForm::fstartJacktripSec);
}
