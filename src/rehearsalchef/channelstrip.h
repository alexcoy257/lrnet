#ifndef CHANNELSTRIP_H
#define CHANNELSTRIP_H

#include <QObject>
#include <QWidget>
#include "ui_channelStrip.h"
#include "compressor.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ChannelStrip;}
QT_END_NAMESPACE

struct LRMClient;

class ChannelStrip : public QWidget
{
    Q_OBJECT

    QVector<float> currentControls = {0,0,0,0,0,0,0,0};
    LRMClient * mCStruct;

public:
    typedef enum{
        COMP_BYPASS=0,
        COMP_RATIO,
        COMP_THRESHOLD,
        COMP_ATTACK,
        COMP_RELEASE,
        COMP_MAKEUP,
        GROUP_GAIN,
        INDIV_GAIN
    } CSValuesE;

    explicit ChannelStrip(LRMClient * cStruct, QWidget *parent = nullptr, QString cname = "");
    ~ChannelStrip();
    void setCompressorZone(QLayout *zone);
    void setName(const QString & nname);
    void setSection(const QString & sname);

public slots:
    void newControls(QVector<float> & controls);
    void setControl(int type, float value){
        currentControls[type] = value;
    }
    QVector<float> getCurrentControls(){return currentControls;}

private:
    //QWidget * m_parent;
    //Compressor cs_compressor;
    Ui::ChannelStrip *ui;
    QLayout *cs_compressorZone;
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);
    QString name;
    QString section;

signals:
    void setActive();
    void gotNewControls(QVector<float> & controls);
    void valueChanged(LRMClient * myClient, int type, float value);

private slots:
    void showCompressor();
    void passCompressor();


};

#endif // CHANNELSTRIP_H
