#include "compressor.h"
#include "ui_compressor.h"
#include "chefform.h"

Compressor::Compressor(LRMClient * cStruct, QWidget *parent)
   : QWidget(parent)
   , ui(new Ui::Compressor)
   , mCStruct(cStruct)
   , threshTimer(new QTimer(this))
   , ratioTimer(new QTimer(this))
   , attackTimer(new QTimer(this))
   , releaseTimer(new QTimer(this))
   , makeupTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupSignals();

}

Compressor::Compressor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Compressor)
    , threshTimer(new QTimer(this))
    , ratioTimer(new QTimer(this))
    , attackTimer(new QTimer(this))
    , releaseTimer(new QTimer(this))
    , makeupTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupSignals();

}

void Compressor::setupSignals(){
    QObject::connect(ui->c_threshKnob, &QAbstractSlider::valueChanged, this, &Compressor::setThresh);
    threshTimer->callOnTimeout(this, [=](){if (threshChanged) {
        emit valueChanged(mCStruct, THRESHOLD, (float)threshold);
        threshChanged = false;
        ui->c_threshVal->setText(QString("Threshold: %1 dB").arg(threshold));
    }});
    threshTimer->start(100);

    QObject::connect(ui->c_ratioKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRatio);
    ratioTimer->callOnTimeout(this, [=](){if (ratioChanged) {
        emit valueChanged(mCStruct, RATIO, (float)ratio);
        ratioChanged = false;
        ui->c_ratioVal->setText(QString("Ratio: %1").arg(ratio));
    }});
    ratioTimer->start(100);

    QObject::connect(ui->c_attackKnob, &QAbstractSlider::valueChanged, this, &Compressor::setAttack);
    attackTimer->callOnTimeout(this, [=](){if (attackChanged) {
        emit valueChanged(mCStruct, ATTACK, (float)attack);
        attackChanged = false;
        ui->c_attackVal->setText(QString("Attack: %1 ms").arg(attack));
    }});
    attackTimer->start(100);


    QObject::connect(ui->c_releaseKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRelease);
    releaseTimer->callOnTimeout(this, [=](){if (releaseChanged) {
        emit valueChanged(mCStruct, RELEASE, (float)release);
        releaseChanged = false;
        ui->c_releaseVal->setText(QString("Release: %1 ms").arg(release));
    }});
    releaseTimer->start(100);


    QObject::connect(ui->c_makeupKnob, &QAbstractSlider::valueChanged, this, &Compressor::setMakeup);
    makeupTimer->callOnTimeout(this, [=](){if (makeupChanged) {
        emit valueChanged(mCStruct, MAKEUP, (float)makeup);
        makeupChanged = false;
        ui->c_makeupVal->setText(QString("Make-up Gain: %1 dB").arg(makeup));
    }});
    makeupTimer->start(100);


}

void Compressor::setThresh(int value){
    threshChanged = true;
    threshold = value;
}

void Compressor::setRatio(int value){
    ratioChanged = true;
    ratio = value;
}

void Compressor::setAttack(int value){
    attackChanged = true;
    attack = value;
}

void Compressor::setRelease(int value){
    releaseChanged = true;
    release = value;
}

void Compressor::setMakeup(int value){
    makeupChanged = true;
    makeup= value;
}

Compressor::~Compressor()
{
    delete ui;
}
