include(lrnetserver_tests.pri)



TEMPLATE = subdirs
SUBDIRS = rostertests commandstests
rostertests.file=./rostertests.pro
commandstests.file=./commandstests.pro

