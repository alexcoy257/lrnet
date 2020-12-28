#ifndef CHANNELSTRIP_H
#define CHANNELSTRIP_H

#include <QObject>
#include <QWidget>
#include "ui_channelStrip.h"
#include "compressor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChannelStrip;}
QT_END_NAMESPACE

class ChannelStrip : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelStrip(QWidget *parent = nullptr, QString cname = "");
    ~ChannelStrip();
    void setCompressorZone(QLayout *zone);
    void setName(const QString & nname);

private:
    //QWidget * m_parent;
    Compressor cs_compressor;
    Ui::ChannelStrip *ui;
    QLayout *cs_compressorZone;
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);
    QString name;

signals:
    void setActive();

private slots:
    void showCompressor();
    void passCompressor();


};

#endif // CHANNELSTRIP_H
