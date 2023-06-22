#include "form.h"
#include "ui_form.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

form::form(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::form)
{
    ui->setupUi(this);

    PopupPeriod = 60;

    QString SettingsName = "config.ini";
    QSettings settings(SettingsName, QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    if(!QFile::exists(SettingsName))    {
        settings.beginGroup("MAIN_SETTINGS");
        settings.setValue("sending","true");
        settings.setValue("server","smtp.mail.ru");
        settings.setValue("port",465);
        settings.setValue("login","");
        settings.setValue("password","");
        settings.setValue("note_period","60");
        settings.setValue("system_name","Планировщик задач");
        settings.setValue("sender_name","Имя не указано");
        settings.endGroup();
    }
    settings.beginGroup("MAIN_SETTINGS");
    m_spam        = (settings.value("sending", false)).toBool();
    m_smtp_server = (settings.value("server", "smtp.mail.ru")).toString();
    m_smtp_port   = (settings.value("port", 465)).toInt();
    m_login       = (settings.value("login", "")).toString();
    m_password    = (settings.value("password", "")).toString();
    PopupPeriod   = (settings.value("note_period", 60)).toInt();
    m_system_name = (settings.value("system_name", "Планировщик задач")).toString();
    m_sender_name = (settings.value("sender_name", "Имя не указано")).toString();
    settings.endGroup();

    if(m_spam == true)  {
        if(m_login.isEmpty() || m_password.isEmpty())   {
            QMessageBox::warning(this, "Внимание!", "Настройте параметры сети в config.ini. Приложение будет закрыто.", QMessageBox::Yes);
            exit(0);
        }
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
    //timer->setInterval(300*1000);
    timer->setInterval(30*1000);
    timer->start();

    timer_notification = new QTimer();
    connect(timer_notification,&QTimer::timeout,this,[&](){
        pop->setPopupText("У вас есть незавершённые задачи,\n запланированные на сегодня. \n Проверьте, чтоб не забыть!");
        pop->show();
    });
    timer_notification->setInterval(PopupPeriod*1000);
    timer_notification->start();

    randomMessages.append("Говорят, что чёрные кошки к несчастью\n Наверное, у этих глупышей просто не было чёрной кошки");

    checkDateForSendEmail();

    this->setWindowIcon(QIcon(":/images/app.ico"));

    writeLog(" ============ Начало работы =============");
    qDebug() << QSslSocket::supportsSsl() << QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();

    refreshTable();
}

form::~form()
{
    delete ui;

    writeLog(" ============ Конец работы ==============");
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
            bNeedToEMail = !sendEmail();
        }
            // если сообщение отправилось, то меняем дату
        if(bNeedToEMail == false)   {
            QFile file("current.dt");
            file.open(QIODevice::WriteOnly);
            QDate dt = QDate::currentDate();
            QTextStream out(&file);
            out.setCodec(QTextCodec::codecForName("UTF-8"));
            out << QString("%1.%2.%3").arg(dt.day()).arg(dt.month()).arg(dt.year());
            file.close();
        }
    } else {

    }
}

void form::OnTimer()
{
    if(EditorFrom.events_list_main.size() > 0)  {
        checkDateForSendEmail();
        refreshTable();
    }    
}

void form::checkDateForSendEmail()
{
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
                //QFile file("current.dt");
                //file.open(QIODevice::WriteOnly);
                //QDate dt = QDate::currentDate();
                //QTextStream out(&file);
                //out.setCodec(QTextCodec::codecForName("UTF-8"));
                //out << QString("%1.%2.%3").arg(dt.day()).arg(dt.month()).arg(dt.year());
                //file.close();
                bNeedToEMail = true;
            }
        } else {
            bNeedToEMail = true;
        }
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

bool form::sendEmail()
{
    if(m_spam == false) {
        return false;
    }

    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://www.google.com"));
    QNetworkReply *reply = nam.get(req);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if(!reply->bytesAvailable())    {
        writeLog("Отсутствует интернет соединение. Рассылка не отправлена.");
        return false;
    }

    if(RecieversList.recievers.size() <= 0) {
        writeLog("Нет ни одного получателя. Сообщение не отправлено.");
        return false;
    }

    EmailAddress sender = EmailAddress(m_login, m_system_name);
    QString subject = "Задачи на сегодня";

    QDate d = QDate::currentDate();
    date_t dt = date_t(d.day(),d.month(),d.year());

    QString html;
    html += "<h2><b> Перечень запланированных задач на сегодня: </b></h2><p><ul>";
    for(int i = 0; i < events_list_current.size(); i++) {
        if(dt > events_list_current[i].Date)    {
            html += "<p><h5><font color=FF0000> !!!ПРОСРОЧЕНО!!! - " + events_list_current[i].Text + "</font></h5>";
        } else {
            html += "<p><h5><font color=000000>" + events_list_current[i].Text + "</font></h5>";
        }
    }
    html += "</ul>";
    html += "<h4>Примечание:</h4> Отправитель <b>\"" + m_sender_name + "\"</b>";

    SmtpClient smtp(m_smtp_server, m_smtp_port, SmtpClient::SslConnection);
    MimeMessage message;
    message.setSender(sender);
    message.setSubject(subject);
    for(int i = 0; i < RecieversList.recievers.size(); i++) {
        message.addRecipient(EmailAddress(RecieversList.recievers[i]));
    }
    MimeHtml content;
    content.setHtml(html);
    message.addPart(&content);

    /*
    QList<QFile*> files;
    QList<MimeAttachment*> attachments;
    for (int i = 0; i < ui->attachments->count(); ++i)
    {
        QFile* file = new QFile(ui->attachments->item(i)->text());
        files.append(file);

        MimeAttachment* attachment = new MimeAttachment(file);
        attachments.append(attachment);

        message.addPart(attachment);
    }
    */

    writeLog(QString("Отправляю с %1 по %2 на %3 порт").arg(m_login).arg(m_smtp_server).arg(m_smtp_port));
    //writeLog(m_password);
    //writeLog(m_smtp_server);
    //writeLog(QString("%1").arg(m_smtp_port));

    smtp.connectToHost();
    if (!smtp.waitForReadyConnected())
    {
        writeLog("Ошибка соединения SMTP");
        return false;
    }

    smtp.login(m_login, m_password);
    if (!smtp.waitForAuthenticated())
    {
        writeLog("Ошибка аутентификации");
        return false;
    }

    smtp.sendMail(message);
    if (!smtp.waitForMailSent())
    {
        writeLog("Ошибка отправки сообщения");
        return false;
    }
    else
    {
        writeLog("Сообщение успешно отправлено");
    }

    smtp.quit();

/*
    for (auto file : files) {
        delete file;
    }

    for (auto attachment : attachments) {
        delete attachment;
    }
*/
    return true;
}

void form::writeLog(QString message)
{
    qDebug() << message;
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
