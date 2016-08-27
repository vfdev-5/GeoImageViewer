// Qt
#include <qmath.h>

// Project
#include "Core/ObjectTreeModel.h"
#include "Core/BaseLayer.h"
#include "Core/GeoImageLayer.h"
#include "Core/GeoShapeLayer.h"

// Sandbox
#include "Form.h"


Form::Form(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui_Form),
    _removeLayer("Remove", this)
{
    _ui->setupUi(this);

    // setup Objects
    double z = 0.0;
    Core::BaseLayer * invisibleRoot = new Core::BaseLayer(0, this);
    _mainImage = new Core::GeoImageLayer(0, invisibleRoot);
    _mainImage->setObjectName("Main image");
    _mainImage->setProperty("zValue", z);
    SD_TRACE_PTR("_mainImage", _mainImage);

    QObject * o1 = new Core::GeoImageLayer(0, _mainImage);
    o1->setObjectName("o1");
    z = Core::computeZValue(z, 0, 1);
    o1->setProperty("zValue", z);
    SD_TRACE_PTR("o1", o1);

    QObject * o12 = new Core::GeoShapeLayer(0, o1);
    o12->setObjectName("o12");
    z = Core::computeZValue(z, 0, 2);
    o12->setProperty("zValue", z);
    SD_TRACE_PTR("o12", o12);

    QObject * o2 = new Core::GeoShapeLayer(0, _mainImage);
    o2->setObjectName("o2");
    z = Core::computeZValue(_mainImage->property("zValue").toDouble(), 1, 1);
    o2->setProperty("zValue", z);
    SD_TRACE_PTR("o2", o2);

    QObject * o3 = new Core::BaseLayer(0, _mainImage);
    o3->setObjectName("o3");
    z = Core::computeZValue(_mainImage->property("zValue").toDouble(), 2, 1);
    o3->setProperty("zValue", z);
    SD_TRACE_PTR("o3", o3);

    // Display children ordering :
//    _mainImage->dumpObjectTree();

    // Reoder
//    SD_TRACE(">>> Reorder");
//    o2->setParent(0);
//    o2->setParent(_mainImage);

    // Display children ordering :
//    _mainImage->dumpObjectTree();

    // setup model & view
    _model = new Core::ObjectTreeModel(invisibleRoot, this);
    _model->setRole(Qt::DisplayRole, "type");
    _model->setRole(Qt::EditRole, "type");
    _model->setRole(Qt::CheckStateRole, "isVisible");
    _model->setOrderingRole("zValue");

    _ui->_view->setModel(_model);
    _ui->_view->addAction(&_removeLayer);
    connect(&_removeLayer, SIGNAL(triggered()), this, SLOT(onRemoveLayer()));
}

Form::~Form()
{
    delete _ui;
}

void Form::on__add_clicked()
{
    // Random add
//    QObjectList stack = QObjectList();
//    stack << _mainImage;
//    int count = qrand() % _mainImage->children().count();
    QObject * o = 0;
//    while(!stack.isEmpty() && count > 0)
//    {
//        QObject * child = stack.takeLast();
//        o = child;
//        stack << child->children();
//        count--;
//    }

    o = _mainImage;
    while (!o->children().isEmpty())
    {
        o = o->children().last();
    }

    QObject * oo = new Core::GeoShapeLayer(0, o);
//    QObject * oo = new QObject(o);
    oo->setObjectName("new GeoShapeLayer");

    double z = qrand() % 1000 + 2.5;
    oo->setProperty("zValue", z);

}

void Form::on__remove_clicked()
{
    // Random remove
//    QObjectList stack = QObjectList();
//    stack << _mainImage->children();
//    int count = qrand() % _mainImage->children().count();
    QObject * o = 0;
//    while(!stack.isEmpty() && count > 0)
//    {
//        QObject * child = stack.takeLast();
//        o = child;
//        stack << child->children();
//        count--;
//    }

    o = _mainImage;
    while (!o->children().isEmpty())
    {
        o = o->children().last();
    }
    if (o != _mainImage)
    {
        o->setParent(0);
        delete o;
    }
}

void Form::onRemoveLayer()
{



}

