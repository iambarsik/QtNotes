#ifndef RECIEVERSLIST_H
#define RECIEVERSLIST_H

#include <QWidget>
#include <QList>

namespace Ui {
class recieversList;
}

class recieversList : public QWidget
{
    Q_OBJECT

public:
    explicit recieversList(QWidget *parent = nullptr);
    ~recieversList();

    QList<QString> recievers;

signals:
    void writeLog(QString);

private slots:
    void listUpdate();
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::recieversList *ui;

};

#endif // RECIEVERSLIST_H
