#ifndef CHEFFORM_H
#define CHEFFORM_H

#include <QWidget>
#include <QDebug>

#include "channelstrip.h"
#include "compressor.h"
#include "chatform.h"

namespace Ui {
class ChefForm;
}

class ChefForm : public QWidget
{
    Q_OBJECT
    typedef struct{
        ChannelStrip * cs;
        Compressor * comp;
    }LRMClient;

    Compressor * m_actComp;
    QHash<int, LRMClient *> m_clients;
public:
    explicit ChefForm(QWidget *parent = nullptr);
    ~ChefForm();
    ChatForm * m_chatForm;

public slots:
    void highlightInsert(Compressor * cp);
    void addChannelStrip(const QString& mName, const QString& sName, int id);
    void updateChannelStrip(const QString& mName, const QString& sName, int id);
    void deleteChannelStrip(int id);
    void updateAuthCodeEnabled();

signals:
    void authCodeUpdated(const QString & nname);
    void authCodeEnabledUpdated(bool enabled);

private:
    Ui::ChefForm *ui;
    void updateAuthCode();

};

#endif // CHEFFORM_H
