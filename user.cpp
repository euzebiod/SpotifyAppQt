#include "user.h"
#include <QStringList>

void User::addPlayList(const QString &nome,const QString &id)
{
    playList.append(nome);
    mapPlayListId[nome] = id;
}

QString User::getPlayListId(const QString& name)
{
    return mapPlayListId[name];
}

void User::clearPlayList()
{
    playList.clear();
    mapPlayListId.clear();
}

QStringList User::getPlayListName(){
    return playList;
}

User::User()
{

}
