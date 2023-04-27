#include "helpform.h"
#include "ui_helpform.h"
#include <QIcon>

#include <QDir>

helpForm::helpForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::helpForm)
{
    ui->setupUi(this);

    ui->textBrowser->setSource(QUrl("help.html"));
    this->setWindowIcon(QIcon(":/images/icon.ico"));
}

helpForm::~helpForm()
{
    delete ui;
}
