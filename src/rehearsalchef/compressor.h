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

signals:
    void valueChanged(LRMClient * myClient, CValuesE type, float value);

private:
    Ui::Compressor *ui;
    void setupSignals();
};

#endif // COMPRESSOR_H
