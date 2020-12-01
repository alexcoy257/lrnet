#include "channeltester.h"
#include "ui_channeltester.h"

ChannelTester::ChannelTester(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChannelTester)
{
    ui->setupUi(this);
}

void ChannelTester::deleteFocusedChannelStrip(){

}

ChannelTester::~ChannelTester()
{
    delete ui;
}
