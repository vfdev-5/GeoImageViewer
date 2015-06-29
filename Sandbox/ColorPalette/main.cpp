
// Qt
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QPushButton>

// Project
#include "Core/Global.h"
#include "Gui/ColorPalette.h"
#include "Form.h"

//#define EXAMPLE_1
#define EXAMPLE_2

#ifdef EXAMPLE_1
Gui::ColorPalette * PALETTE = 0;

void onButtonClicked()
{
    if (PALETTE)
    {
        QGradientStops stops = QGradientStops()
                << QGradientStop(0.1, QColor(Qt::black))
                << QGradientStop(0.5, QColor(Qt::blue))
                << QGradientStop(0.7, QColor(Qt::red))
                << QGradientStop(0.9, QColor(Qt::green));

        PALETTE->setupPalette(stops, 20, 350, false);
    }
}
#endif


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);


#ifdef EXAMPLE_1
    QGraphicsScene scene;
    scene.setSceneRect(-0.15, -0.15, 1.30, 1.30);

    QGraphicsView view;
    view.setScene(&scene);


    QPushButton btn("Setup palette");
    btn.show();
    QObject::connect(&btn, &QPushButton::clicked, onButtonClicked);


    Gui::ColorPalette * palette = new Gui::ColorPalette();
    PALETTE = palette;
    scene.addItem(palette);

    view.show();
    view.fitInView(scene.sceneRect());
#endif


#ifdef EXAMPLE_2
    Form form;
    form.show();
#endif

    return a.exec();

}
