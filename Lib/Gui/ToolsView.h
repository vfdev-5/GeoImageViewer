#ifndef TOOLSVIEW_H
#define TOOLSVIEW_H

// Qt
#include <QWidget>
#include <QHash>

// Project
#include "Core/LibExport.h"
#include "AbstractToolsView.h"

namespace Ui {
class ToolsView;
}

class QToolButton;

namespace Gui
{

//******************************************************************************

class GIV_DLL_EXPORT ToolsView : public AbstractToolsView
{
    Q_OBJECT

public:
    explicit ToolsView(QWidget *parent = 0);
    ~ToolsView();

    virtual void addTool(Tools::AbstractTool *tool);
    virtual void setCurrentTool(const QString & toolName);

protected slots:
    void setCurrentTool(QToolButton *button);
    void onToolButtonClicked();

private:
    Ui::ToolsView *ui;
    QToolButton * _currentTool;
    QHash<QString, QToolButton*> _toolButtonMap;

    int _col, _row;

};

//******************************************************************************

}

#endif // TOOLSVIEW_H
