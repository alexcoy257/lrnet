#include "compressor.h"
#include "ui_compressor.h"
#include "chefform.h"

Compressor::Compressor(LRMClient * cStruct, QWidget *parent) :
    QWidget(parent),
    mCStruct(cStruct),
    ui(new Ui::Compressor)
{
    ui->setupUi(this);
    setupSignals();

}

Compressor::Compressor(QWidget *parent) :
    QWidget(parent),
    mCStruct(NULL),
    ui(new Ui::Compressor)
{
    ui->setupUi(this);
    setupSignals();

}

void Compressor::setupSignals(){
    QObject::connect(ui->c_threshKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){
        emit valueChanged(mCStruct, THRESHOLD, (float)val);
        ui->c_threshVal->setText(QString("Threshold: %1 dB").arg(val));
    });
    QObject::connect(ui->c_ratioKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){
        emit valueChanged(mCStruct, RATIO, (float)val);
        ui->c_ratioVal->setText(QString("Ratio: %1").arg(val));
    });
    QObject::connect(ui->c_attackKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, ATTACK, (float)val);
                    ui->c_attackVal->setText(QString("Attack: %1 ms").arg(val));
    });
    QObject::connect(ui->c_releaseKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, RELEASE, (float)val);
                    ui->c_releaseVal->setText(QString("Release: %1 ms").arg(val));
    });
    QObject::connect(ui->c_gainKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){
                    emit valueChanged(mCStruct, MAKEUP, (float)val);
                    ui->c_gainVal->setText(QString("Make-up Gain: %1 dB").arg(val));
    });
}


Compressor::~Compressor()
{
    delete ui;
}
