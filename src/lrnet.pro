TEMPLATE=subdirs
SUBDIRS = lrnetclient lrnetserver lrmixer rehearsalchef lrdb_client lrdb_client_app
lrdb_client_app.file = lrdb_client/lrdb_client_app.pro
lrdb_client_app.depends = lrdb_client
lrnetserver.depends = lrmixer lrdb_client
rehearsalchef.depends = lrnetclient
