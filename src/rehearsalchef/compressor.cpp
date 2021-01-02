#include "compressor.h"
#include "ui_compressor.h"
#include "chefform.h"

Compressor::Compressor(LRMClient * cStruct, QWidget *parent) :
    QWidget(parent),
    mCStruct(cStruct),
    ui(new Ui::Compressor)
{
    ui->setupUi(this);
    QObject::connect(ui->c_threshKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, THRESHOLD, (float)val);});
    QObject::connect(ui->c_ratioKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, RATIO, (float)val);});
    QObject::connect(ui->c_attackKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, ATTACK, (float)val);});
    QObject::connect(ui->c_releaseKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, RELEASE, (float)val);});
    QObject::connect(ui->c_gainKnob, &QAbstractSlider::sliderMoved,
                     this, [=](int val){emit valueChanged(mCStruct, MAKEUP, (float)val);});

}

Compressor::~Compressor()
{
    delete ui;
}
