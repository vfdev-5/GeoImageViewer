#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt
#include <QMainWindow>

// Project
#include "Gui/GIViewer.h"
#include "Gui/HistogramRendererView.h"
#include "Gui/DefaultRendererView.h"
#include "Gui/ToolsView.h"

#define USE_HISTOGRAM

namespace Ui {
class MainWindow;
}

//******************************************************************************

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    void createDockWidget(QWidget * w, Qt::DockWidgetArea where);

    Ui::MainWindow *ui;
    Gui::GIViewer _viewer;
#ifdef USE_HISTOGRAM
    Gui::HistogramRendererView _histogram;
#else
    Gui::DefaultRendererView _contrast;
#endif
    Gui::ToolsView _tools;




};

//******************************************************************************

#endif // MAINWINDOW_H
