#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <QWidget>

namespace Ui {
class Compressor;
}

class Compressor : public QWidget
{
    Q_OBJECT

public:
    explicit Compressor(QWidget *parent = nullptr);
    ~Compressor();

private:
    Ui::Compressor *ui;
};

#endif // COMPRESSOR_H
