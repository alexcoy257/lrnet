#include <QApplication>
#include "lrnetclient.h"

int main(int argc, char *argv[])
    {
        QApplication app(argc, argv, false);
        Client client("localhost", 4463);
        return app.exec();
    }