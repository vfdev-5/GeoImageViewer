

// Project
#include "Core/ObjectTreeModel.h"
#include "Core/BaseLayer.h"
#include "Core/GeoImageLayer.h"
#include "Core/GeoShapeLayer.h"

// Sandbox
#include "Form.h"


Form::Form(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui_Form)
{
    _ui->setupUi(this);

    // setup Objects
    QObject * root = new Core::BaseLayer(0, this);
    QObject * o1 = new Core::GeoImageLayer(0, root);
    QObject * o2 = new Core::GeoShapeLayer(0, root);
    QObject * o12 = new Core::GeoShapeLayer(0, o1);

    // setup model & view
    Core::ObjectTreeModel * model = new Core::ObjectTreeModel(root, this);
    _ui->_view->setModel(model);

}

Form::~Form()
{
    delete _ui;
}
