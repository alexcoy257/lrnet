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

private:
    Ui::MemberForm *ui;
    ChatForm * m_chatForm;
};

#endif // MEMBERFORM_H
