#ifndef LAYERLOADER_H
#define LAYERLOADER_H

// Qt
#include <QObject>
#include <QPixmap>
#include <QRectF>
#include <QHash>
#include <QQueue>
#include <QRunnable>
#include <QMutexLocker>


// Project
#include "LibExport.h"

class QGraphicsItemGroup;
class QGraphicsScene;
class QGraphicsPixmapItem;

namespace Core
{

//******************************************************************************

class ImageLayer;
class LayerRenderer;
class LoadTilesTask;



class LayerLoader : public QObject
{

    Q_OBJECT
    friend class LoadTilesTask;

public:

    struct Settings
    {
        int TileSize;
        int CacheSize;
        Settings() :
            TileSize(512),
            CacheSize(50)
        {
        }
    };

    explicit LayerLoader(QObject * parent);

    virtual ~LayerLoader();

    void clear();
    void clearCache();

    bool hasLayer()
    { return _layer != 0; }

    void setLayer(ImageLayer * layer);
    void setRenderer(LayerRenderer * renderer)
    { _renderer = renderer; }

    void setScene(QGraphicsScene * scene)
    { _scene = scene; }

    int computeZoomMinLevel();
    void updateLayer(int zoomLevel, const QRect & visiblePixelExtent);

protected slots:
    void onTileLoaded(QGraphicsPixmapItem*tile, QGraphicsItemGroup * tileGroup, const QString &key);

protected:

    void maintainCache(const QList<QString> & visibleTilesInCache, int nbOfTilesToAdd);
    void showCacheInfo();

    ImageLayer * _layer;
    LayerRenderer * _renderer;
    QGraphicsScene * _scene;

    int _nbXTiles;
    int _nbYTiles;

    Settings _settings;

    QHash<QString,QGraphicsItemGroup*> _zoomTileGroups;
    QHash<QString,QGraphicsPixmapItem*> _tilesCache;
    QList<QString> _tilesCacheHistory;

    LoadTilesTask * _loadTilesTask;

};



class LoadTilesTask : public QObject, public QRunnable
{
    Q_OBJECT

public:

    struct TileToLoad
    {

        TileToLoad(int _x,
                   int _y,
                   double s,
                   const QRect & te,
                   int ts,
                   const QString & key,
                   QGraphicsItemGroup * g) :
            x(_x),
            y(_y),
            scale(s),
            tileExtent(te),
            tileSize(ts),
            cacheKey(key),
            tileGroup(g)
        {}
        int x;
        int y;
        double scale;
        QRect tileExtent;
        QString cacheKey;
        QGraphicsItemGroup * tileGroup;
        int tileSize;
    };

    LoadTilesTask(LayerLoader * loader) :
        QObject(0),
        _loader(loader),
        _canceled(false)
    {
        setAutoDelete(false);
    }

    void setTilesToLoad(const QList<TileToLoad> & tileList);
    void cancel()
    { _canceled = true; }

signals:
    void tileLoaded(QGraphicsPixmapItem* tile, QGraphicsItemGroup * tileGroup, const QString & key);

protected:

    void run();
    QMutex _mutex;
    QList<TileToLoad> _tilesToLoad;
    LayerLoader * _loader;
    bool _canceled;


};

//******************************************************************************

}

#endif // LAYERLOADER_H
