
// Qt
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>

// Project
#include "Core/Global.h"
#include "Gui/ColorPalette.h"


//QGraphicsView * VIEW = 0;

//void onContextMenuRequested(const QPoint & point)
//{
//    QMenu menu;
//    QAction * zoomIn = menu.addAction("Zoom In");
//    QAction * zoomOut = menu.addAction("Zoom Out");
//    if (VIEW)
//    {
//        QAction * selectedAction  = menu.exec(VIEW->mapToGlobal(point));
//        if (selectedAction)
//        {
//            SD_TRACE(selectedAction->text());
//        }
//    }
//    if (VIEW)
//    {
//    }
//}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QGraphicsScene scene;
    scene.setSceneRect(-0.15, -0.15, 1.30, 1.30);


    Gui::ColorPalette * palette = new Gui::ColorPalette();
    scene.addItem(palette);

    QGradientStops stops = QGradientStops()
            << QGradientStop(0.1, QColor(Qt::black))
            << QGradientStop(0.5, QColor(Qt::blue))
            << QGradientStop(0.7, QColor(Qt::red))
            << QGradientStop(0.9, QColor(Qt::green));

    palette->setupPalette(stops, 20, 350, false);


    QGraphicsView view;
    view.setScene(&scene);
//    view.setContextMenuPolicy(Qt::CustomContextMenu);
//    VIEW = &view;
//    QObject::connect(&view, &QGraphicsView::customContextMenuRequested, onContextMenuRequested);

    view.show();
    view.fitInView(scene.sceneRect());

    return a.exec();

}
