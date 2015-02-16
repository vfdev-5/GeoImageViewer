#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

// Qt
#include <QWidget>
#include <QStringList>
#include <QVariant>
#include <QHash>
#include <QPen>
#include <QFrame>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

class QFormLayout;
class QToolButton;
class QComboBox;
class QSpinBox;

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT PropertyEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyEditor(QWidget *parent = 0);

    void setup(QObject * );

    void setNameFilter(const QStringList & filter)
    { _filter = filter; }


protected:

    QWidget * readableWidget(const QVariant & value);
    QWidget * editableWidget(const QVariant & value);

    QStringList _filter;

protected slots:
    void onStringPropertyChanged();
    void onPenPropertyChanged();
    void onBrushPropertyChanged();

private:

    QObject * _object;
    QHash<QWidget*, int> _widgetPropertyMap;

    QWidget * _frame;

};

//******************************************************************************

class ColorEditor : public QFrame
{
    Q_OBJECT
    Q_PROPERTY_WITH_ACCESSORS(int, colorIconSize, getColorIconSize, setColorIconSize)
public:
    ColorEditor(const QColor & color, QWidget * parent = 0);
    QColor getColor() const;

signals:
    void editingFinished();

protected slots:
    void onColorIconClicked();

protected:

    QToolButton * _color;
    QColor _colorValue;

};

//******************************************************************************

class PenEditor : public ColorEditor
{
    Q_OBJECT
public:
    PenEditor(const QPen &pen, QWidget * parent = 0);
    QPen getPen() const;

protected:
    QComboBox * _style;
    QSpinBox * _width;
};

//******************************************************************************

class BrushEditor : public ColorEditor
{
    Q_OBJECT
public:
    BrushEditor(const QBrush &brush, QWidget * parent = 0);
    QBrush getBrush() const;

protected:

};

//******************************************************************************

QIcon createIconFromColor(const QColor & c, int size);

}

#endif // PROPERTYEDITOR_H
