#ifndef DIALOGMUSICS_H
#define DIALOGMUSICS_H

#include <QDialog>
#include <QListWidget>
#include <QComboBox>

namespace Ui {
class DialogMusics;
}

class DialogMusics : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMusics(QWidget *parent = 0);
    ~DialogMusics();
    QListWidget* getListWidgetMusics();
    QComboBox * getComboBoxPlayList();
    QPushButton * getPushButtonAddMusicPlayList();

private:
    Ui::DialogMusics *ui;
};

#endif // DIALOGMUSICS_H
