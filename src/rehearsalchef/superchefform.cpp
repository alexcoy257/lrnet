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

    connect(ui->toSuperChef, &QAbstractButton::released, this, [=]{updateSelectedPermissions(SUPERCHEF);});
    connect(ui->toChef, &QAbstractButton::released, this, [=]{updateSelectedPermissions(CHEF);});
    connect(ui->toMember, &QAbstractButton::released, this, [=]{updateSelectedPermissions(MEMBER);});
    connect(ui->removeUsers, &QAbstractButton::released, this, &SuperChefForm::removeSelectedUsers);
}

void SuperChefForm::updateSelectedPermissions(AuthTypeE authType){
    QList<QListWidgetItem *> itemsSelected = QList<QListWidgetItem *>()
                                        << ui->superChefList->selectedItems()
                                        << ui->chefList->selectedItems()
                                        << ui->memberList->selectedItems();

    if (itemsSelected.empty())
        return;

    QList<QString> *netidsSelected = new QList<QString>();
    for (QListWidgetItem * item : itemsSelected) {
        netidsSelected->append(item->text());
    }

    emit updatePermissions(netidsSelected, authType);
}

void SuperChefForm::removeSelectedUsers(){
    QList<QListWidgetItem *> itemsSelected = QList<QListWidgetItem *>()
                                        << ui->superChefList->selectedItems()
                                        << ui->chefList->selectedItems()
                                        << ui->memberList->selectedItems();

    if (itemsSelected.empty())
        return;

    QList<QString> *netidsSelected = new QList<QString>();
    for (QListWidgetItem * item : itemsSelected)
        netidsSelected->append(item->text());

    emit removeUsers(netidsSelected);
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
