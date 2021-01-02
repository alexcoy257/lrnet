#include "chefform.h"
#include "ui_chefform.h"

ChefForm::ChefForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChefForm)

{
    ui->setupUi(this);
    m_chatForm = ui->m_chatForm;
    ui->m_channelStripArea->addStretch();
    //ui->chatArea->addWidget(m_chatForm);

    QObject::connect(ui->authCodeEdit, &QLineEdit::editingFinished, this, &ChefForm::updateAuthCode);
    QObject::connect(ui->codeEnabledBox, &QCheckBox::stateChanged, this, &ChefForm::updateAuthCodeEnabled);
}

ChefForm::~ChefForm()
{
    delete ui;
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
        qDebug() <<"Widgets present: " << ui->m_channelStripArea->count();
        ui->m_channelStripArea->insertWidget(ui->m_channelStripArea->count()-1,cStruct->cs);
        //ui->m_channelStripArea->addWidget(cStruct->cs, 0, Qt::AlignLeft);
        //cStruct->cs->show();
        QObject::connect(cStruct->cs, &ChannelStrip::setActive, this, [=](){highlightInsert(cStruct->comp);}, Qt::QueuedConnection);
        QObject::connect(cStruct->comp, &Compressor::valueChanged, this, &ChefForm::newValueHandler);
        QObject::connect(cStruct->cs, &ChannelStrip::valueChanged, this, &ChefForm::newValueHandler);
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
