#ifndef GEOIMAGEITEM_H
#define GEOIMAGEITEM_H


// Qt
#include <QObject>
#include <QGraphicsItem>
#include <QRunnable>
#include <QMutex>

// Project
#include "Global.h"

namespace Core
{

class ImageRenderer;
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
        Settings() :
            TileSize(512),
            CacheSize(50)
        {}
    };



    explicit GeoImageItem(QGraphicsItem * parent = 0);
    virtual ~GeoImageItem();

    enum { Type = UserType + 2 };
    virtual int type() const { return Type; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *o, QWidget * w);

    void clearCache();
    int getZoomMinLevel()
    { return _zoomMinLevel; }

    void setRenderer(ImageRenderer * renderer);
    void setDataProvider(ImageDataProvider * provider);

    const ImageDataProvider * getConstDataProvider() const
    { return _dataProvider; }

    const ImageRenderer * getConstRenderer() const
    { return _renderer; }

    ImageRenderer * getRenderer()
    { return _renderer; }


public slots:
    void updateItem(int zoomLevel, const QRectF & visiblePixelExtent);


protected slots:
    void onTileLoaded(QGraphicsPixmapItem*tile, QGraphicsItemGroup * tileGroup, const QString &key);

protected:

    void maintainCache(const QList<QString> & visibleTilesInCache, int nbOfTilesToAdd);
    void showCacheInfo();
    void computeZoomMinLevel();

    ImageRenderer * _renderer;
    ImageDataProvider * _dataProvider;

    int _nbXTiles;
    int _nbYTiles;
    int _zoomMinLevel;

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
    GeoImageItem * _item;
    bool _canceled;


};

//******************************************************************************

}

#endif // GEOIMAGEITEM_H
