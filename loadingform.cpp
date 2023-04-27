#include "loadingform.h"
#include "ui_loadingform.h"
#include <QScreen>

loadingForm::loadingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::loadingForm)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |        // Отключаем оформление окна
                   Qt::WindowStaysOnTopHint);       // Устанавливаем поверх всех окон



QScreen *screen = QApplication::screens().at(0);
    this->setGeometry(screen->availableSize().width()/2 - this->width()/2,
                      screen->availableSize().height()/2 - this->height()/2,
                      this->width(),this->height());

}



loadingForm::~loadingForm()
{
    delete ui;
}
