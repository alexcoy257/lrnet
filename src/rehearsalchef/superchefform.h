#ifndef SUPERCHEFFORM_H
#define SUPERCHEFFORM_H

#include <QWidget>
#include "../lrnetserver/auth_types.h"
#include "../lrnetclient/lrnetclient.h"

namespace Ui {
class SuperChefForm;
}

class SuperChefForm : public QWidget
{
    Q_OBJECT

public:
    explicit SuperChefForm(QWidget *parent = nullptr);
    virtual ~SuperChefForm();

public slots:
    void updateLists(QList<AuthRoster> * authRoster);

signals:
    void updatePermission(QString netid, AuthTypeE authType);
    void requestRoles();
    void removeUser(QString netid, AuthTypeE authType);


private:
    Ui::SuperChefForm *ui;
    void updatePermissions(AuthTypeE);
    void removeUsers();
};

#endif // SUPERCHEFFORM_H
