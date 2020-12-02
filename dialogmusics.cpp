#include "dialogmusics.h"
#include "ui_dialogmusics.h"

DialogMusics::DialogMusics(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMusics)
{
    ui->setupUi(this);
}

DialogMusics::~DialogMusics()
{
    delete ui;
}

QListWidget *DialogMusics::getListWidgetMusics()
{
    return ui->listWidgetMusics;
}

QComboBox *DialogMusics::getComboBoxPlayList()
{
    return ui->comboBoxPlayList;
}

QPushButton *DialogMusics::getPushButtonAddMusicPlayList()
{
    return ui->pushButtonAddMusicPlayList;
}
