#ifndef LRDBSETTINGS_H
#define LRDBSETTINGS_H

#include <QObject>

class LRdbSettings : public QObject
{
    Q_OBJECT
public:
    explicit LRdbSettings(QObject *parent = nullptr);
    int loadSettingsFile(QString & filename);
    QString username;
    QString password;
    QString host;
    QString database;

signals:

};

#endif // LRDBSETTINGS_H
