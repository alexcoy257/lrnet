#ifndef CHANNELTESTER_H
#define CHANNELTESTER_H

#include <QWidget>

namespace Ui {
class ChannelTester;
}

class ChannelTester : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelTester(QWidget *parent = nullptr);
    ~ChannelTester();

private:
    Ui::ChannelTester *ui;
    void deleteFocusedChannelStrip();
};

#endif // CHANNELTESTER_H
