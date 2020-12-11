#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>
#include <QThread>
#include "mainwindow.h"


class TestWorker : public QObject
{
    Q_OBJECT

    MainWindow * w;
public:
    TestWorker(MainWindow * _w):w(_w){};
    void doTests(){
        //w->addChannelStrip("Mike-Tbn");
        //w->addChannelStrip("Jimmy-Sax");
        //QThread::sleep(4);
        //w->addChannelStrip("John-Tbn");
    }
signals:
    void resultReady(const QString &s);
};

#endif // TESTWORKER_H
