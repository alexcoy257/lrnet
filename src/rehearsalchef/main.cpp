#include "mainwindow.h"
#include "channelstrip.h"

#include <QApplication>
#include <QtTest>
#include <QThread>
#include "testworker.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();    

    //w.addChannelStrip("Mike-Tbn");
    //w.addChannelStrip("Jimmy-Sax");
    //w.addChannelStrip("John-Tbn");
    //w.addChannelStrip("Paul-Tbn");
    //w.addChannelStrip("George-Tbn");

    return a.exec();
}


