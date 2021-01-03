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

typedef struct LRMClient{
    ChannelStrip * cs;
    Compressor * comp;
    int64_t id;
}LRMClient;

class ChefForm : public QWidget
{
    Q_OBJECT


    Compressor * m_actComp = NULL;
    QHash<int, LRMClient *> m_clients;
public:
    explicit ChefForm(QWidget *parent = nullptr);
    virtual ~ChefForm();


public slots:
    void highlightInsert(Compressor * cp);
    void addChannelStrip(const QString& mName, const QString& sName, QVector<float> controls, int id);
    void updateChannelStrip(const QString& mName, const QString& sName, int id);
    void deleteChannelStrip(int id);

signals:
    void sendControlUpdate(int id, QVector<float> & controls);
    void authCodeUpdated(const QString & nname);
    void authCodeEnabledUpdated(bool enabled);

private:
    Ui::ChefForm *ui;
    void updateAuthCode();
    void updateAuthCodeEnabled();

private slots:
    void newValueHandler(LRMClient * myClient, int type, float value);

public:
    ChatForm * m_chatForm;
};

#endif // CHEFFORM_H
