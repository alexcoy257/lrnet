#include "chefform.h"
#include "ui_chefform.h"

ChefForm::ChefForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChefForm)
{
    ui->setupUi(this);
    ui->m_channelStripArea->addStretch();
}

ChefForm::~ChefForm()
{
    delete ui;
}

void ChefForm::addChannelStrip(const QString& mName, const QString& sName, int id){

    if (! m_clients.contains(id)){
        LRMClient * cStruct = (LRMClient *)malloc(sizeof(LRMClient));
        cStruct->cs = new ChannelStrip(this, mName);

        cStruct->comp = new Compressor(this);
        cStruct->comp->hide();
        qDebug() <<"Widgets present: " << ui->m_channelStripArea->count() <<"\n";
        ui->m_channelStripArea->insertWidget(ui->m_channelStripArea->count()-1,cStruct->cs);
        //ui->m_channelStripArea->addWidget(cStruct->cs, 0, Qt::AlignLeft);
        //cStruct->cs->show();
        QObject::connect(cStruct->cs, &ChannelStrip::setActive, this, [=](){highlightInsert(cStruct->comp);}, Qt::QueuedConnection);
        m_clients[id] = cStruct;
    }
}

void ChefForm::updateChannelStrip(const QString& mName, const QString& sName, int id){
    LRMClient * client = m_clients[id];
    if(client){
        client ->cs;
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


    ui->m_actCompArea->addWidget(cp);
    m_actComp = cp;
    m_actComp->show();
}
