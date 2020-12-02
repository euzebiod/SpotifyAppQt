#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <qthread.h>
#include <QMessageBox>
#include <QResource>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) { QThread::sleep(secs); }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QPixmap pixmap(":/new/prefix1/spotifyQt.jpg");

    if(pixmap.isNull())
    {
        QMessageBox::warning(0, "Error", "Failed to load Splash Screen image!");
    }

    QSplashScreen splash(pixmap);
    splash.show();

    splash.showMessage(QObject::tr("Initiating your program now..."),
                    Qt::AlignLeft | Qt::AlignBottom, Qt::black);

    MainWindow w;

    I::sleep(5);

    splash.finish(&w);

    w.show();

    return a.exec();
}
