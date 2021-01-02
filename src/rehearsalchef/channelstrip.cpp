#include "channelstrip.h"
#include "chefform.h"
#include <QDebug>

ChannelStrip::ChannelStrip(LRMClient * cStruct, QWidget *parent, QString cName)
  : QWidget(parent)
  , mCStruct(cStruct)
  , ui(new Ui::ChannelStrip)
  , cs_compressorZone(NULL)
  , name(cName)
{
  ui->setupUi(this);
  ui->cs_preGain->setMaximumSize(45,45);
  ui->cs_preGain->updateGeometry();
  ui->cs_preGain->hide();
  //qDebug() <<name;
  ui->cs_cName->setText(cName);
  setFocusPolicy(Qt::ClickFocus);
  //cs_compressor.hide();
  QObject::connect(ui->cs_compButton, &QAbstractButton::released, this, &ChannelStrip::passCompressor, Qt::QueuedConnection);
  QObject::connect(ui->cs_postGain, &QAbstractSlider::sliderMoved, this,
                   [=](int value){emit valueChanged(mCStruct, (int)INDIV_GAIN, float(value));});
}

ChannelStrip::~ChannelStrip()
{
    delete ui;
}

void ChannelStrip::focusInEvent(QFocusEvent *){
    qDebug() << "Received focus";
}

void ChannelStrip::focusOutEvent(QFocusEvent *){
    qDebug() << "Lost focus";
}

void ChannelStrip::setCompressorZone(QLayout *zone){
    cs_compressorZone = zone;
    qDebug() << "Compressor Zone set \n";
}

void ChannelStrip::passCompressor(){
    emit setActive();
}

void ChannelStrip::showCompressor(){
    cs_compressorZone->addWidget(mCStruct->comp);
    //cs_compressor.hide();
    //this->hide();
    qDebug() <<"Compressor " <<(mCStruct->comp->isVisible()?"visible":"invisible") <<" \n";
}

void ChannelStrip::setName(const QString & nname){
    name = nname;
    ui->cs_cName->setText(nname);
}

void ChannelStrip::setSection(const QString & nsection){
    section = nsection;
}

void ChannelStrip::newControls(QVector<float> & controls){
    currentControls = controls;
    ui->cs_postGain->setValue(controls[7]);
}
