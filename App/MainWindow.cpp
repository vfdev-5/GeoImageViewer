
// Qt
#include <QDockWidget>

// Project
#include "MainWindow.h"
#include "ui_MainWindow.h"

//******************************************************************************

/*!
  \class MainWindow
  \brief This is an example class of usage of GIViewer and other views and tools.
 */

//******************************************************************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    // Setup UI :
    ui->setupUi(this);

    setCentralWidget(&_viewer);

    createDockWidget(&_tools, Qt::LeftDockWidgetArea)->setFloating(true);
    _viewer.setToolsView(&_tools);

    createDockWidget(&_layers, Qt::LeftDockWidgetArea);
    _viewer.setLayersView(&_layers);


#ifdef USE_HISTOGRAM
    createDockWidget(&_histogram, Qt::RightDockWidgetArea);
    _viewer.setRendererView(&_histogram);
#else
    createDockWidget(&_contrast, Qt::RightDockWidgetArea);
    _viewer.setRendererView(&_contrast);
#endif


}

//******************************************************************************

MainWindow::~MainWindow()
{
    delete ui;
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
