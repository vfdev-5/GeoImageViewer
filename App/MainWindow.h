#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt
#include <QMainWindow>
#include <QShowEvent>

// Project
#include "Gui/GeoImageViewer.h"
#include "Gui/HistogramRendererView.h"
#include "Gui/DefaultRendererView.h"
#include "Gui/ToolsView.h"
#include "Gui/LayersView.h"

#define USE_HISTOGRAM

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
    ~MainWindow();

protected slots:
    void onOpenImageActionTriggered();
    void onExitActionTriggered();

protected:
    void closeEvent(QCloseEvent * event);

private:

    QDockWidget * createDockWidget(QWidget * w, Qt::DockWidgetArea where);

    Ui::MainWindow *ui;
    Gui::GeoImageViewer _viewer;
#ifdef USE_HISTOGRAM
    Gui::HistogramRendererView _histogram;
#else
    Gui::DefaultRendererView _contrast;
#endif
    Gui::ToolsView _tools;
    Gui::LayersView _layers;



};

//******************************************************************************

#endif // MAINWINDOW_H
