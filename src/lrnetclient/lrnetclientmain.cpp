#include <QApplication>
#include "lrnetclient.h"

int main(int argc, char *argv[])
    {
        QApplication app(argc, argv, false);
        LRNetClient client;
        return app.exec();
    }
