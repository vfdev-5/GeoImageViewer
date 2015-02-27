#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt
#include <QMainWindow>

// Project
#include "Gui/ShapeViewer.h"
#include "Gui/ToolsView.h"
#include "Gui/LayersView.h"

namespace Ui {
class MainWindow;
}

class QDockWidget;

//******************************************************************************

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private:

    QDockWidget * createDockWidget(QWidget * w, Qt::DockWidgetArea where);
    Gui::ShapeViewer _viewer;
    Gui::ToolsView _tools;
    Gui::LayersView _layers;

};

//******************************************************************************

#endif // MAINWINDOW_H
