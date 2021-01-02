include(lrnet.pri)
TEMPLATE=subdirs
SUBDIRS = lrnetclient rehearsalchef

!win32{
SUBDIRS += lrnetserver lrnetserver_tests lrdb_client lrdb_client_app
lrnetserver_tests.file = lrnetserver/lrnetserver_tests.pro
lrdb_client_app.file = lrdb_client/lrdb_client_app.pro
lrdb_client_app.depends = lrdb_client
lrnetserver.depends = lrdb_client
}
rehearsalchef.depends = lrnetclient
