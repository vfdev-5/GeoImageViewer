
// Qt
#include <QApplication>
#include <QDockWidget>

// Project
#include "MainWindow.h"

//******************************************************************************

/*!
  \class MainWindow
  \brief This is an example class of usage of GIViewer and other views and tools.
 */

//******************************************************************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _viewer("This is a BaseViewer")
{
    // Setup UI :
    setCentralWidget(&_viewer);

    createDockWidget(&_layers, Qt::RightDockWidgetArea);
    _viewer.setLayersView(&_layers);

    createDockWidget(&_tools, Qt::LeftDockWidgetArea)->setFloating(true);
    _viewer.setToolsView(&_tools);


    // resize
    resize(600, 600);

}

//******************************************************************************

QDockWidget * MainWindow::createDockWidget(QWidget *w, Qt::DockWidgetArea where)
{
    QDockWidget * dock = new QDockWidget(w->windowTitle(), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(w);
    addDockWidget(where, dock);
    return dock;
}

//******************************************************************************
