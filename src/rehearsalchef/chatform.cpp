#include "chatform.h"
#include "ui_chatform.h"

#include <QDebug>

ChatForm::ChatForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatForm)
{
    ui->setupUi(this);

    ui->messageToSend->setFocusPolicy(Qt::StrongFocus);
    ui->messageLog->setFocusPolicy(Qt::NoFocus);
    ui->messageLog->setReadOnly(true);

    connect(ui->messageToSend, &QLineEdit::returnPressed,
            this, &ChatForm::returnPressed);

    tableFormat.setBorder(0);
}

void ChatForm::returnPressed()
{
    QString text = ui->messageToSend->text();
    if (!text.isEmpty()){
        emit sendChat(text);
        ui->messageToSend->clear();
    }
}


void ChatForm::appendMessage(const QString &from, const QString &msg)
{
    if (from.isEmpty() || msg.isEmpty())
        return;

    QTextCursor cursor(ui->messageLog->textCursor());
    cursor.movePosition(QTextCursor::End);
    QTextTable *table = cursor.insertTable(1, 2, tableFormat);

    table->cellAt(0, 0).firstCursorPosition().insertText('<' + from + "> ");
    table->cellAt(0, 1).firstCursorPosition().insertText(msg);
    QScrollBar *bar = ui->messageLog->verticalScrollBar();
    bar->setValue(bar->maximum());
}



ChatForm::~ChatForm()
{
    delete ui;
}
