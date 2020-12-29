include(lrnetserver_tests.pri)



TEMPLATE = subdirs
SUBDIRS = rostertests commandstests portpooltests
rostertests.file=./rostertests.pro
commandstests.file=./commandstests.pro
portpooltests.file=./portpooltests.pro

