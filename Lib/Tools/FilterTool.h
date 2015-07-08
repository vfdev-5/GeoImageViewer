#ifndef FILTERTOOL_H
#define FILTERTOOL_H

// Qt
#include <QObject>

// Project
#include "Core/LibExport.h"
#include "Core/Global.h"
#include "AbstractTool.h"


class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsView;

namespace Tools
{

//******************************************************************************

class GIV_DLL_EXPORT FilterTool : public ImageCreationTool
{
    Q_OBJECT
    PTR_PROPERTY_ACCESSORS(const Core::ImageDataProvider, dataProvider, getDataProvider, setDataProvider)

    PROPERTY_ACCESSORS(double, size, getSize, setSize)

    Q_PROPERTY(double opacity READ getOpacity WRITE setOpacity)
    PROPERTY_ACCESSORS(double, opacity, getOpacity, setOpacity)
    Q_CLASSINFO("opacity","label:Transparency;minValue:0.0;maxValue:1.0")

public:

    FilterTool(QGraphicsScene* scene, QGraphicsView * view, QObject * parent = 0);

    void setGraphicsSceneAndView(QGraphicsScene* scene, QGraphicsView * view)
    {
        _scene = scene;
        _view = view;
        _isValid = _scene && _view;
    }

    enum {
        Type = 3,
    };

    virtual bool dispatch(QEvent * e, QGraphicsScene * scene);
    virtual bool dispatch(QEvent * e, QWidget * viewport);

    virtual void setErase(bool erase);

signals:
    void drawingsFinalized(const QString&, Core::DrawingsItem * item);
    void itemCreated(QGraphicsItem * item);

protected slots:
    virtual void onFinalize();
    virtual void clear();
    void onHideShowResult();

protected:
    virtual void createCursor();
    virtual void destroyCursor();

    void drawAtPoint(const QPointF & pt);
    virtual cv::Mat processData(const cv::Mat & data) = 0; //!< Abstract method to process data. Returns RGBA (4-channels, 8U) matrix


    QGraphicsPixmapItem * _cursorShape;
    double _cursorShapeScale;
    const int _cursorShapeZValue;

    QGraphicsScene * _scene;
    QGraphicsView * _view;
    QAction * _finalize;
    QAction * _clear;
    QAction * _hideShowResult;

    bool _isValid;


};

//******************************************************************************

}

#endif // FILTERTOOL_H
