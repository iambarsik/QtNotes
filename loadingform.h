#ifndef LOADINGFORM_H
#define LOADINGFORM_H

#include <QWidget>

namespace Ui {
class loadingForm;
}

class loadingForm : public QWidget
{
    Q_OBJECT

public:
    explicit loadingForm(QWidget *parent = nullptr);
    ~loadingForm();

public slots:
    void showForm() { this->show(); }
    void hideForm() { this->hide(); }

private:
    Ui::loadingForm *ui;
};

#endif // LOADINGFORM_H
