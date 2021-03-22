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
    }});
    threshTimer->start(100);

    QObject::connect(ui->c_ratioKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRatio);
    ratioTimer->callOnTimeout(this, [=](){if (ratioChanged) {
        emit valueChanged(mCStruct, RATIO, (float)ratio);
        ratioChanged = false;
    }});
    ratioTimer->start(100);

    QObject::connect(ui->c_attackKnob, &QAbstractSlider::valueChanged, this, &Compressor::setAttack);
    attackTimer->callOnTimeout(this, [=](){if (attackChanged) {
        emit valueChanged(mCStruct, ATTACK, (float)attack);
        attackChanged = false;
    }});
    attackTimer->start(100);


    QObject::connect(ui->c_releaseKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRelease);
    releaseTimer->callOnTimeout(this, [=](){if (releaseChanged) {
        emit valueChanged(mCStruct, RELEASE, (float)release);
        releaseChanged = false;
    }});
    releaseTimer->start(100);


    QObject::connect(ui->c_makeupKnob, &QAbstractSlider::valueChanged, this, &Compressor::setMakeup);
    makeupTimer->callOnTimeout(this, [=](){if (makeupChanged) {
        emit valueChanged(mCStruct, MAKEUP, (float)makeup);
        makeupChanged = false;
    }});
    makeupTimer->start(100);


}

void Compressor::setThresh(int value){
    threshChanged = true;
    threshold = value;
}

void Compressor::setThreshWithoutSignal(int value){
    if (!threshChanged){
        QObject::disconnect(ui->c_threshKnob, &QAbstractSlider::valueChanged, this, &Compressor::setThresh);
        ui->c_threshKnob->setValue(value);
        ui->c_threshVal->setText(QString("Threshold: %1 dB").arg(value));
        QObject::connect(ui->c_threshKnob, &QAbstractSlider::valueChanged, this, &Compressor::setThresh);
    }
}

void Compressor::setRatio(int value){
    ratioChanged = true;
    ratio = value;
}

void Compressor::setRatioWithoutSignal(int value){
    if (!ratioChanged){
        QObject::disconnect(ui->c_ratioKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRatio);
        ui->c_ratioKnob->setValue(value);
        ui->c_ratioVal->setText(QString("Ratio: %1").arg(value));
        QObject::connect(ui->c_ratioKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRatio);
    }
}

void Compressor::setAttack(int value){
    attackChanged = true;
    attack = value;
}

void Compressor::setAttackWithoutSignal(int value){
    if (!attackChanged){
        QObject::disconnect(ui->c_attackKnob, &QAbstractSlider::valueChanged, this, &Compressor::setAttack);
        ui->c_attackKnob->setValue(value);
        ui->c_attackVal->setText(QString("Attack: %1 ms").arg(value));
        QObject::connect(ui->c_attackKnob, &QAbstractSlider::valueChanged, this, &Compressor::setAttack);
    }
}

void Compressor::setRelease(int value){
    releaseChanged = true;
    release = value;
}

void Compressor::setReleaseWithoutSignal(int value){
    if (!releaseChanged){
        QObject::disconnect(ui->c_releaseKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRelease);
        ui->c_releaseKnob->setValue(value);
        ui->c_releaseVal->setText(QString("Release: %1 ms").arg(value));
        QObject::connect(ui->c_releaseKnob, &QAbstractSlider::valueChanged, this, &Compressor::setRelease);
    }
}

void Compressor::setMakeup(int value){
    makeupChanged = true;
    makeup= value;
}

void Compressor::setMakeupWithoutSignal(int value){
    if (!makeupChanged){
        QObject::disconnect(ui->c_makeupKnob, &QAbstractSlider::valueChanged, this, &Compressor::setMakeup);
        ui->c_makeupKnob->setValue(value);
        ui->c_makeupVal->setText(QString("Make-up Gain: %1 dB").arg(value));
        QObject::connect(ui->c_makeupKnob, &QAbstractSlider::valueChanged, this, &Compressor::setMakeup);
    }
}

void Compressor::newControls(QVector<float> & controls){
    setRatioWithoutSignal(controls[1]);
    setThreshWithoutSignal(controls[2]);
    setAttackWithoutSignal(controls[3]);
    setReleaseWithoutSignal(controls[4]);
    setMakeupWithoutSignal(controls[5]);
}

Compressor::~Compressor()
{
    delete ui;
}
