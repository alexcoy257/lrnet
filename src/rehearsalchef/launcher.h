#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

#include "../lrnetserver/auth_types.h"

class Launcher : public QWidget
{
    Q_OBJECT
    QHBoxLayout * m_layout;
    QPushButton * m_superChefButton;
    QPushButton * m_chefButton;
    QPushButton * m_memberButton;
public:
    explicit Launcher(AuthTypeE level = NONE, QWidget *parent = nullptr);

public slots:
signals:
    void choseSuperChef();
    void choseChef();
    void choseMember();

};

#endif // LAUNCHER_H
