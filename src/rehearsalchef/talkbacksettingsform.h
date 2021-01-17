#ifndef TALKBACKSETTINGSFORM_H
#define TALKBACKSETTINGSFORM_H

#include <QWidget>

namespace Ui {
class TalkbackSettingsForm;
}

class TalkbackSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit TalkbackSettingsForm(QWidget *parent = nullptr);
    ~TalkbackSettingsForm();

private:
    Ui::TalkbackSettingsForm *ui;
};

#endif // TALKBACKSETTINGSFORM_H
