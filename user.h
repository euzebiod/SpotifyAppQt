#pragma once
#include <QString>
#include <QStringList>
#include <list>
#include <map>

class User
{
public:
    User();
    void addPlayList(const QString&, const QString&);
    QString getPlayListId(const QString&);
    void clearPlayList();
    QStringList getPlayListName();

private:
    QStringList playList;
    std::map<QString, QString> mapPlayListId;
};
