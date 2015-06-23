#ifndef GEOIMAGEITEM_H
#define GEOIMAGEITEM_H


// Qt
#include <QObject>
#include <QGraphicsItem>
#include <QRunnable>
#include <QMutex>

// Project
#include "Global.h"
#include "ImageRenderer.h"

namespace Core
{

//class ImageRenderer;
//class ImageRendererConfiguration;
class ImageDataProvider;

//******************************************************************************

class TilesLoadTask;

class GeoImageItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    friend class TilesLoadTask;

public:
    struct Settings
    {
        int TileSize;
        int CacheSize;
        int MaxNbOfThreads;
        Settings() :
            TileSize(512),
            CacheSize(50),
            MaxNbOfThreads(3)
        {}
    };

    explicit GeoImageItem(ImageDataProvider * provider, ImageRenderer * renderer, ImageRendererConfiguration *conf, QGraphicsItem * parent = 0);
    virtual ~GeoImageItem();

    enum { Type = UserType + 2 };
    int type() const { return Type; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w);

    void clearCache();
    int getZoomMinLevel()
    { return _zoomMinLevel; }

    const ImageDataProvider * getConstDataProvider() const
    { return _dataProvider; }
    const ImageRendererConfiguration * getRendererConfiguration() const;

public slots:
    void updateItem(int zoomLevel, const QRectF & visiblePixelExtent);
    void onRendererConfigurationChanged(Core::ImageRendererConfiguration *conf);

protected slots:
    void onTileLoaded(QGraphicsPixmapItem*tile, QGraphicsItemGroup * tileGroup, const QString &key);

protected:

    void maintainCache(const QList<QString> & visibleTilesInCache, int nbOfTilesToAdd);
    void showCacheInfo();
    void computeZoomMinLevel();

    void setRenderer(ImageRenderer * renderer);
    void setDataProvider(ImageDataProvider * provider);

    ImageRenderer * _renderer;
    ImageRendererConfiguration * _rconf;
    ImageDataProvider * _dataProvider;

    int _nbXTiles;
    int _nbYTiles;
    int _zoomMinLevel;

    //
    int _currentZoomLevel;
    QRectF _currentVisiblePixelExtent;

    Settings _settings;

    QGraphicsItemGroup* _root;
    QHash<QString,QGraphicsItemGroup*> _zoomTileGroups;
    QHash<QString,QGraphicsPixmapItem*> _tilesCache;
    QList<QString> _tilesCacheHistory;

    TilesLoadTask * _task;

};

//******************************************************************************

class TilesLoadTask : public QObject, public QRunnable
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
            x(_x), //!< Coordinate X in Scene CS
            y(_y), //!< Coordinate Y in Scene CS
            scale(s), //!< Scale of the tile ~ Zoom level
            tileExtent(te), //!< pixel extent of the tile in the Image CS
            tileSize(ts), //!< tile size in pixels
            cacheKey(key), //!< tile key in the cache
            tileGroup(g) //!< tile group
        {}
        int x;
        int y;
        double scale;
        QRect tileExtent;
        QString cacheKey;
        QGraphicsItemGroup * tileGroup;
        int tileSize;
    };

    TilesLoadTask(GeoImageItem * item) :
        QObject(0),
        _item(item),
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
    const GeoImageItem * _item;
    bool _canceled;


};

//******************************************************************************

}

#endif // GEOIMAGEITEM_H
