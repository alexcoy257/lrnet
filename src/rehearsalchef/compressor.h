#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <QWidget>

namespace Ui {
class Compressor;
}

struct LRMClient;

class Compressor : public QWidget
{
    Q_OBJECT

    LRMClient * mCStruct;
public:
    typedef enum {
        RATIO = 1,
        THRESHOLD,
        ATTACK,
        RELEASE,
        MAKEUP
    } CValuesE;
    explicit Compressor(LRMClient * cStruct, QWidget *parent = nullptr);
    explicit Compressor(QWidget *parent = nullptr);
    ~Compressor();

public slots:
    void setThresh(int value);
    void setRatio(int value);
    void setAttack(int value);
    void setRelease(int value);
    void setMakeup(int value);

signals:
    void valueChanged(LRMClient * myClient, CValuesE type, float value);

private:
    Ui::Compressor *ui;
    void setupSignals();

    QTimer * threshTimer;
    bool threshChanged = false;
    int threshold = 0;

    QTimer * ratioTimer;
    bool ratioChanged = false;
    int ratio = 0;

    QTimer * attackTimer;
    bool attackChanged = false;
    int attack = 0;

    QTimer * releaseTimer;
    bool releaseChanged = false;
    int release = 0;

    QTimer * makeupTimer;
    bool makeupChanged = false;
    int makeup = 0;

};

#endif // COMPRESSOR_H
