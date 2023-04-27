#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QTableWidgetItem>
#include <QStringList>

struct date_t   {
    int day;
    int month;
    int year;

    date_t() {}
    date_t(int d, int m, int y)     {
            day = d;
            month = m;
            year = y;
    }    
    bool operator > (const date_t & dt) {
        if((year*365 + month*31 + day) > (dt.year*365 + dt.month*31 + dt.day))  {
            return true;
        } else {
            return false;
        }
    }
    bool operator >= (const date_t & dt) {
        if((year*365 + month*31 + day) >= (dt.year*365 + dt.month*31 + dt.day))  {
            return true;
        } else {
            return false;
        }
    }
    bool operator < (const date_t & dt) {
        if((year*365 + month*31 + day) < (dt.year*365 + dt.month*31 + dt.day))  {
            return true;
        } else {
            return false;
        }
    }
    bool operator <= (const date_t & dt) {
        if((year*365 + month*31 + day) <= (dt.year*365 + dt.month*31 + dt.day))  {
            return true;
        } else {
            return false;
        }
    }
    bool operator != (const date_t & dt){
        if(day != dt.day ||
           month != dt.month ||
           year != dt.year)
        {
            return true;
        } else {
            return false;
        }
    }
    bool operator == (const date_t & dt){
        if(day == dt.day &&
           month == dt.month &&
           year == dt.year)
        {
            return true;
        } else {
            return false;
        }
    }
    QString toString()   {
            QString result;
            QString d, m;
            if(day < 10) d = QString("0%1").arg(day); else d = QString("%1").arg(day);
            if(month < 10) m = QString("0%1").arg(month); else m = QString("%1").arg(month);
            result = QString("%1.%2.%3").arg(d).arg(m).arg(year);
            return result;
    }
    void fromString(QString s)  {
        int mode = 0;
        QString buff = "";
        QStringList sl = s.split(".");
        day = sl[0].toInt();
        month = sl[1].toInt();
        year = sl[2].toInt();
    }
};


struct event_t  {
    QString Text;
    date_t Date;

    event_t() {}

    bool operator == (const event_t & ev)       {
        if(Text == ev.Text &&
           Date.day == ev.Date.day &&
           Date.month == ev.Date.month &&
           Date.year == ev.Date.year)
        {
            return true;
        } else {
            return false;
        }
    }
};


#endif // COMMON_H
