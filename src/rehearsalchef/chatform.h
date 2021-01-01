#ifndef CHATFORM_H
#define CHATFORM_H

#include <QWidget>
#include <QTextEdit>
#include <QTextTable>
#include <QScrollBar>

namespace Ui {
class ChatForm;
}

class ChatForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatForm(QWidget *parent = nullptr);
    ~ChatForm();

public slots:
    void appendMessage(const QString &from, const QString &msg);

signals:
    void sendChat(QString & chatMsg);

private:
    Ui::ChatForm *ui;
    QTextTableFormat tableFormat;

private slots:
    void returnPressed();

};

#endif // CHATFORM_H
