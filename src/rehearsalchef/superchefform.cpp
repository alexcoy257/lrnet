#include "superchefform.h"
#include "ui_superchefform.h"

SuperChefForm::SuperChefForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SuperChefForm)
{
    ui->setupUi(this);
    ui->superChefList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->chefList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->memberList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(ui->toSuperChef, &QAbstractButton::released, this, [=]{updatePermissions(SUPERCHEF);});
    connect(ui->toChef, &QAbstractButton::released, this, [=]{updatePermissions(CHEF);});
    connect(ui->toMember, &QAbstractButton::released, this, [=]{updatePermissions(MEMBER);});
    connect(ui->removeUsers, &QAbstractButton::released, this, &SuperChefForm::removeUsers);
}

void SuperChefForm::updatePermissions(AuthTypeE authType){
    QList<QListWidgetItem *> usersSelected = QList<QListWidgetItem *>()
                                        << ui->superChefList->selectedItems()
                                        << ui->chefList->selectedItems()
                                        << ui->memberList->selectedItems();

    if (usersSelected.empty())
        return;

    for (QListWidgetItem * user : usersSelected) {
        emit updatePermission(user->text(), authType);
    }

    emit requestRoles();
}

void SuperChefForm::removeUsers(){
    bool empty = true;
    for (QListWidgetItem * user : ui->superChefList->selectedItems()) {
        empty = false;
        emit removeUser(user->text(), SUPERCHEF);
    }

    for (QListWidgetItem * user : ui->chefList->selectedItems()) {
        empty = false;
        emit removeUser(user->text(), CHEF);
    }

    for (QListWidgetItem * user : ui->memberList->selectedItems()) {
        empty = false;
        emit removeUser(user->text(), MEMBER);
    }

    if (!empty)
        emit requestRoles();
}

void SuperChefForm::updateLists(QList<AuthRoster> * authRoster){
    ui->superChefList->clear();
    ui->chefList->clear();
    ui->memberList->clear();
    for (AuthRoster authItem : *authRoster){
        if (authItem.authType == NONE)
            qDebug() << "Hit NONE";
        else if (authItem.authType == MEMBER){;
            ui->memberList->addItem(authItem.name);
        }
        else if (authItem.authType == CHEF){
            ui->chefList->addItem(authItem.name);
        }
        else if (authItem.authType == SUPERCHEF){
            ui->superChefList->addItem(authItem.name);
        }
    }
}


SuperChefForm::~SuperChefForm()
{
    delete ui;
}
