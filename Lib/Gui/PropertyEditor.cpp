
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
        layout->addRow(new QLabel(QString("y=%1, x=%2").arg(pt.y()).arg(pt.x())));
    }
    return w;
}

QWidget * createVectorDWidget(const QVector<double> & v)
{
    QString vStr;
    if (!v.isEmpty())
    {
        vStr = "( ";
        foreach (double el, v)
        {
            vStr.append(QString("%1, ").arg(el));
        }
        vStr.remove(vStr.size()-2,2);
        vStr.append(" )");
    }
    else
    {
        vStr = "Unknown";
    }
    QLabel * l = new QLabel(vStr);
    l->setWordWrap(true);
    return l;
}


//******************************************************************************

/*!
  \class PropertyEditor
  \brief Widget that allows to display/edit object properties. Call setup() method on
    the object and Ui is automatically updated. To filter names use the methods
    setPropertyFilter() and setPropertyUnfilter(). The method setPropertyFilter() specifies
    list of properties desired to be shown. The method setPropertyUnfilter() specifies
    list of properties non-desired to be shown. Two filters can not work together.
    Once one is specified, second is cleared

    Annotations as Q_CLASSINFO("size","label:Size;minValue:1;maxValue:500")
    or Q_CLASSINFO("size","label:Size;minValue:1;maxValue:500;step:2")
    allows to replace and configure property editor widget. In the example property with the name
    'size' will be labelled as 'Size' and minValue/maxValue properties of the editor spinbox will
    be set to 1 and 500 with step = 2 (or 1 by default)
    Other examples:
    Q_CLASSINFO("type","label:Type of filtering;possibleValues:Small,Normal,Large")

    Availables options :
    * label:<a label name> = property label name shown by PropertyEditor
    * minValue:1,
    * maxValue:20,
    * step:2 = min/max values and step for spinboxes for int/float/double type properties
    * possibleValues:a,b,c = enum values for string properties
    //* optional:<any char> = creates a checkbox before the property widget representation

*/

//******************************************************************************

PropertyEditor::PropertyEditor(QWidget *parent) :
    QWidget(parent),
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
    QWidget * frame = _scrollArea->takeWidget();
    if (frame)
    {
        delete frame;
    }

    if (!object)
    {
        _scrollArea->hide();
        return;
    }

    frame = new QWidget();
    QFormLayout * layout = new QFormLayout(frame);
    layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);


    // disconnect previous connections
//    if (_object)
//    {
//        disconnect(_object, 0, this, 0);
//    }

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

        if ( (!_filterPos.isEmpty() && _filterPos.contains(name)) ||
             (!_filterNeg.isEmpty() && !_filterNeg.contains(name)))
        {

            QHash<QString, QString> options = propertyOptionsMap.value(name);
            QString label = name;
            if (options.contains("label"))
            {
                label = options["label"];
            }

            label[0] = label[0].toUpper();
            label = "<b>"+label;
            label += " :</b>";

//            QMetaMethod notifySignal = property.notifySignal();
//            if (notifySignal.isValid())
//            { // connect signal with 'this'
//                QString signalSignature = QString("%1").arg(notifySignal.methodSignature().data());
//                connect(_object, signalSignature.toLatin1().constData(),
//                        _mapper, SLOT(onPropertyChanged(const QVariant &)));
//            }

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

    if (!_scrollArea->isVisible())
        _scrollArea->show();

    _scrollArea->setWidget(frame);

    if (!frame->isVisible())
        frame->show();

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

    if (value.type() == QVariant::Color)
    {
        ColorEditor * editor = new ColorEditor(value.value<QColor>());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onColorPropertyChanged()));
        out = editor;
    }
    else if (value.type() == QVariant::Pen)
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
            foreach (QString item, options["possibleValues"].split(","))
            {
                editor->addItem(item);
            }
            editor->setCurrentText(value.toString());
            connect(editor, SIGNAL(currentIndexChanged(QString)), this, SLOT(onStringPropertyChanged()));
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
    else if (value.type() == QVariant::Int)
    {
        QSpinBox * editor = new QSpinBox();
        if (options.contains("minValue") && options.contains("maxValue"))
        {
            int minValue = options["minValue"].toInt();
            int maxValue = options["maxValue"].toInt();
            editor->setMinimum(minValue);
            editor->setMaximum(maxValue);
            if (options.contains("step"))
            {
                int step = options["step"].toInt();
                editor->setSingleStep(step);
            }
            else
            {
                editor->setSingleStep(1);
            }
        }
        editor->setValue(value.toInt());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onIntPropertyChanged()));
        out = editor;
    }
    else if (value.type() == QVariant::Double || value.type() == QMetaType::Float)
    {
        QDoubleSpinBox * editor = new QDoubleSpinBox();
        if (options.contains("minValue") && options.contains("maxValue"))
        {
            double minValue = options["minValue"].toDouble();
            double maxValue = options["maxValue"].toDouble();
            editor->setMinimum(minValue);
            editor->setMaximum(maxValue);
            if (options.contains("step"))
            {
                double step = options["step"].toDouble();
                editor->setSingleStep(step);
            }
            else
            {
                editor->setSingleStep(qAbs(maxValue-minValue)*0.01);
            }
        }
        editor->setValue(value.toDouble());
        connect(editor, SIGNAL(editingFinished()), this, SLOT(onDoublePropertyChanged()));
        out = editor;
    }

//    if (options.contains("optional"))
//    {
////        QWidget * box =
//    }

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
    else if (value.canConvert< QVector<double> >())
    {
        return createVectorDWidget(value.value< QVector<double> >());
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

#define OnPropertyChanged(Type, newValue) \
    Type * edit = qobject_cast<Type *>(sender()); \
    if (!edit) return; \
    if (!_widgetPropertyMap.contains(edit)) return; \
    int propertyIndex = _widgetPropertyMap.value(edit, -1); \
    const QMetaObject * metaObject = _object->metaObject(); \
    QMetaProperty property = metaObject->property(propertyIndex); \
    if (!property.write(_object, newValue)) \
    { \
        SD_TRACE(QString("onPropertyChanged : failed to write new property value !")); \
    }

//******************************************************************************

void PropertyEditor::onStringPropertyChanged()
{
    QObject * s = sender();
    if (qobject_cast<QLineEdit*>(s))
    {
        OnPropertyChanged(QLineEdit, edit->text());
    }
    else if (qobject_cast<QComboBox*>(s))
    {
        OnPropertyChanged(QComboBox, edit->currentText());
    }
}

//******************************************************************************

void PropertyEditor::onBoolPropertyChanged()
{
    OnPropertyChanged(QCheckBox, edit->checkState() == Qt::Checked);
}

//******************************************************************************

void PropertyEditor::onIntPropertyChanged()
{
    OnPropertyChanged(QSpinBox, edit->value())
}

//******************************************************************************

void PropertyEditor::onDoublePropertyChanged()
{
    OnPropertyChanged(QDoubleSpinBox, edit->value());
}

//******************************************************************************

void PropertyEditor::onColorPropertyChanged()
{
    OnPropertyChanged(ColorEditor, edit->getColor());
}

//******************************************************************************

void PropertyEditor::onPenPropertyChanged()
{
    OnPropertyChanged(PenEditor, edit->getPen());
}

//******************************************************************************

void PropertyEditor::onBrushPropertyChanged()
{
    OnPropertyChanged(BrushEditor, edit->getBrush());
}

//******************************************************************************

//void PropertyEditor::onPropertyChanged(const QVariant & newValue)
//{
//    int a = 10;
//    a++;
//    int b = a * 10;
//}

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


