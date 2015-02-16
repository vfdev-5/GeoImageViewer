#ifndef COLORPICKERFRAME_H
#define COLORPICKERFRAME_H


// Qt
#include <QFrame>
#include <QToolButton>
#include <QPushButton>
#include <QPaintEvent>

// Project
#include "Core/LibExport.h"

class QGridLayout;

namespace Gui
{

//*************************************************************************

class ColorPickerButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ColorPickerButton(const QColor &c, QWidget *parent=0);

    bool isSelected() const
    { return !autoRaise(); }

    void setSelected(bool v)
    { setAutoRaise(!v); }

    QColor getColor() const
    { return _color; }

protected:
    virtual void paintEvent(QPaintEvent * event);
    QColor _color;

};


class GIV_DLL_EXPORT ColorPickerFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ColorPickerFrame(QWidget *parent = 0);

    QColor getCurrentColor() const
    { return _currentColor->getColor(); }

protected slots:
    void onColorPickerButtonClicked();
    void onMoreColorsClicked();

signals:
    void colorPicked(QColor);

private:

    void setColorAsCurrent(ColorPickerButton * b);

    QPushButton _moreColors;
    ColorPickerButton * _currentColor;
    int _cols;
    QGridLayout * _colorGrid;

};

//*************************************************************************

} // namespace Gui

#endif // COLORPICKERFRAME_H
