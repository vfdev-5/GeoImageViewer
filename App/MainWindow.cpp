
// Qt
#include <QApplication>
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
    // Setup plugins
    _viewer.setupPlugins(qApp->applicationDirPath() + "/Plugins");



    // Setup UI :
    ui->setupUi(this);
    setCentralWidget(&_viewer);
#ifdef USE_HISTOGRAM
    createDockWidget(&_histogram, Qt::RightDockWidgetArea);
    _viewer.setRendererView(&_histogram);
#else
    createDockWidget(&_contrast, Qt::RightDockWidgetArea);
    _viewer.setRendererView(&_contrast);
#endif

    createDockWidget(&_tools, Qt::LeftDockWidgetArea);
    _viewer.setToolsView(&_tools);

}

//******************************************************************************

MainWindow::~MainWindow()
{
    delete ui;
}

//******************************************************************************

void MainWindow::createDockWidget(QWidget *w, Qt::DockWidgetArea where)
{
    QDockWidget * dock = new QDockWidget(w->windowTitle(), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(w);
    addDockWidget(where, dock);
}

//******************************************************************************
