
// Qt
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

// Project
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Filters/FiltersManager.h"
#include "Filters/AbstractFilter.h"
#include "Gui/PropertyEditor.h"


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
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(onSettingsActionTriggered()));

    //
    QSettings settings("GeoImageViewer_dot_com", "GIV");
    if (settings.contains("MainWindow/geometry"))
    {
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    }
    else
    {
        showMaximized();
    }

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

void MainWindow::onSettingsActionTriggered()
{
    QDialog d;
    d.setLayout(new QVBoxLayout());

    Gui::PropertyEditor editor;
    editor.setPropertyFilter(QStringList () << "backgroundColor");
    editor.setup(&_viewer);

    d.layout()->addWidget(&editor);

    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    d.layout()->addWidget(buttons);


    if (d.exec() == QDialog::Accepted)
    {
//        QSettings settings("GeoImageViewer_dot_com", "GIV");
//        settings.setValue("GeoImageViewer/backgroundColor", saveGeometry());
    }

}

//******************************************************************************

void MainWindow::closeEvent(QCloseEvent * event)
{
    QSettings settings("GeoImageViewer_dot_com", "GIV");
    settings.setValue("MainWindow/geometry", saveGeometry());

    QMainWindow::closeEvent(event);
}

//******************************************************************************

void MainWindow::onOpenImageActionTriggered()
{
    QUrl url = QFileDialog::getOpenFileUrl(this,
                                           tr("Open Image"),
                                           QString(),
                                           tr("Images (*.*)"));
    if (url.isValid())
    {
        _viewer.loadImage(url);
    }
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

//    dock->setMaximumWidth(500);

    return dock;
}

//******************************************************************************
