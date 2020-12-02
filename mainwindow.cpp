#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogmusics.h"

#include <QDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QtWebView/QtWebView>
#include <QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QStringList>
#include <QByteArray>
#include <QFileDialog>
#include <QNetworkRequest>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    webView(NULL),
    m_nam(new QNetworkAccessManager)
{
    ui->setupUi(this);

    auto replyHandler = new QOAuthHttpServerReplyHandler(49150, this);
    spotify.setReplyHandler(replyHandler);
    spotify.setAuthorizationUrl(QUrl("https://accounts.spotify.com/authorize"));
    spotify.setAccessTokenUrl(QUrl("https://accounts.spotify.com/api/token"));
    spotify.setClientIdentifier(clientId);
    spotify.setClientIdentifierSharedKey(clientSecret);
    spotify.setScope("user-read-private user-top-read playlist-read-private playlist-modify-public playlist-modify-private user-read-currently-playing  user-read-playback-state user-modify-playback-state");

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,this, &MainWindow::myWeb);

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::statusChanged,
            this, &MainWindow::authStatusChanged);

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::granted,
            this, &MainWindow::granted);

    connect(dlgMusicas.getPushButtonAddMusicPlayList(), SIGNAL(clicked()),this,
            SLOT(on_pushButtonAddMusicPlayList()));

}

bool MainWindow::myWeb(const QUrl &url)
{
    if(!webView) delete webView;
    webView = new QWebEngineView;
    webView->load(url);
    webView->show();

    QString plainText = "Url de  autorização: " + url.toString() + "\n";
    ui->plainTextEditOutSpotity->appendPlainText(plainText.toUtf8());
    return true;
}

void MainWindow::on_pushButtonAddMusicPlayList()
{
    if(dlgMusicas.getComboBoxPlayList()->count() == 0) {
        QMessageBox msgBox;
        msgBox.setText("Recupere primeiro a playlist");
        msgBox.exec();
        return;
    }

    QString playListSelected = user.getPlayListId(dlgMusicas.getComboBoxPlayList()->currentText());
    QModelIndex index = dlgMusicas.getListWidgetMusics()->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    QString playList = "https://api.spotify.com/v1/users/"+ userName + "/playlists/" + playListSelected + "/tracks?uris=" +
            mapMusicSearch[itemText];

    QNetworkRequest*  request = new QNetworkRequest(QUrl(playList));
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request->setRawHeader("Accept", "application/json");
    request->setRawHeader("Content-length", 0);
    QString autori = "Bearer " + token;
    request->setRawHeader("Authorization", autori.toUtf8());

    QNetworkReply *reply = m_nam->post(*request,"");

    connect(reply, &QNetworkReply::finished, this, [this, reply] () {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }
        reply->deleteLater();
        auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(dlgMusicas.getComboBoxPlayList()->currentText());
        on_listWidgetPlayList_currentItemChanged(item, NULL);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::granted ()
{
    token = spotify.token();
}

void MainWindow::authStatusChanged(QAbstractOAuth::Status status)
{
    if (status == QAbstractOAuth::Status::Granted){
        QUrl u ("https://api.spotify.com/v1/me");

        ui->plainTextEditOutSpotity->appendPlainText(QString("EndPoint Dados Usuario: " + u.toString() + "\n").toUtf8());
        auto reply = spotify.get(u);

        connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() != QNetworkReply::NoError) {
                QMessageBox msgBox;
                msgBox.setText("Erro de conexão");
                msgBox.exec();
                return;
            }
            const auto data = reply->readAll();

            const auto document = QJsonDocument::fromJson(data);
            const auto root = document.object();
            userName = root.value("id").toString();
            QString displayName = root.value("display_name").toString();

            webView->close();

            QMessageBox msgBox(this);
            msgBox.setText("Usuario " + displayName + " Autorizado");
            msgBox.exec();
            reply->deleteLater();
            if(ui->listWidgetPlayList->count() > 0){
                on_pushButtonPlayList_clicked();
            }
            enabladWidgets();

        });
    }
}
void MainWindow::enabladWidgets(){
    ui->lineEditSearch->setEnabled(true);
    ui->pushButtonSearchMusicas->setEnabled(true);
    ui->pushButtonPlayList->setEnabled(true);
    ui->pushButtonSavePlayList->setEnabled(true);
    ui->pushButtonAddPlayList->setEnabled(true);
    ui->lineEditAddPlayList->setEnabled(true);
    ui->pushButtonPlay->setEnabled(true);
    ui->pushButtonExcluirMusica->setEnabled(true);
}

void MainWindow::on_pushButtonPlayList_clicked()
{
    if (userName.length() == 0) return;

    clearLists = true;
    ui->listWidgetPlayList->clear();
    ui->listWidgetMusics->clear();
    clearLists = false;

    user.clearPlayList();

    QUrl u ("https://api.spotify.com/v1/users/" + userName + "/playlists");
    ui->plainTextEditOutSpotity->appendPlainText(QString("EndPoint Listar Playlists: " + u.toString() + "\n").toUtf8());
    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText("Erro de conexão");
            msgBox.exec();
            return;
        }
        const auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);

        playListJson = QJsonDocument::fromJson(data);
        const auto root = playListJson.object();
        const auto jsonArray = root["items"].toArray();

        foreach (const QJsonValue & value, jsonArray) {
            auto obj = value.toObject();
            ui->listWidgetPlayList->addItem(obj.value("name").toString());
            user.addPlayList(obj.value("name").toString(), obj.value("id").toString());
        }
        reply->deleteLater();
        dlgMusicas.getComboBoxPlayList()->clear();
        dlgMusicas.getComboBoxPlayList()->addItems(user.getPlayListName());
    });
}

void MainWindow::on_listWidgetPlayList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if(clearLists) return;

    mapMusicsFromPlaylist.clear();
    if (userName.length() == 0) return;
    ui->listWidgetMusics->clear();

    QUrl u ("https://api.spotify.com/v1/users/" + userName + "/playlists/"
            + user.getPlayListId(current->text()) + "/tracks");
    ui->plainTextEditOutSpotity->appendPlainText(QString("EndPoint Musicas de uma Playlist: " + u.toString() + "\n").toUtf8());
    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }

        const auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        const auto jsonArray = root["items"].toArray();

        foreach (const QJsonValue & value, jsonArray) {
            QString music;
            const auto obj = value.toObject();
            const auto track = obj.value("track").toObject();
            music = track.value("name").toString() + " - ";
            QString musicId = track.value("uri").toString();

            const auto jsonArrayArtists = track.value("artists").toArray();
            foreach (const QJsonValue & value, jsonArrayArtists){
                const auto artists = value.toObject();
                music += artists.value("name").toString() + " ";
            }
            ui->listWidgetMusics->addItem(music);
            mapMusicsFromPlaylist[current->text()][music] = musicId;
            music.clear();
        }
        reply->deleteLater();
    });
}


void MainWindow::on_pushButtonAddPlayList_clicked()
{
    QString uri = "https://api.spotify.com/v1/users/"+ userName + "/playlists";

    QNetworkRequest*  request = new QNetworkRequest(QUrl(uri));
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request->setRawHeader("Accept", "application/json");
    QString autori = "Bearer " + token;
    request->setRawHeader("Authorization", autori.toUtf8());

    on_pushButtonPlayList_clicked();

    QJsonObject obj{
        {"name", ui->lineEditAddPlayList->text()},
        {"description", "New playlist description"},
        {"public", "false"}
    };

    QNetworkReply *reply = m_nam->post(*request, QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] () {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }
        reply->deleteLater();
        ui->listWidgetPlayList->addItem(ui->lineEditAddPlayList->text());
        auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);
    });
}

void MainWindow::on_pushButtonExcluirMusica_clicked()
{
    QString playListSelectedId = user.getPlayListId(ui->listWidgetPlayList->currentItem()->text());
    QString musicSelectedUri = mapMusicsFromPlaylist[ui->listWidgetPlayList->currentItem()->text()][ui->listWidgetMusics->currentItem()->text()];

    QString endPoint = "https://api.spotify.com/v1/users/"+ userName + "/playlists/" + playListSelectedId + "/tracks";
    QNetworkRequest*  request = new QNetworkRequest(QUrl(endPoint));
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request->setRawHeader("Accept", "application/json");
    QString autori = "Bearer " + token;
    request->setRawHeader("Authorization", autori.toUtf8());

    QJsonObject obj;
    QJsonObject uris;
    uris.insert("uri", musicSelectedUri);
    QJsonArray arrayUri;
    arrayUri.append(uris);
    obj.insert("tracks", arrayUri );

    QNetworkReply *reply = m_nam->sendCustomRequest(*request, "DELETE" , QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] () {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }
        reply->deleteLater();
        auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);
    });

    delete ui->listWidgetMusics->takeItem(ui->listWidgetMusics->row(ui->listWidgetMusics->currentItem()));
}

void MainWindow::on_pushButtonPlay_clicked()
{
    QDesktopServices::openUrl(QUrl("https://open.spotify.com/browse/featured"));

    QTime dieTime= QTime::currentTime().addSecs(15);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    QUrl u ("https://api.spotify.com/v1/me/player/devices");
    ui->plainTextEditOutSpotity->appendPlainText(QString("EndPoint Para pegar os dispositivos de play: " + u.toString() + "\n").toUtf8());

    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }
        const auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        const auto jsonArray = root["devices"].toArray();
        QString  deveciId;
        foreach (const QJsonValue & value, jsonArray) {
            const auto obj = value.toObject();
            deveciId = obj.value("id").toString();
            break;
        }

        QString endpoint = "https://api.spotify.com/v1/me/player/play?device_id=" + deveciId;
        QNetworkRequest*  request = new QNetworkRequest(QUrl(endpoint));
        request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request->setRawHeader("Accept", "application/json");
        QString autori = "Bearer " + token;
        request->setRawHeader("Authorization", autori.toUtf8());

        QString musicSelectedUri = mapMusicsFromPlaylist[ui->listWidgetPlayList->currentItem()->text()][ui->listWidgetMusics->currentItem()->text()];

        QJsonObject obj;
        QJsonArray arrayUri;
        arrayUri.append(musicSelectedUri);
        obj.insert("uris", arrayUri);

        QNetworkReply *reply = m_nam->put(*request, QJsonDocument(obj).toJson());

        connect(reply, &QNetworkReply::finished, this, [this, reply] () {
            if (reply->error() != QNetworkReply::NoError) {
                QMessageBox msgBox;
                msgBox.setText(reply->errorString());
                msgBox.exec();
                return;
            }
            reply->deleteLater();
            auto data = reply->readAll();
            ui->plainTextEditOutSpotity->appendPlainText(data);
        });


    });
}

void MainWindow::on_pushButtonSavePlayList_clicked()
{

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save PlayList"), "",
                                                    tr("Address Book (*.json);;All Files (*)"));

    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(playListJson.toJson());
}

void MainWindow::on_pushButtonLoadPlayList_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open PlayList"), "",
                                                    tr("Address Book (*.json);;All Files (*)"));

    QString val;
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();

    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    playListJson = QJsonDocument::fromJson(val.toUtf8());
    QString saida = "Json Lido: \n" + val;
    ui->plainTextEditOutSpotity->appendPlainText(saida.toUtf8());

    const auto root = playListJson.object();
    const auto jsonArray = root["items"].toArray();

    foreach (const QJsonValue & value, jsonArray) {
        auto obj = value.toObject();
        ui->listWidgetPlayList->addItem(obj.value("name").toString());
        user.addPlayList(obj.value("name").toString(), obj.value("id").toString());
    }
}

void MainWindow::on_pushButtonGrant_clicked()
{
    spotify.grant();
}

void MainWindow::on_pushButtonSearchMusicas_clicked()
{
    dlgMusicas.getListWidgetMusics()->clear();
    QString search = ui->lineEditSearch->text();
    search.replace(" ", "\\%20");

    QUrl u ("https://api.spotify.com/v1/search?q=" + search + "&type=track");
    auto reply = spotify.get(u);
    ui->plainTextEditOutSpotity->appendPlainText(QString("EndPoint Buscar Musicas: " + u.toString() + "\n").toUtf8());

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox msgBox;
            msgBox.setText(reply->errorString());
            msgBox.exec();
            return;
        }

        const auto data = reply->readAll();
        ui->plainTextEditOutSpotity->appendPlainText(data);

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        const auto tracks = root["tracks"].toObject();
        const auto jsonArray = tracks["items"].toArray();

        foreach (const QJsonValue & value, jsonArray) {
            QString music;
            const auto item = value.toObject();
            music = item.value("name").toString() + " - ";

            const auto jsonArrayArtists = item.value("artists").toArray();
            foreach (const QJsonValue & value, jsonArrayArtists){
                const auto artists = value.toObject();
                music += artists.value("name").toString() + " ";
            }
            dlgMusicas.getListWidgetMusics()->addItem(music);
            mapMusicSearch[music] = item.value("uri").toString();
            music.clear();
        }

        reply->deleteLater();
    });

    dlgMusicas.show();
    dlgMusicas.getComboBoxPlayList()->clear();
    dlgMusicas.getComboBoxPlayList()->addItems(user.getPlayListName());
}
