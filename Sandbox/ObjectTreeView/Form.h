#ifndef FORM_H
#define FORM_H

// Qt
#include <QWidget>
#include <QObject>

// Sandbox
#include "ui_Form.h"

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

private:
    Ui_Form * _ui;
};

#endif // FORM_H
