#ifndef FRIENDS_H
#define FRIENDS_H
#include "mainwindow.h"
#include "multimanager.h"
#include <QDialog>

class MainWindow;
class MultiManager;
struct friendData{
    QString name;
    bool unKnown = false;
    QString convo;
    bool unseen = false;
    bool onLine = false;
};
namespace Ui {
class Friends;
}

class Friends : public QDialog
{
    Q_OBJECT

public:
    explicit Friends(QWidget *parent = 0, MainWindow * m = 0, MultiManager * man = 0);
    ~Friends();
    void setOurName(QString name);

public slots:
    void handleAdd(int code);
    void handleFriendRemove(int code);
    void handleUpdate(QString online, QString offline);
    void handlePm(QString friendName, QString message);
    void handleSentPm(QString friendName, QString message);
private slots:
    void on_friendAddbutton_clicked();

    void on_friendAddEdit_returnPressed();

    void on_friendRemoveButton_clicked();

    void on_friendRemoveEdit_returnPressed();

    void on_frinedsChatEdit_returnPressed();

    void on_friendTable_clicked(const QModelIndex &index);

private:
    Ui::Friends *ui;
    MultiManager * manager;
    MainWindow * mw;
    QStringList offFriend;
    QString ourName = "";
    QStringList onFriend;
    QList<friendData> convoList;
    void setBasicTableStuff();
};

#endif // FRIENDS_H
