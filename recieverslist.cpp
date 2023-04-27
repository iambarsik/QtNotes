#include "recieverslist.h"
#include "ui_recieverslist.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include <QListWidgetItem>

recieversList::recieversList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::recieversList)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/images/icon.ico"));

    if(!QFile::exists("recievers.txt")) {
        QMessageBox::warning(this,"Внимание!","Не указан список получателей для рассылки",QMessageBox::Ok);
    } else {
        QFile file;
        file.setFileName("recievers.txt");
        if(file.open(QFile::ReadOnly))  {
            QTextStream out(&file);
            out.setCodec(QTextCodec::codecForName("UTF-8"));
            QString line;
            while(out.readLineInto(&line)){
                recievers.append(line);
                line.clear();
            }
            file.close();
            for(int i = 0; i < recievers.size(); i++)   {
                ui->listWidget->addItem(recievers[i]);
            }
        }
    }
}

recieversList::~recieversList()
{
    delete ui;
}

void recieversList::listUpdate()
{
    recievers.clear();
    for(int i = 0; i < ui->listWidget->count(); i++)    {
        recievers.push_back(ui->listWidget->item(i)->text());
    }
    QFile file("recievers.txt");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    for(int i = 0; i < recievers.size(); i++)   {
        out << recievers[i];
        if(i < recievers.size() - 1)
            out << "\n";
    }
    file.close();
}

void recieversList::on_pushButton_clicked()
{

}

void recieversList::on_pushButton_2_clicked()
{
    if(!ui->lineEdit->text().isEmpty()) {
        ui->listWidget->addItem(ui->lineEdit->text());
        emit writeLog(QString("Абонент %1 добавлен в список").arg(ui->lineEdit->text()));
        listUpdate();
        ui->lineEdit->clear();
    }
}

void recieversList::on_pushButton_3_clicked()
{
    if(ui->listWidget->count() <= 0 || ui->listWidget->currentRow() < 0)   {
        QMessageBox::warning(this,"Ошибка","Не выбран ни один абонент!",QMessageBox::Ok);
        return;
    }
    int index = ui->listWidget->currentRow();
    emit writeLog(QString("Абонент %1 удалён из списка").arg(ui->listWidget->item(index)->text()));
    ui->listWidget->takeItem(index);
    listUpdate();
}
