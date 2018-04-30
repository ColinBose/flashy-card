#include "friends.h"
#include "ui_friends.h"


Friends::Friends(QWidget *parent, MainWindow *m, MultiManager *man) :
    QDialog(parent),
    ui(new Ui::Friends)
{
    mw = m;
    manager = man;
    ui->setupUi(this);
    setBasicTableStuff();
}

Friends::~Friends()
{
    delete ui;
}

void Friends::on_friendAddbutton_clicked()
{
    ui->friendErrorLabel->setText("");
    QString name = ui->friendAddEdit->text();
    if(name == mw->loggedInName){
        ui->friendErrorLabel->setText("You can add yourself");
        ui->friendAddEdit->setText("");
    }
    if(name.length() == 0)
        return;
    if(name.length() > 100)
        return;
    manager->sendFriendAdd(name);
}

void Friends::on_friendAddEdit_returnPressed()
{
    //on_friendAddbutton_clicked();
}
void Friends::handleAdd(int code){
    if(code == 0){
        ui->friendErrorLabel->setText("Error adding friend, user not found");
    }
    else{
        ui->friendErrorLabel->setText("Friend added!");
        ui->friendAddEdit->setText("");
    }
}
void Friends::handleUpdate(QString online, QString offline){
    onFriend = online.split('\t');
    offFriend = offline.split('\t');
    int row = 0;
    ui->friendTable->clear();
    ui->friendTable->setRowCount(0);


    for(int i = 0; i < onFriend.length(); i++){
        if(onFriend[i].length() == 0)
            continue;
        ui->friendTable->insertRow(row);
        ui->friendTable->setItem(row,0,new QTableWidgetItem(onFriend[i]));
        ui->friendTable->item(row,0)->setBackground(Qt::green);
        bool found = false;
        for(int j = 0; j < convoList.length(); j++){
            if(convoList[j].name == onFriend[i]){
                convoList[j].onLine = true;
                found = true;
            }
        }
        if(!found){
            friendData f;
            f.name = onFriend[i];
            f.convo = "";
            f.unKnown = false;
            convoList.push_back(f);
        }
        row++;
        int remOld = -1;
        for(int j = 0; j < convoList.length(); j++){
            if(convoList[j].unKnown){
                if(convoList[j].name == onFriend[i]){
                    remOld = j;
                }
            }
        }
        if(remOld != -1){
            friendData f;
            f.name = convoList[remOld].name;
            f.onLine = convoList[remOld].onLine;
            f.unKnown = false;
            f.unseen = false;
            QStringList oldConvo = convoList[remOld].convo.split('\n');
            for(int p = 1; p < oldConvo.length(); p++){
                f.convo.push_back(oldConvo[p]);
                f.convo.push_back('\n');
            }
            if(f.convo.length() < 0)
                f.convo = f.convo.left(f.convo.length()-1);
            convoList.removeAt(remOld);
            convoList.push_back(f);


        }

    }
    for(int i = 0; i < offFriend.length(); i++){
        if(offFriend[i].length() == 0)
            continue;
        ui->friendTable->insertRow(row);
        ui->friendTable->setItem(row,0,new QTableWidgetItem(offFriend[i]));
        row++;
        bool found = false;
        for(int j = 0; j < convoList.length(); j++){
            if(convoList[j].name == offFriend[i]){
                convoList[j].onLine = false;
                found = true;

            }
        }
        if(!found){
            friendData f;
            f.name = offFriend[i];
            f.convo = "";
            f.unKnown = false;
            convoList.push_back(f);
        }
    }
    for(int i = 0; i < convoList.length(); i++){
        if(convoList[i].unKnown == false)
            continue;
        ui->friendTable->insertRow(row);
        ui->friendTable->setItem(row,0,new QTableWidgetItem(convoList[i].name));
        ui->friendTable->item(row,0)->setBackground(Qt::blue);

        row++;
    }
}

void Friends::setBasicTableStuff(){
    ui->friendTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->friendTable->setColumnCount(1);
    ui->friendTable->horizontalHeader()->setStretchLastSection(true);
    ui->friendTable->verticalHeader()->hide();
    ui->friendTable->horizontalHeader()->hide();

}

void Friends::on_friendRemoveButton_clicked()
{
    ui->friendErrorLabel->setText("");
    QString name = ui->friendRemoveEdit->text();
    if(name == mw->loggedInName){
        ui->friendErrorLabel->setText("You can add yourself");
        ui->friendRemoveEdit->setText("");
    }
    if(name.length() == 0)
        return;
    if(name.length() > 100)
        return;
    manager->sendFriendRemove(name);
}

void Friends::on_friendRemoveEdit_returnPressed()
{
    on_friendRemoveButton_clicked();
}
void Friends::handleFriendRemove(int code){
    if(code == 1){
        ui->friendRemoveEdit->setText("");
        ui->friendErrorLabel->setText("Friend removed");
    }else{
        ui->friendErrorLabel->setText("Error removing friend - invalid user");
    }
}

void Friends::on_frinedsChatEdit_returnPressed()
{
    QString message = ui->frinedsChatEdit->text();
    if(message.length() == 0)
        return;
    if(message.length() > 500)
        message = message.left(500);
    int row = ui->friendTable->currentIndex().row();
    if(row < 0)
        return;
    QString friendName = ui->friendTable->item(row,0)->text();
    manager->sendPm(message, friendName);
    ui->frinedsChatEdit->setText("");
}
void Friends::handlePm(QString friendName, QString message){
    int row = ui->friendTable->currentIndex().row();
    QString currentFriend = "";
    if(row >= 0){
        currentFriend = ui->friendTable->item(row,0)->text();
    }

    int index = -1;
    for(int i = 0; i < convoList.length(); i++){
        if(convoList[i].name == friendName){
            index = i;
            break;
        }
    }
    if(index != -1){
        QString fullMessage =  friendName;
        fullMessage += "> ";
        fullMessage += message;
        convoList[index].convo += fullMessage;
        convoList[index].convo += '\n';
        if(friendName == currentFriend){
            ui->friendsChatBox->setText(convoList[index].convo);
        }
        else{
            convoList[index].unseen = true;
            ui->friendTable->item(index,0)->setBackground(Qt::darkGreen);
        }
    }
    else{
        friendData f;
        f.name = friendName;
        f.convo = "Unknown user messaged you. Add them to chat.";
        f.convo += '\n';
        QString fullMessage =  friendName;
        fullMessage += "> ";
        fullMessage += message;
        f.convo += fullMessage;
        f.convo += '\n';
        f.unKnown = true;
        convoList.push_back(f);
        handleUpdate("","");
    }
}
void Friends::handleSentPm(QString friendName, QString message){
    int index = -1;
    int row = ui->friendTable->currentIndex().row();
    QString currentFriend = "";
    if(row >= 0){
        currentFriend = ui->friendTable->item(row,0)->text();
    }
    for(int i = 0; i < convoList.length(); i++){
        if(convoList[i].name == friendName){
            index = i;
            break;
        }
    }
    if(index != -1){
        QString fullMessage = ourName;
        fullMessage += "> ";
        fullMessage += message;
        convoList[index].convo += fullMessage;
        convoList[index].convo += '\n';

        if(friendName == currentFriend){
            ui->friendsChatBox->setText(convoList[index].convo);
        }
        else{

            ui->friendTable->item(index,0)->setBackground(Qt::darkGreen);
        }
    }

}

void Friends::on_friendTable_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString name = ui->friendTable->item(row,0)->text();
    for(int i = 0; i < convoList.length(); i++){
        if(convoList[i].name == name){
            ui->friendsChatBox->setText(convoList[i].convo);
            convoList[i].unseen = false;
            if(convoList[i].onLine){
                ui->friendTable->item(i,0)->setBackground(Qt::green);
            }else{
                 ui->friendTable->item(i,0)->setBackground(Qt::white);
            }
        }

    }
}
void Friends::setOurName(QString name){
    ourName = name;
}
