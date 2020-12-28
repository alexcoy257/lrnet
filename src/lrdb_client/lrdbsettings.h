#ifndef LRDBSETTINGS_H
#define LRDBSETTINGS_H

#include <QObject>

class LRdbSettings : public QObject
{
    Q_OBJECT

    bool valid;
public:
    explicit LRdbSettings(const QString & settingsFile = "/etc/lrnet/settings.json", QObject *parent = nullptr);
    int loadSettingsFile(QString & filename);
    QString username;
    QString password;
    QString host;
    QString database;
    bool validFile(){return valid;};

signals:

};

#endif // LRDBSETTINGS_H
