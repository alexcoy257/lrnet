#ifndef MEMBERFORM_H
#define MEMBERFORM_H

#include <QWidget>

#include "chatform.h"
#include "../JackServerTest/jackparameterform.h"

namespace Ui {
class MemberForm;
}

class MemberForm : public QWidget
{
    Q_OBJECT


public:
    explicit MemberForm(QWidget *parent = nullptr);
    virtual ~MemberForm();

    ChatForm * m_chatForm;

    void setName(const QString & nname);
    void setSection(const QString & nsection);

signals:
    void nameUpdated(const QString & nname);
    void sectionUpdated(const QString & nsection);
    void startJackTrip();
    void stopJackTrip();

private:
    Ui::MemberForm *ui;
    //JackParameterForm * m_jackForm;

    void updateName();
    void fstartJacktrip();
    void fstopJacktrip();


};

#endif // MEMBERFORM_H
