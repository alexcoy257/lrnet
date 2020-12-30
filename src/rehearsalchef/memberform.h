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
    ChatForm * m_chatForm;

signals:
    void nameUpdated(const QString & nname);
    void sectionUpdated(const QString & nsection);

private:
    Ui::MemberForm *ui;
};

#endif // MEMBERFORM_H
