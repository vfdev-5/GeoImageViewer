
// STD
#include <iostream>
#include <limits>

// Qt
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QString>

// Project
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Gui/HistogramView.h"

//******************************************************************************

#define SD_TRACE(msg) std::cout << QString(msg).toStdString() << std::endl;

//******************************************************************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

//******************************************************************************

MainWindow::~MainWindow()
{
    delete ui;
}

//******************************************************************************
