#include "form.h"
#include "ui_form.h"

form::form(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::form)
{
    ui->setupUi(this);

    QString SettingsName = "config.ini";
    QSettings settings(SettingsName, QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    if(!QFile::exists(SettingsName))    {
        settings.beginGroup("MAIN_SETTINGS");
        settings.setValue("server","smtp.mail.ru");
        settings.setValue("port",465);
        settings.setValue("login","");
        settings.setValue("password","");
        settings.endGroup();
    }
    settings.beginGroup("MAIN_SETTINGS");
    m_smtp_server = (settings.value("server", "smtp.mail.ru")).toString();
    m_smtp_port   = (settings.value("port", 465)).toInt();
    m_login       = (settings.value("login", "")).toString();
    m_password    = (settings.value("password", "")).toString();
    settings.endGroup();

    if(m_login.isEmpty() || m_password.isEmpty())   {
        QMessageBox::warning(this, "Внимание!", "Настройте параметры сети в config.ini. Приложение будет закрыто.", QMessageBox::Yes);
        exit(0);
    }

    QDate log_date = QDate::currentDate();
    m_log_name = QString("log_%1-%2-%3").arg(log_date.day()).arg(log_date.month()).arg(log_date.year());

    this->setWindowTitle("Актуальные события");
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&EditorFrom,&editorForm::signalEventUpdated,this,&form::refreshTable);
    connect(this,&QTableWidget::customContextMenuRequested,this, &form::slotPopup);
    connect(&EditorFrom,&editorForm::writeLog,this,&form::writeLog);

    pop = new PopUp();
    connect(pop,&PopUp::popupClicked,this,[&](){
        this->showNormal();
        this->activateWindow();
    });

    timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&form::OnTimer);
    timer->setInterval(360000);
    //timer->setInterval(30000);
    timer->start();

    randomMessages.append("Говорят, что чёрные кошки к несчастью\n Наверное, у этих глупышей просто не было чёрной кошки");

    if(!QFile::exists("current.dt"))    {
        bNeedToEMail = true;
        QFile file("current.dt");
        file.open(QIODevice::WriteOnly);
        QDate dt = QDate::currentDate();
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        out << QString("%1.%2.%3").arg(dt.day()).arg(dt.month()).arg(dt.year());
        file.close();
    } else {
        QFile file;
        file.setFileName("current.dt");
        if(file.open(QFile::ReadOnly))  {
            QTextStream out(&file);
            out.setCodec(QTextCodec::codecForName("UTF-8"));
            QString line;
            out.readLineInto(&line);
            file.close();
            date_t d;
            d.fromString(line);
            QDate dt = QDate::currentDate();
            if(d.day == dt.day() &&
               d.month == dt.month() &&
               d.year == dt.year())
            {
                bNeedToEMail = false;
            } else {
                bNeedToEMail = true;
            }

        } else {
            bNeedToEMail = true;
        }
    }    
    this->setWindowIcon(QIcon(":/images/icon.ico"));

    writeLog(" ============ Начало работы =============");

    refreshTable();

}

form::~form()
{
    delete ui;
}

void form::refreshTable()
{
    QDate d = QDate::currentDate();
    date_t dt = date_t(d.day(),d.month(),d.year());

    events_list_current.clear();

    for(int i = 0; i < EditorFrom.events_list_main.size(); i++) {
        if(dt >= EditorFrom.events_list_main[i].Date)   {
            events_list_current.push_back(EditorFrom.events_list_main[i]);
        }
    }

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(events_list_current.size());
    ui->tableWidget->setColumnCount(2);
    for(int i = 0; i < events_list_current.size(); i++) {
        QTableWidgetItem *e = new QTableWidgetItem(events_list_current[i].Text);
        QTableWidgetItem *d = new QTableWidgetItem(events_list_current[i].Date.toString());
        ui->tableWidget->setItem(i,0,e);
        ui->tableWidget->setItem(i,1,d);
        if(dt == events_list_current[i].Date)    {
            ui->tableWidget->item(i,0)->setBackground(Qt::gray);
            ui->tableWidget->item(i,1)->setBackground(Qt::gray);
        }
        if(dt > events_list_current[i].Date)    {
            ui->tableWidget->item(i,0)->setBackground(Qt::red);
            ui->tableWidget->item(i,1)->setBackground(Qt::red);
        }
    }
    ui->tableWidget->setColumnWidth(0,400);
    ui->tableWidget->setColumnWidth(1,180);

    if(bNeedToEMail == true)    {
        if(events_list_current.size() > 0)  {
            sendEmail();
        }
        bNeedToEMail = false;
        QFile file("current.dt");
        file.open(QIODevice::WriteOnly);
        QDate dt = QDate::currentDate();
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        out << QString("%1.%2.%3").arg(dt.day()).arg(dt.month()).arg(dt.year());
        file.close();
    }
}

void form::OnTimer()
{
    if(EditorFrom.events_list_main.size() > 0)  {
        refreshTable();
    }
    if(events_list_current.size() > 0)  {
        pop->setPopupText("У вас есть незавершённые задачи,\n запланированные на сегодня. \n Проверьте, чтоб не забыть!");
        pop->show();
    }

}


void form::on_action_triggered()
{
    EditorFrom.show();
}

void form::slotPopup(const QPoint & pos)
{
    QMenu *menu = new QMenu(this);
    QAction * ac1 = new QAction("Удалить пункт", this);
    QAction * ac2 = new QAction("Отправить на Email", this);
    QAction * ac3 = new QAction("(резерв)", this);

    menu->addAction(ac1);
    connect(ac1, &QAction::triggered, this, &form::slotRemove);
    connect(ac2, &QAction::triggered, this, &form::sendEmail);
    menu->addAction(ac2);
    menu->addAction(ac3);
    QPoint p = pos;
    p.setY(pos.y() - 20);
    menu->popup(ui->tableWidget->viewport()->mapToGlobal(p));
}

void form::slotRemove()
{
    if(ui->tableWidget->currentRow() > -1)  {
        event_t e = events_list_current[ui->tableWidget->currentRow()];
        for(int i = 0; i < EditorFrom.events_list_main.size();i++)  {
            if(e == EditorFrom.events_list_main[i]) {
                EditorFrom.events_list_main.removeOne(EditorFrom.events_list_main[i]);
            }
        }
        ui->label->setText("");
        refreshTable();
        EditorFrom.refreshTable();
    }
}

void form::on_tableWidget_cellPressed(int row, int column)
{
    ui->tableWidget->selectRow(row);
    ui->label->setText(ui->tableWidget->item(row,0)->text());

    Q_UNUSED(column);
}

void form::on_action_2_triggered()
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Внимание!",
                                                                    tr("Закрыть приложение?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {

    } else {
        EditorFrom.saveTofile();
        EditorFrom.close();
        HelpForm.close();
        RecieversList.close();
        exit(0);
    }
}

void form::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Внимание!",
                                                                    tr("Закрыть приложение?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        EditorFrom.saveTofile();
        EditorFrom.close();
        HelpForm.close();
        RecieversList.close();
        event->accept();
    }
}


void form::sendEmail()
{
    if(RecieversList.recievers.size() <= 0)
        return;


    for(int i = 0; i < RecieversList.recievers.size(); i++) {


            // Create a MimeMessage

        MimeMessage message;

        EmailAddress sender(m_login, "Планировщик");
        message.setSender(sender);

        EmailAddress to(RecieversList.recievers[i], "Получатель");
        message.addRecipient(to);

        message.setSubject("Задачи на сегодня");

            // Add some text
        MimeText text;

        QString str;
        for(int i = 0; i < events_list_current.size(); i++) {
            str += events_list_current[i].Text + "\n";
        }

        text.setText("Перечень задач:\n\n " + str);


        message.addPart(&text);

            // Now we create the attachment object
        //MimeAttachment attachment (new QFile(QCoreApplication::applicationDirPath() + "/Octalife.jpg"));

            // the file type can be setted. (by default is application/octet-stream)
        //attachment.setContentType("image/jpg");

            // Now add it to message
        //message.addPart(&attachment);

            // Add an another attachment
            // MimeAttachment document(new QFile("document.pdf"));
            // message.addPart(&document);

             // Now we can send the mail

        SmtpClient smtp(m_smtp_server, m_smtp_port, SmtpClient::SslConnection);

        smtp.connectToHost();
        if (!smtp.waitForReadyConnected()) {
            writeLog("Failed to connect to host!");
            return;
        }
        smtp.login(m_login,m_password);
        if (!smtp.waitForAuthenticated()) {
            writeLog("Failed to login!");
            return;
        }
        smtp.sendMail(message);
        if (!smtp.waitForMailSent()) {
            writeLog("Failed to send a mail!");
            return;
        }

        smtp.quit();
        writeLog("Message is sent!");

    }
}

void form::writeLog(QString message)
{
    QString filename = QDir::currentPath() + "/log/" + m_log_name;
    QFile file(m_log_name);
    file.open(QIODevice::Append);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("UTF-8"));
    QString st = QTime::currentTime().toString();
    out << QString("%1 : %2").arg(st).arg(message) << "\n";

    file.close();
}

void form::on_action_4_triggered()
{
    QMessageBox::about(this,"О программе...","Планировщик задач.\nВерсия 1.0\nАвтор: gunstaria@mail.ru");
}

void form::on_action_3_triggered()
{
    HelpForm.show();
}

void form::on_action_5_triggered()
{
    RecieversList.show();
}
