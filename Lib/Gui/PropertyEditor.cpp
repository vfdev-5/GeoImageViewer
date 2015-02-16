
// Qt
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QColorDialog>

// Project
#include "PropertyEditor.h"

namespace Gui
{

//******************************************************************************

/*!
  \class PropertyEditor
  \brief Widget that allows to display/edit object properties. Call setup() method on
    the object and Ui is automatically updated.

*/

//******************************************************************************

PropertyEditor::PropertyEditor(QWidget *parent) :
    QWidget(parent),
    _frame(0),
    _object(0)
{
}

//******************************************************************************

void PropertyEditor::setup(QObject * object)
{
    // remove the layout and construct new one:
    if (_frame)
    {
        delete _frame;
    }
    _frame = new QWidget(this);
    QFormLayout * layout = new QFormLayout(_frame);


    _object = object;
    _widgetPropertyMap.clear();

    // setup layout:
    const QMetaObject * metaObject = object->metaObject();
    int count = metaObject->propertyCount();
    for (int i=0; i<count; i++)
    {
        QMetaProperty property = metaObject->property(i);
        QString name = property.name();
        if (!_filter.contains(name))
        {
            name[0] = name[0].toUpper();
            name += " : ";
            if (property.isWritable())
            {
                QWidget * w = editableWidget(property.read(object));
                _widgetPropertyMap.insert(w, i);
                if (w)
                {
                    layout->addRow(name, w);
                }

            }
            else
            {

                layout->addRow(name, readableWidget(property.read(object)));
            }
        }
    }

    if (!_frame->isVisible())
        _frame->show();

}

//******************************************************************************

QWidget * PropertyEditor::editableWidget(const QVariant &value)
{
    QWidget * out = 0;
    if (value.type() == QVariant::Pen)
    {
        PenEditor * editor = new PenEditor(value.value<QPen>());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onPenPropertyChanged()));
        out = editor;
    }
    else if (value.type() == QVariant::Brush)
    {
        BrushEditor * editor = new BrushEditor(value.value<QBrush>());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onBrushPropertyChanged()));
        out = editor;
    }
    else if (value.type() == QVariant::String)
    {
        QLineEdit * editor = new QLineEdit(value.toString());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onStringPropertyChanged()));
        out = editor;
    }
    return out;
}

//******************************************************************************

QWidget * PropertyEditor::readableWidget(const QVariant &value)
{
    if (value.canConvert<QString>())
    {
        return new QLabel(value.toString());
    }
    return 0;
}

//******************************************************************************

void PropertyEditor::onStringPropertyChanged()
{
    QLineEdit * edit = qobject_cast<QLineEdit*>(sender());
    if (!edit) return;

    if (!_widgetPropertyMap.contains(edit)) return;

    int propertyIndex = _widgetPropertyMap.value(edit, -1);

    const QMetaObject * metaObject = _object->metaObject();
    QMetaProperty property = metaObject->property(propertyIndex);

    if (!property.write(_object, edit->text()))
    {
        SD_TRACE(QString("onStringPropertyChanged : failed to write new property value !"));
    }
}

//******************************************************************************

void PropertyEditor::onPenPropertyChanged()
{
    PenEditor * edit = qobject_cast<PenEditor*>(sender());
    if (!edit) return;

    if (!_widgetPropertyMap.contains(edit)) return;

    int propertyIndex = _widgetPropertyMap.value(edit, -1);

    const QMetaObject * metaObject = _object->metaObject();
    QMetaProperty property = metaObject->property(propertyIndex);

    if (!property.write(_object, edit->getPen()))
    {
        SD_TRACE(QString("onPenPropertyChanged : failed to write new property value !"));
    }
}

//******************************************************************************

void PropertyEditor::onBrushPropertyChanged()
{
    BrushEditor * edit = qobject_cast<BrushEditor*>(sender());
    if (!edit) return;

    if (!_widgetPropertyMap.contains(edit)) return;

    int propertyIndex = _widgetPropertyMap.value(edit, -1);

    const QMetaObject * metaObject = _object->metaObject();
    QMetaProperty property = metaObject->property(propertyIndex);

    if (!property.write(_object, edit->getBrush()))
    {
        SD_TRACE(QString("onBrushPropertyChanged : failed to write new property value !"));
    }
}

//******************************************************************************
/*!
    \class ColorEditor
    \brief Widget to display/edit QColor property of objects
  */
//******************************************************************************

ColorEditor::ColorEditor(const QColor &color, QWidget *parent) :
    QFrame(parent),
    _colorIconSize(32)
{
    //    setFrameStyle(QFrame::Sunken);
    //    setFrameShape(QFrame::Panel);

        QFormLayout * layout = new QFormLayout(this);

        _color = new QToolButton();
//        _color->setAutoRaise(true);
        _color->setIcon(createIconFromColor(color, _colorIconSize));
        _color->setIconSize(QSize(_colorIconSize, _colorIconSize));
        _colorValue = color;
        connect(_color, SIGNAL(clicked()), this, SLOT(onColorIconClicked()));
        layout->addRow(tr("color : "), _color);
}

//******************************************************************************

void ColorEditor::onColorIconClicked()
{
    const QColorDialog::ColorDialogOptions options = QColorDialog::ShowAlphaChannel;
    const QColor color = QColorDialog::getColor(_colorValue, this, "Select Color", options);
    if (color.isValid()) {
        _colorValue = color;
        _color->setIcon(createIconFromColor(_colorValue, _colorIconSize));
        emit editingFinished();
    }
}

//******************************************************************************
/*!
    \class PenEditor
    \brief Widget to display/edit QPen property of objects
  */
//******************************************************************************

PenEditor::PenEditor(const QPen & pen, QWidget *parent) :
    ColorEditor(pen.color(), parent)
{
//    setFrameStyle(QFrame::Sunken);
//    setFrameShape(QFrame::Panel);
    QFormLayout * layout = qobject_cast<QFormLayout*>(this->layout());
    if (!layout)
    {
        SD_TRACE("PenEditor : layout is null");
        return;
    }

    _style = new QComboBox();
    _style->addItem(tr("Solid"), (int) Qt::SolidLine);
    _style->addItem(tr("Dash"), (int) Qt::DashLine);
    _style->addItem(tr("Dash Dot"), (int) Qt::DashDotLine);
    int index = _style->findData((int) pen.style());
    _style->setCurrentIndex(index);
    connect(_style, SIGNAL(currentIndexChanged(int)), this, SIGNAL(editingFinished()));
    layout->addRow(tr("style :"), _style);

    _width = new QSpinBox();
    _width->setMinimum(0);
    _width->setMaximum(100);
    _width->setValue(pen.width());
    connect(_width, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    layout->addRow(tr("width : "), _width);

}

//******************************************************************************

QPen PenEditor::getPen() const
{
    QPen out(QBrush(_colorValue), _width->value());
    out.setStyle((Qt::PenStyle) _style->currentData().toInt());
    return out;
}

//******************************************************************************
/*!
    \class BrushEditor
    \brief Widget to display/edit QBrush property of objects
  */
//******************************************************************************

BrushEditor::BrushEditor(const QBrush &brush, QWidget *parent) :
    ColorEditor(brush.color(), parent)
{

}

//******************************************************************************

QBrush BrushEditor::getBrush() const
{
    QBrush brush(_colorValue);
    return brush;
}

//******************************************************************************

QIcon createIconFromColor(const QColor &c, int size)
{
    QPixmap p(size, size);
    p.fill(c);
    return QIcon(p);
}

//******************************************************************************

}


