#ifndef FORM_H
#define FORM_H

#include <QMainWindow>
#include <QList>

#include <QDate>
#include <QTime>
#include <QTimer>
#include <QCloseEvent>
#include <QFile>
#include <QSettings>
#include <QCursor>

#include "common.h"
#include "editorform.h"
#include "popup.h"
#include "helpform.h"
#include "recieverslist.h"

#include "src/SmtpMime"
#include <QSslSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class form; }
QT_END_NAMESPACE

class form : public QMainWindow
{
    Q_OBJECT

public:
    form(QWidget *parent = nullptr);
    ~form();

    QList<event_t> events_list_current;

    editorForm EditorFrom;
    helpForm HelpForm;
    recieversList RecieversList;

public slots:
    void refreshTable();

private slots:
    void OnTimer();

    void checkDateForSendEmail();

    void on_action_triggered();
    void on_action_2_triggered();
    void slotPopup(const QPoint &);
    void slotRemove();

    void on_tableWidget_cellPressed(int row, int column);

    bool sendEmail();

    void writeLog(QString message);

    void on_action_4_triggered();

    void on_action_3_triggered();

    void on_action_5_triggered();

private:
    Ui::form *ui;

    QTimer *timer;
    QTimer *timer_notification;
    PopUp *pop;
    void closeEvent(QCloseEvent *event);

    QList<QString> randomMessages;

    bool bNeedToEMail;

    int PopupPeriod;

    bool m_spam;
    QString m_log_name;

    QString m_smtp_server;
    qint32 m_smtp_port;
    QString m_login;
    QString m_password;
    QString m_system_name;
    QString m_sender_name;


};
#endif // FORM_H
