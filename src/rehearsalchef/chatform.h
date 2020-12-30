#ifndef CHATFORM_H
#define CHATFORM_H

#include <QWidget>

namespace Ui {
class ChatForm;
}

class ChatForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatForm(QWidget *parent = nullptr);
    ~ChatForm();

signals:
    void newMessage(QString & message);

private:
    Ui::ChatForm *ui;

};

#endif // CHATFORM_H
