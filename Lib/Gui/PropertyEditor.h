#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

// Qt
#include <QWidget>
#include <QStringList>
#include <QVariant>
#include <QHash>
#include <QPen>
#include <QFrame>
//#include <QSignalMapper>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"

class QFormLayout;
class QToolButton;
class QComboBox;
class QSpinBox;
class QScrollArea;
class QMetaObject;

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

    QObject * getObject()
    { return _object; }


protected:

    QWidget * readableWidget(const QVariant & value);
    QWidget * editableWidget(const QVariant & value, const QHash<QString, QString> &options);

    QHash<QString, QHash<QString, QString> > getPropertyInfos(const QMetaObject *metaObject);

    QStringList _filter;

protected slots:
    void onStringPropertyChanged();
    void onBoolPropertyChanged();
    void onIntPropertyChanged();
    void onDoublePropertyChanged();
    void onColorPropertyChanged();
    void onPenPropertyChanged();
    void onBrushPropertyChanged();

private:

    QObject * _object;
    QHash<QWidget*, int> _widgetPropertyMap;

//    QSignalMapper _mapper;

//    QWidget * _frame;
    QScrollArea * _scrollArea;

};

//******************************************************************************

class ColorEditor : public QFrame
{
    Q_OBJECT
    Q_PROPERTY_WITH_ACCESSORS(int, colorIconSize, getColorIconSize, setColorIconSize)
public:
    ColorEditor(const QColor & color, QWidget * parent = 0);
    QColor getColor() const
    { return _colorValue; }

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
