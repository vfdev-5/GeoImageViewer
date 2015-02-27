
// Qt
#include <QDir>
#include <QToolButton>

// Project
#include "Core/Global.h"
#include "ToolsView.h"
#include "PropertyEditor.h"
#include "Tools/AbstractTool.h"
#include "Tools/ToolsManager.h"
#include "ui_ToolsView.h"

namespace Gui
{

#define COLSIZE 4

//******************************************************************************

ToolsView::ToolsView(QWidget *parent) :
    AbstractToolsView(parent),
    ui(new Ui::ToolsView),
    _currentTool(0),
    _col(0), _row(0)
{
    ui->setupUi(this);

    ui->_editor->setNameFilter(QStringList()
                               << "objectName"
                               << "icon"
                               << "cursor"
                );

}

//******************************************************************************

ToolsView::~ToolsView()
{
    delete ui;
}

//******************************************************************************

void advance(int * col, int * row)
{
    (*col)++;
    if (*col >= COLSIZE)
    {
        (*row)++;
        *col = 0;
    }
}

void ToolsView::addTool(Tools::AbstractTool *tool)
{

    // Create tool button :
    QToolButton * button = new QToolButton(ui->_toolsFrame);
    button->setAutoRaise(true);
    button->setIconSize(QSize(20, 20));
    button->setIcon(tool->getIcon());
    button->setObjectName(tool->objectName());
    ui->_toolsFrameLayout->addWidget(button, _row, _col);
    advance(&_col, &_row);

    _toolButtonMap.insert(tool->objectName(), button);

    connect(button, SIGNAL(clicked()), this, SLOT(onToolButtonClicked()));


}

//******************************************************************************

void ToolsView::setCurrentTool(const QString & toolName)
{
    QToolButton * button = _toolButtonMap.value(toolName, 0);
    if (button)
    {
        setCurrentTool(button);
    }
}

//******************************************************************************

void ToolsView::setCurrentTool(QToolButton *button)
{
    if (_currentTool) _currentTool->setAutoRaise(true);
    _currentTool=button;
    _currentTool->setAutoRaise(false);
    QString toolName = button->objectName();
    ui->_editor->setup( Tools::ToolsManager::get()->getTool(toolName) );
    emit toolChanged(toolName);
}

//******************************************************************************

void ToolsView::onToolButtonClicked()
{
    QToolButton * button = qobject_cast<QToolButton*>(sender());
    if (!button || button == _currentTool)
        return;

    setCurrentTool(button);
}

//******************************************************************************

}
