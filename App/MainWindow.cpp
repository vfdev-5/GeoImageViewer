
// Qt
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>

// Project
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Filters/FiltersManager.h"


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

    createDockWidget(&_tools, Qt::LeftDockWidgetArea);
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

    setCentralWidget(&_viewer);

    // insert filter actions :
    foreach (Filters::AbstractFilter * f, Filters::FiltersManager::get()->getFilters())
    {
        QAction * a = ui->menuFilters->addAction(f->getName(), &_viewer, SLOT(onFilterTriggered()));
        a->setData( QVariant::fromValue( static_cast<QObject*>(f) ) );
    }

    // connect actions:
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onExitActionTriggered()));
    connect(ui->actionOpen_Image, SIGNAL(triggered()), this, SLOT(onOpenImageActionTriggered()));


    //
    showMaximized();


}

//******************************************************************************

MainWindow::~MainWindow()
{
    delete ui;
}

//******************************************************************************

void MainWindow::onExitActionTriggered()
{
    close();
}

//******************************************************************************

void MainWindow::onOpenImageActionTriggered()
{
    QUrl url = QFileDialog::getOpenFileUrl(this,
                                           tr("Open Image"),
                                           QString(),
                                           tr("Images (*.*)"));
    _viewer.loadImage(url);
}

//******************************************************************************

QDockWidget * MainWindow::createDockWidget(QWidget *w, Qt::DockWidgetArea where)
{
//    w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QDockWidget * dock = new QDockWidget(w->windowTitle(), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(w);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(where, dock);
    return dock;
}

//******************************************************************************
