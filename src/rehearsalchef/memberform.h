#ifndef MEMBERFORM_H
#define MEMBERFORM_H

#include <QWidget>

#include "chatform.h"

namespace Ui {
class MemberForm;
}

class MemberForm : public QWidget
{
    Q_OBJECT


public:
    explicit MemberForm(QWidget *parent = nullptr);
    ~MemberForm();
    void setName(const QString & nname);
    void setSection(const QString & nsection);

signals:
    void nameUpdated(const QString & nname);
    void sectionUpdated(const QString & nsection);

private:
    Ui::MemberForm *ui;
    ChatForm * m_chatForm;

    void updateName();


};

#endif // MEMBERFORM_H
