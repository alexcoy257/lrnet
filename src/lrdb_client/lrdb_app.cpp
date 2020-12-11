#include "lrdb_app.h"
#include <QDebug>
#include <QScopedPointer>

LRdbApp::LRdbApp(int argc, char ** argv) : QCoreApplication(argc, argv)
{
    QCommandLineOption showProgressOption("p", QCoreApplication::translate("main", "Show progress during copy"));
    parser.addOption(showProgressOption);
    parser.process(*this);

    client = QSharedPointer<LRdbClient>(new LRdbClient("lrdbread", "lrdbreadpw"));

}
