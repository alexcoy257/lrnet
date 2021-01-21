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

void SuperChefForm::updateLists(QList<AuthRoster> * authRoster){
    ui->superChefList->clear();
    ui->chefList->clear();
    ui->memberList->clear();
    for (AuthRoster authItem : *authRoster){
        if (authItem.authType == NONE)
            qDebug() << "Hit NONE";
        else if (authItem.authType == MEMBER){
            qDebug() << "Hit MEMBER";
            ui->memberList->addItem(authItem.name);
        }
        else if (authItem.authType == CHEF){
                qDebug() << "Hit CHEF";
                ui->chefList->addItem(authItem.name);
        }
        else if (authItem.authType == SUPERCHEF){
                qDebug() << "Hit SUPERCHEF";
                ui->superChefList->addItem(authItem.name);
        }
    }
}


SuperChefForm::~SuperChefForm()
{
    delete ui;
}
