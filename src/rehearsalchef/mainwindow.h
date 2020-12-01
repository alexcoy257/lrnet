#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QObject>
#include "channelstrip.h"
#include "compressor.h"
#include "channeltester.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; class ChannelStrip;}
QT_END_NAMESPACE

typedef struct{
    ChannelStrip * cs;
    Compressor * comp;
}LRMClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addChannelStrip(QString cName);

private:
    Ui::MainWindow *ui;
    ChannelStrip * m_channelStrip;
    Compressor * m_comp;
    Compressor * m_actComp;
    QHash<QString, LRMClient *> m_clients;

signals:
    void deleteChannel(QString s);

private slots:
    void highlightInsert(Compressor * cp);
};
#endif // MAINWINDOW_H
