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
    void updatePermissions(QList<QString> *netidsSelected, AuthTypeE authType);
    void requestRoles();
    void removeUsers(QList<QString> *netidsSelected);


private:
    Ui::SuperChefForm *ui;
    void updateSelectedPermissions(AuthTypeE authType);
    void removeSelectedUsers();
};

#endif // SUPERCHEFFORM_H
