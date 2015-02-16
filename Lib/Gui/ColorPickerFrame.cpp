// Qt
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QColorDialog>

// Project
#include "ColorPickerFrame.h"

namespace Gui
{

//*************************************************************************

/*!

        \class ColorPickerFrame
        \ingroup Gui
        \brief This class implements simple color picker frame

        \class ColorPickerButton
        \ingroup Gui
        \brief This class inherits from QPushButton and implements a color item


 */

//*************************************************************************

/*!
    Constructor
*/
ColorPickerFrame::ColorPickerFrame(QWidget *parent) :
    QFrame(parent),
    _cols(4),
    _moreColors(tr("More Colors")),
    _currentColor(0),
    _colorGrid(0)
{

    _moreColors.setFlat(true);
    connect(&_moreColors, SIGNAL(clicked()), this, SLOT(onMoreColorsClicked()));

    QList<ColorPickerButton*> colors = QList<ColorPickerButton*>()
            << new ColorPickerButton(Qt::white)
            << new ColorPickerButton(Qt::black)
            << new ColorPickerButton(Qt::red)
            << new ColorPickerButton(Qt::darkRed)
            << new ColorPickerButton(Qt::green)
            << new ColorPickerButton(Qt::darkGreen)
            << new ColorPickerButton(Qt::blue)
            << new ColorPickerButton(Qt::darkBlue)
            << new ColorPickerButton(Qt::cyan)
            << new ColorPickerButton(Qt::darkCyan)
            << new ColorPickerButton(Qt::magenta)
            << new ColorPickerButton(Qt::darkMagenta)
            << new ColorPickerButton(Qt::yellow)
            << new ColorPickerButton(Qt::darkYellow)
            << new ColorPickerButton(Qt::gray)
            << new ColorPickerButton(Qt::darkGray)
            << new ColorPickerButton(Qt::lightGray);


    QVBoxLayout * frameLayout = new QVBoxLayout;

    _colorGrid = new QGridLayout();
    int count=0;
    int col, row;
    foreach (ColorPickerButton* c, colors)
    {
        row = count / _cols;
        col = count % _cols;
        _colorGrid->addWidget(c, row, col);
        count++;

        connect(c, SIGNAL(clicked()), this, SLOT(onColorPickerButtonClicked()));

    }

    frameLayout->addLayout(_colorGrid);
    frameLayout->addWidget(&_moreColors);

    colors[0]->setSelected(true);
    _currentColor = colors[0];

    setLayout(frameLayout);

}

//*************************************************************************

/*!
    Method to set color as current
*/
void ColorPickerFrame::setColorAsCurrent(ColorPickerButton *b)
{
    _currentColor->setSelected(false);
    b->setSelected(true);
    _currentColor = b;

    emit colorPicked(_currentColor->getColor());
}

//*************************************************************************

/*!
    Slot to handle color picker button click
*/
void ColorPickerFrame::onColorPickerButtonClicked()
{
    ColorPickerButton * b = qobject_cast<ColorPickerButton*>(sender());
    if (!b)
        return;

    setColorAsCurrent(b);
}

//*************************************************************************

/*!
    Slot to handle more colors button click
*/
void ColorPickerFrame::onMoreColorsClicked()
{
    QColorDialog d;
    if (d.exec() == QDialog::Accepted)
    {
        // create new ColorPickerButton for the new color
        QColor c = d.currentColor();
        ColorPickerButton * b = new ColorPickerButton(c);
        connect(b, SIGNAL(clicked()), this, SLOT(onColorPickerButtonClicked()));
        int count = _colorGrid->count();
        int row = count / _cols;
        int col = count % _cols;
        _colorGrid->addWidget(b, row, col);

        setColorAsCurrent(b);

    }
}

//*************************************************************************
//*************************************************************************

/*!
    Constructor
*/
ColorPickerButton::ColorPickerButton(const QColor & c, QWidget *parent):
    QToolButton(parent),
    _color(c)
{
    setAutoRaise(true);
}

//*************************************************************************

/*!
    \overload
*/
void ColorPickerButton::paintEvent(QPaintEvent* event)
{
    QToolButton::paintEvent(event);

    QRectF r(QPoint(0,0), size());
    QPainter p;
    p.begin(this);
    p.setBrush(_color);
    p.drawRect(r.adjusted(r.width()*0.2,r.height()*0.2, -r.width()*0.2, -r.height()*0.2));
    p.end();

}

//*************************************************************************

} // namespace Gui

