#include "editorform.h"
#include "ui_editorform.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QAxObject>

#include <QStringList>
#include <QAction>
#include <QThread>

editorForm::editorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::editorForm),
    data_filename("data_events.evs")
{
    ui->setupUi(this);

    for(int i = 1; i < 32; i++) {
        ui->cbDay->addItem(QString("%1").arg(i));
    }
    ui->cbDay->setCurrentIndex(0);

    ui->cbMonth->addItem(QString("января"));
    ui->cbMonth->addItem(QString("февраля"));
    ui->cbMonth->addItem(QString("марта"));
    ui->cbMonth->addItem(QString("апреля"));
    ui->cbMonth->addItem(QString("мая"));
    ui->cbMonth->addItem(QString("июня"));
    ui->cbMonth->addItem(QString("июля"));
    ui->cbMonth->addItem(QString("августа"));
    ui->cbMonth->addItem(QString("сентября"));
    ui->cbMonth->addItem(QString("октября"));
    ui->cbMonth->addItem(QString("ноября"));
    ui->cbMonth->addItem(QString("декабря"));
    ui->cbMonth->setCurrentIndex(0);

    ui->cbYear->addItem(QString("2023"));
    ui->cbYear->addItem(QString("2024"));
    ui->cbYear->addItem(QString("2025"));
    ui->cbYear->addItem(QString("2026"));
    ui->cbYear->addItem(QString("2027"));
    ui->cbYear->addItem(QString("2028"));
    ui->cbYear->addItem(QString("2029"));
    ui->cbYear->setCurrentIndex(0);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableWidget::customContextMenuRequested, this, &editorForm::slotPopup);
    connect(this, &editorForm::signalEventUpdated, this, &editorForm::saveTofile);
    connect(this, &editorForm::signalEventUpdated, this, &editorForm::refreshTable);

    connect(this,&editorForm::startLoading,&LoadingForm,&loadingForm::showForm);
    connect(this,&editorForm::endLoading,&LoadingForm,&loadingForm::hideForm);

    this->setWindowTitle("Редактор событий");

    QDate dt = QDate::currentDate();
    ui->cbDay->setCurrentIndex(dt.day() - 1);
    ui->cbMonth->setCurrentIndex(dt.month() - 1);
    ui->cbYear->setCurrentIndex(dt.year() - 2023);

    bool isLoadedFromFile = loadFromFile();
    if(isLoadedFromFile == false)   {
        QMessageBox::warning(this, "Внимание!", "Не найден файл сохранений. Запущен \"чистый\" режим", QMessageBox::Yes);
        emit writeLog("Запущен чистый режим");
    } else {
        emit signalEventUpdated();
    }
    this->setWindowIcon(QIcon(":/images/icon.ico"));
}

editorForm::~editorForm()
{
    delete ui;
}

void editorForm::saveTofile()
{
    QFile file;
    file.setFileName(data_filename);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    for(int i = 0; i < events_list_main.size(); i++)    {
        out << events_list_main[i].Text << "\n";
        out << events_list_main[i].Date.toString() << "\n";
        out << "reserved for time";
        if(i < events_list_main.size() - 1)
            out << "\n";
    }
    file.close();
    //emit writeLog("Сохранено в файл");
}

bool editorForm::loadFromFile()
{
    if(!QFile::exists(data_filename))
        return false;

    QFile file;
    QStringList ls;
    file.setFileName(data_filename);
    if(file.open(QFile::ReadOnly))  {
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        QString line;
        ls.clear();
        while (out.readLineInto(&line)) {
            ls.append(line.toUtf8());
            line.clear();
        }
    } else {
        return false;
    }
    file.close();

    for(int i = 0; i < ls.size(); i+=3)    {
        event_t e;
        e.Text = ls[i];
        e.Date.fromString(ls[i+1]);

        events_list_main.push_back(e);
    }
    emit writeLog("Загружено из файла");

    return true;
}

void editorForm::refreshTable()
{
    std::sort(events_list_main.begin(),events_list_main.end(),[](const event_t & e1, const event_t & e2)
    {
        return ((e1.Date.year*365 + e1.Date.month*31 + e1.Date.day) <
                (e2.Date.year*365 + e2.Date.month*31 + e2.Date.day));
    });

    ui->tableWidget->setRowCount(events_list_main.size());
    ui->tableWidget->setColumnCount(2);
    for(int i = 0; i < events_list_main.size(); i++)    {
        QTableWidgetItem *e = new QTableWidgetItem(events_list_main[i].Text);
        QTableWidgetItem *d = new QTableWidgetItem(events_list_main[i].Date.toString());
        ui->tableWidget->setItem(i,0,e);
        ui->tableWidget->setItem(i,1,d);
    }

    ui->tableWidget->setColumnWidth(0,800);
    ui->tableWidget->setColumnWidth(1,200);
}

void editorForm::on_pushButton_clicked()
{
    event_t e;
    e.Text = ui->eventEdit->text();
    e.Date = date_t(ui->cbDay->currentIndex() + 1,
                    ui->cbMonth->currentIndex() + 1,
                    ui->cbYear->currentText().toInt());

    events_list_main.push_back(e);;
    ui->eventEdit->clear();

    refreshTable();

    emit signalEventUpdated();

    emit writeLog(QString("Добавлено событие: %1 - %2").arg(e.Date.toString()).arg(e.Text));
}

void editorForm::on_pushButton_2_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Открыть файл"),
                                                    "",
                                                    tr("Excel (*.xls *.xlsx)"));
    if(!QFile::exists(file))    {
        qDebug() << "File doent exist";
        QMessageBox::warning(this,"Ошибка","Невозможно открыть файл",QMessageBox::Yes);
        return;
    }

    QThread::sleep(3);

    QAxObject* excel = new QAxObject("Excel.Application", this);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", file);
    QAxObject* sheets = workbook->querySubObject("WorkSheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);

    QAxObject* usedRange = sheet->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    int countRows = rows->property("Count").toInt();

    QAxObject* columns = usedRange->querySubObject("Columns");
    int countCols = columns->property("Count").toInt();

    qDebug() << "Row count = " << countRows;
    qDebug() << "Col count = " << countCols;

    ui->tableWidget->setRowCount(countRows);
    ui->tableWidget->setColumnCount(countCols);

    for(int row = 1; row < countRows; row++ ){
        QAxObject* e_text = sheet->querySubObject("Cells(int,int)", row + 1, 1);
        QVariant etv = e_text->property("Value");
        QAxObject* e_date = sheet->querySubObject("Cells(int,int)", row + 1, 2);
        QVariant edv = e_date->property("Text");

        qDebug() << etv.toString();
        qDebug() << edv.toString();

        event_t e;
        e.Text = etv.toString();
        date_t d;
        d.fromString(edv.toString());

        e.Date = d;

        events_list_main.push_back(e);
    }

    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");

    refreshTable();

    ui->tableWidget->setColumnWidth(0,800);
    ui->tableWidget->setColumnWidth(1,200);

    emit signalEventUpdated();

    emit writeLog("Загружен список из Excel файла : " + file);
}

void editorForm::slotPopup(const QPoint & pos)
{
    QMenu *menu = new QMenu(this);
    QAction * ac1 = new QAction("Удалить пункт", this);
    QAction * ac2 = new QAction("(резерв)", this);
    QAction * ac3 = new QAction("Очистить весь список", this);

    menu->addAction(ac1);
    connect(ac1, &QAction::triggered, this, &editorForm::slotRemove);
    connect(ac3, &QAction::triggered, this, &editorForm::slotRemoveAll);
    menu->addAction(ac2);
    menu->addAction(ac3);
    menu->popup(ui->tableWidget->viewport()->mapToGlobal(pos));
}

void editorForm::slotRemove()
{
    int index = ui->tableWidget->currentRow();
    event_t e = events_list_main.at(index);
    events_list_main.removeAt(index);
    emit signalEventUpdated();
    emit writeLog(QString("Удалено событие : %1 - %2").arg(e.Date.toString()).arg(e.Text));
}

void editorForm::slotRemoveAll()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Внимание!",
                                                                    tr("Вы уверены, что хотите удалить все задачи?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes)
        return;
    events_list_main.clear();
    emit signalEventUpdated();
    emit writeLog(QString("Очистка всего списка задач."));

}

void editorForm::on_tableWidget_cellPressed(int row, int column)
{
    ui->tableWidget->selectRow(row);
    Q_UNUSED(column);
}

void editorForm::on_tableWidget_cellChanged(int row, int column)
{
    /*
    if(row < 0 || column < 0)
        return;
    event_t e = events_list_main[row];
    switch (column) {
        case 0:
            events_list_main[row].Text = ui->tableWidget->item(row,column)->text();
        break;
        case 1:
            events_list_main[row].Date.fromString(ui->tableWidget->item(row,column)->text());
        break;
    }
    emit signalEventUpdated();
    emit writeLog(QString("Задача %1 - %2 изменена на %3 - %4").arg(e.Text).arg(e.Date.toString()).arg(events_list_main[row].Text).arg(events_list_main[row].Date.toString()));
    */
}
