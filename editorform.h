#ifndef EDITORFORM_H
#define EDITORFORM_H

#include <QWidget>
#include <QMenu>
#include <QDate>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>

#include "common.h"
#include "loadingform.h"

namespace Ui {
class editorForm;
}

class editorForm : public QWidget
{
    Q_OBJECT

public:
    explicit editorForm(QWidget *parent = nullptr);
    ~editorForm();

    QList<event_t> events_list_main;

    loadingForm LoadingForm;


signals:
    void signalEventUpdated();
    void writeLog(QString);

    void startLoading();
    void endLoading();

public slots:

    void saveTofile();
    bool loadFromFile();

    void refreshTable();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_tableWidget_cellPressed(int row, int column);

    void slotPopup(const QPoint &);
    void slotRemove();
    void slotRemoveAll();



    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::editorForm *ui;

    QString data_filename;

};

#endif // EDITORFORM_H
