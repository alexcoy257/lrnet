#include "talkbacksettingsform.h"
#include "ui_talkbacksettingsform.h"

TalkbackSettingsForm::TalkbackSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TalkbackSettingsForm)
{
    ui->setupUi(this);
}

TalkbackSettingsForm::~TalkbackSettingsForm()
{
    delete ui;
}
