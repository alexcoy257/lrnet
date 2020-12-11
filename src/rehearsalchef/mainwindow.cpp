#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_actComp(NULL)
    , m_connectForm(new ConnectForm(NULL))
    , m_openConnectFormButton(new QPushButton(NULL))
    , m_netClient(new LRNetClient())
    , m_keepAliveTimer(this)
{
    ui->setupUi(this);
    //ui->m_channelStripArea->addStretch();
    ui->m_channelStripArea->addStretch();
    ui->m_rosterArea->addWidget(m_openConnectFormButton);



    QObject::connect(m_openConnectFormButton, &QAbstractButton::released, m_connectForm, &QWidget::show);
    QObject::connect(m_connectForm, &ConnectForm::tryConnect, m_netClient, &LRNetClient::tryConnect);
    QObject::connect(m_netClient, &LRNetClient::timeout, this, [=](){qDebug()<<"Timeout";});
    QObject::connect(m_netClient, &LRNetClient::connected, this,
                     [=](){
        qDebug()<<"Connected";
    });
    QObject::connect(m_netClient, &LRNetClient::newMember, this, &MainWindow::addChannelStrip);
    QObject::connect(m_netClient, &LRNetClient::lostMember, this, &MainWindow::deleteChannelStrip);

    /*
    m_channelStrip = new ChannelStrip(this);
    ui->m_channelStripArea->addWidget(m_channelStrip);
    m_comp = new Compressor(this);
    m_comp->hide();

*/

    //ui->m_channelStripArea->addWidget(m_channelStrip);
    //m_channelStrip->setCompressorZone(ui->m_actCompArea);


}

void MainWindow::highlightInsert(Compressor * cp){
    if(m_actComp){
        m_actComp->hide();
        ui->m_actCompArea->removeWidget(m_actComp);
    }


    ui->m_actCompArea->addWidget(cp);
    m_actComp = cp;
    m_actComp->show();
}

void MainWindow::addChannelStrip(const QString& mName, const QString& sName, int id){

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

void MainWindow::deleteChannelStrip(int id){

    if (m_clients.contains(id)){
        delete m_clients[id]->cs;
        delete m_clients[id]->comp;
        m_clients.remove(id);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_connectForm;
}

