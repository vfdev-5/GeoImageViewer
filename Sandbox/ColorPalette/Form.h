#ifndef FORM_H
#define FORM_H


// Qt
#include <QWidget>
#include <QGraphicsScene>
#include <QShowEvent>
#include <QResizeEvent>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

protected:
    void showEvent(QShowEvent * event);
    void resizeEvent(QResizeEvent * event);
private:
    Ui::Form *ui;

    QGraphicsScene _scene;
};

#endif // FORM_H
