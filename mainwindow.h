#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QOAuth2AuthorizationCodeFlow>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QJsonDocument>

#include "dialogmusics.h"
#include "user.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public QDesktopServices
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void enabladWidgets();
public slots:
    void granted();
    void on_pushButtonPlayList_clicked();
    void on_listWidgetPlayList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_pushButtonAddPlayList_clicked();
    bool myWeb(const QUrl&);
    void on_pushButtonAddMusicPlayList();
    void authStatusChanged(QAbstractOAuth::Status);

private slots:
    void on_pushButtonExcluirMusica_clicked();
    void on_pushButtonPlay_clicked();
    void on_pushButtonSavePlayList_clicked();
    void on_pushButtonLoadPlayList_clicked();
    void on_pushButtonGrant_clicked();

    void on_pushButtonSearchMusicas_clicked();

private:
    Ui::MainWindow *ui;
    const QString clientId = "72b31277ab394a1ba275ef7ecbbd43f1";
    const QString clientSecret = "1177f2d5a1fd425d9527469de2c48166";
    QOAuth2AuthorizationCodeFlow spotify;
    QString userName;
    QString token;
    bool isGranted;
    User user;
    DialogMusics dlgMusicas;
    QWebEngineView *webView;
    QMap<QString, QString> mapMusicSearch;
    QMap<QString, QMap<QString, QString>> mapMusicsFromPlaylist;
    bool clearLists;
    QJsonDocument playListJson;
    QNetworkAccessManager *m_nam;
};

#endif // MAINWINDOW_H
