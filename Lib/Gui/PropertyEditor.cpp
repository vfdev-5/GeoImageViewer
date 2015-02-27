
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
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QScrollArea>
#include <QAction>
#include <QPushButton>

// Project
#include "PropertyEditor.h"

namespace Gui
{

#define CreateRectWidget() \
    QWidget * w = new QWidget();    \
    QFormLayout * layout = new QFormLayout(w); \
    layout->addRow(QObject::tr("top-left :"), new QLabel(QString("%1, %2").arg(r.top()).arg(r.left()))); \
    layout->addRow(QObject::tr("bottom-right :"), new QLabel(QString("%1, %2").arg(r.bottom()).arg(r.right()))); \
    layout->addRow(new QLabel(QObject::tr("width=%1, height=%2").arg(r.width()).arg(r.height()))); \
    return w;

QWidget * createRectWidget(const QRectF & r )
{
    CreateRectWidget();
}

QWidget * createRectWidget(const QRect & r )
{
    CreateRectWidget();
}

QWidget * createPolygonWidget(const QPolygonF & p )
{
    QWidget * w = new QWidget();
    QFormLayout * layout = new QFormLayout(w);
    foreach (QPointF pt, p)
    {
        layout->addRow(new QLabel(QString("y=%1, x=%2").arg(pt.x()).arg(pt.y())));
    }
    return w;
}


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
    _scrollArea = new QScrollArea(this);
    setLayout(new QHBoxLayout());
    layout()->addWidget(_scrollArea);

}

//******************************************************************************

void PropertyEditor::setup(QObject * object)
{

    // remove the layout and construct new one:
    if (_frame)
    {
        delete _frame;
    }
    _frame = new QWidget();
    QFormLayout * layout = new QFormLayout(_frame);

    if (!object)
    {
        return;
    }


    _object = object;
    _widgetPropertyMap.clear();

    // setup layout:
    const QMetaObject * metaObject = object->metaObject();
    int count = metaObject->propertyCount();

    QHash<QString, QHash<QString, QString> > propertyOptionsMap =
            getPropertyInfos(metaObject);


    for (int i=0; i<count; i++)
    {
        QMetaProperty property = metaObject->property(i);
        QString name = property.name();
        if (!_filter.contains(name))
        {

            QHash<QString, QString> options = propertyOptionsMap.value(name);
            QString label = name;
            if (options.contains("label"))
            {
                label = options["label"];
            }

            label[0] = label[0].toUpper();
            label += " : ";
            if (property.isWritable())
            {
                QWidget * w = editableWidget(property.read(object), options);
                _widgetPropertyMap.insert(w, i);
                if (w)
                {
                    layout->addRow(label, w);
                }
            }
            else
            {
                layout->addRow(label, readableWidget(property.read(object)));
            }
        }
    }

    _scrollArea->setWidget(_frame);

    if (!_frame->isVisible())
        _frame->show();

}

//******************************************************************************

QHash<QString, QHash<QString, QString> > PropertyEditor::getPropertyInfos(const QMetaObject *metaObject)
{
    QHash<QString, QHash<QString, QString> > propertyOptionsMap;
    int classInfoCount = metaObject->classInfoCount();
    for (int i=0;i<classInfoCount;i++)
    {
        QMetaClassInfo info = metaObject->classInfo(i);
        QString optionString = info.value();
        QHash<QString, QString> options;
        QStringList list = optionString.split(";");

        foreach (QString line, list)
        {
            QStringList pList = line.split(":");
            options.insert(pList.first(), pList.last());
        }
        propertyOptionsMap.insert(info.name(), options);
    }
    return propertyOptionsMap;
}


//******************************************************************************

QWidget * PropertyEditor::editableWidget(const QVariant &value, const QHash<QString, QString> & options)
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
        if (options.contains("possibleValues"))
        {
            QComboBox * editor = new QComboBox();
            // ....
            out = editor;
        }
        else
        {
            QLineEdit * editor = new QLineEdit(value.toString());
            connect(editor, SIGNAL(editingFinished()), this, SLOT(onStringPropertyChanged()));
            out = editor;
        }
    }
    else if (value.type() == QVariant::Bool)
    {
        QCheckBox * editor = new QCheckBox();
        editor->setCheckState(value.toBool() ? Qt::Checked : Qt::Unchecked);
        connect(editor, SIGNAL(stateChanged(int)), this, SLOT(onBoolPropertyChanged()));
        out = editor;
    }
    else if (value.type() == QVariant::Double)
    {
        QDoubleSpinBox * editor = new QDoubleSpinBox();
        editor->setValue(value.toDouble());
        if (options.contains("minValue") && options.contains("maxValue"))
        {
            double minValue = options["minValue"].toDouble();
            double maxValue = options["maxValue"].toDouble();
            editor->setMinimum(minValue);
            editor->setMaximum(maxValue);
            editor->setSingleStep(qAbs(maxValue-minValue)*0.01);
        }
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onDoublePropertyChanged()));
        out = editor;
    }


    return out;
}

//******************************************************************************

QWidget * PropertyEditor::readableWidget(const QVariant &value)
{
    if (value.canConvert<QString>())
    {
        QLabel * l = new QLabel(value.toString());
        l->setWordWrap(true);
        return l;
    }
    else if (value.type() == QVariant::Rect)
    {
        return createRectWidget(value.toRect());
    }
    else if (value.type() == QVariant::RectF)
    {
        return createRectWidget(value.toRectF());
    }
    else if (value.type() == QVariant::PolygonF)
    {
        return createPolygonWidget(value.value<QPolygonF>());
    }
//    else if (!value.value<QAction*>())
//    {
//        QAction * action = value.value<QAction*>();
//        QPushButton * btn = new QPushButton(action->text());
//        connect(btn, SIGNAL(clicked()), action, SIGNAL(triggered()));
//        SD_TRACE("QAction is found");
//        return btn;
//    }
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

void PropertyEditor::onBoolPropertyChanged()
{
    QCheckBox * edit = qobject_cast<QCheckBox*>(sender());
    if (!edit) return;

    if (!_widgetPropertyMap.contains(edit)) return;

    int propertyIndex = _widgetPropertyMap.value(edit, -1);

    const QMetaObject * metaObject = _object->metaObject();
    QMetaProperty property = metaObject->property(propertyIndex);

    if (!property.write(_object, edit->checkState() == Qt::Checked))
    {
        SD_TRACE(QString("onStringPropertyChanged : failed to write new property value !"));
    }
}

//******************************************************************************

void PropertyEditor::onDoublePropertyChanged()
{
    QDoubleSpinBox * edit = qobject_cast<QDoubleSpinBox*>(sender());
    if (!edit) return;

    if (!_widgetPropertyMap.contains(edit)) return;

    int propertyIndex = _widgetPropertyMap.value(edit, -1);

    const QMetaObject * metaObject = _object->metaObject();
    QMetaProperty property = metaObject->property(propertyIndex);

    if (!property.write(_object, edit->value()))
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

    QBrush b = edit->getBrush();
    QColor c = b.color();

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
    // ...
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


