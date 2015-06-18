
// Qt
#include <qmath.h>
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QThreadPool>
#include <QGraphicsScene>

// Project
#include "GeoImageItem.h"
#include "ImageDataProvider.h"

namespace Core
{

#ifdef GEOIMAGEITEM_DISPLAY_VIEWPORT
QGraphicsRectItem * VIEWPORT = 0;
#endif

//*************************************************************************
/*!
    \class GeoImageItem
    \brief inherits from QGraphicsItem and represents the geo image with its tiles of each Z level and the cache.

    Method (slot) that starts data loading : updateItem(int zoomLevel, const QRectF & visiblePixelExtent)

    1) Data loading and rendering
    The class has children instances of ImageDataProvider (as image source) and ImageRenderer (as data renderer to rgba format).
    It also has an instance of a derived QRunnable class (TilesLoadTask) which is used with QThreadPool to load tiles.
    Tiles are represented by QGraphicsPixmapItem and are grouped by Z level in QGraphicsItemGroups.

    2) Z Level groups and Cache
    Tiles cache is implemented using QHash that maps tile name (as key) to its data QGraphicsPixmapItem. A list of tile names (cache keys) stores
    the history of loaded tiles.



  */

//*************************************************************************

GeoImageItem::GeoImageItem(ImageDataProvider * provider, ImageRenderer * renderer, ImageRendererConfiguration * conf, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    QObject(0),
//    _dataProvider(provider),
//    _renderer(renderer),
//    _nbXTiles(0),
//    _nbYTiles(0),
    _root(new QGraphicsItemGroup(this))
{
    _task = new TilesLoadTask(this);
    connect(_task, SIGNAL(tileLoaded(QGraphicsPixmapItem*,QGraphicsItemGroup*,QString)),
            this, SLOT(onTileLoaded(QGraphicsPixmapItem*,QGraphicsItemGroup*,QString)));
    // We do not use Qt::BlockingQueuedConnection because of deadlocks :
    // when calls _task->setTilesToLoad(tiles) in Main Thread and
    // onTileLoaded() in the same thread

    setDataProvider(provider);
    setRenderer(renderer);
    _rconf = conf;
}

//*************************************************************************

GeoImageItem::~GeoImageItem()
{
    // wait until last processed work is done
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    if (pool->waitForDone())
    {
        // scene is already cleared -> tiles and cache also
        delete _task;
    }

    // destroy renderer configuration
    if (_rconf)
        delete _rconf;

}

//*************************************************************************

/*!
    \overload
*/
QRectF GeoImageItem::boundingRect() const
{
    return _root->boundingRect();
}

//*************************************************************************

/*!
    empty method
*/
void GeoImageItem::paint(QPainter * /*p*/, const QStyleOptionGraphicsItem * /*o*/, QWidget * /*w*/)
{
}

//*************************************************************************

const ImageRendererConfiguration *GeoImageItem::getRendererConfiguration() const
{
    return _rconf;
}

//*************************************************************************

void GeoImageItem::onRendererConfigurationChanged(Core::ImageRendererConfiguration * conf)
{
    // Avoid concurrent access :
    clearCache();
    // set conf:
    *_rconf = *conf;
    // reload tiles
    updateItem(_currentZoomLevel, _currentVisiblePixelExtent);
}

//*************************************************************************
/*!
  \brief GeoImageItem::clearCache
  Method to clear all loaded tiles and clean used memory
 */
void GeoImageItem::clearCache()
{
#ifdef GEOIMAGEITEM_CACHE_VERBOSE
    SD_TRACE("---- Clear Cache : ");
#endif

    // wait until last processed work is done
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    _task->cancel();
    if (pool->waitForDone())
    {

#ifdef GEOIMAGEITEM_CACHE_VERBOSE
        SD_TRACE("---- Clear Cache : start clearing");
#endif
        // Clean layer dependant data
        foreach(QGraphicsItemGroup* item, _zoomTileGroups.values())
        {
            scene()->removeItem(item);
            _root->removeFromGroup(item);
            delete item;
        }
        _zoomTileGroups.clear();
        _tilesCache.clear();
        _tilesCacheHistory.clear();
#ifdef GEOIMAGEITEM_CACHE_VERBOSE
        SD_TRACE("---- Clear Cache : end clearing");
#endif

    }
    else
    {
        SD_TRACE("LayerLoader::clearCache : waitForDone returns false");
    }
}

//******************************************************************************
/*!
 * \brief GeoImageItem::computeZoomMinLevel
 * \return Min zoom level. It is a negative value computed using the condition :
 *  Image at min zoom level <= TileSize <-> max(W,H) * 2^(zMin) <= TileSize
 */
void GeoImageItem::computeZoomMinLevel()
{
    int zoomMinLevel= -qCeil(qLn( qMax(_nbXTiles,_nbYTiles) )/qLn(2.0));
    _zoomMinLevel = (zoomMinLevel > 0) ? 0 : zoomMinLevel;
}

//*************************************************************************

void GeoImageItem::setRenderer(ImageRenderer *renderer)
{
    _renderer = renderer;
    if (!_renderer)
        return;
    _renderer->setParent(this);
}

//*************************************************************************

void GeoImageItem::setDataProvider(ImageDataProvider * provider)
{
    _dataProvider = provider;
    if (!_dataProvider)
        return;

    _dataProvider->setParent(this);
    _nbXTiles = qCeil(_dataProvider->getWidth()*1.0/_settings.TileSize);
    _nbYTiles = qCeil(_dataProvider->getHeight()*1.0/_settings.TileSize);
    computeZoomMinLevel();

}

//******************************************************************************

void GeoImageItem::updateItem(int nZoomLevel, const QRectF &nVisiblePixelExtent)
{
    if (!isVisible())
        return;

    if (!_dataProvider || !_renderer || !_rconf)
    {
        SD_TRACE("GeoImageItem::updateItem : data provider and/or renderer are null");
        return;
    }

    int zoomLevel = (nZoomLevel > 0) ? 0 : nZoomLevel;
    zoomLevel = _currentZoomLevel = (zoomLevel < _zoomMinLevel) ? _zoomMinLevel : zoomLevel;
    int nbXTilesAtZ=qCeil(_nbXTiles*qPow(2.0,zoomLevel));
    int nbYTilesAtZ=qCeil(_nbYTiles*qPow(2.0,zoomLevel));
    int tileSize = _settings.TileSize;

    // enlarge nVisibleSceneRect
    QRectF visibleSceneRect = _currentVisiblePixelExtent = nVisiblePixelExtent.adjusted(-10.0,
                                                                                        -10.0,
                                                                                        +10.0,
                                                                                        +10.0);

#ifdef GEOIMAGEITEM_DISPLAY_VIEWPORT
    if (VIEWPORT)
    {
        scene()->removeItem(VIEWPORT);
        delete VIEWPORT;
    }
    VIEWPORT = scene()->addRect(visibleSceneRect);
    VIEWPORT->setZValue(10.0);
    VIEWPORT->setPen(QPen(Qt::black, 0));
    VIEWPORT->setBrush(QColor(10,10,180,22));
#endif


#ifdef GEOIMAGEITEM_SHOW_CACHE_INFO
    showCacheInfo();
#endif


    // set invisible previous tiles :
    foreach (QGraphicsItemGroup * group, _zoomTileGroups)
    {
        group->setVisible(false);
    }

    // prepare current zoom tile group:
    QString key1=QString("TileGroup_%1").arg(zoomLevel);
    QGraphicsItemGroup * tileGroup=0;
    if (_zoomTileGroups.contains(key1))
    {
        tileGroup=_zoomTileGroups[key1];
        tileGroup->setVisible(true);
    }
    else
    {
        tileGroup = new QGraphicsItemGroup();
        _root->addToGroup(tileGroup);
        _zoomTileGroups.insert(key1, tileGroup);
    }

    // Prepare runnable task to load tiles:
    QList<TilesLoadTask::TileToLoad> tiles;
    QList<QString> visibleTilesInCache;
    double scale = qPow(2.0, -1.0*zoomLevel);
    for (int i=0;i<nbXTilesAtZ;i++)
    {
        double x = i*tileSize;
        for (int j=0;j<nbYTilesAtZ;j++)
        {
            double y = j*tileSize;

            QRectF tileSceneRect(scale*x + pos().x(),
                                scale*y + pos().y(),
                                scale*tileSize,
                                scale*tileSize);

            if (visibleSceneRect.intersects(tileSceneRect))
            {
                QString key2=QString("Tile_%1_%2_%3")
                        .arg(i)
                        .arg(j)
                        .arg(zoomLevel);

#ifdef GEOIMAGEITEM_CACHE_VERBOSE
                SD_TRACE("---- Load : " + key2);
#endif

                if (!_tilesCache.contains(key2))
                {
                    QRect tileExtent = QRect(scale*x,
                                             scale*y,
                                             scale*tileSize,
                                             scale*tileSize);
                    tiles << TilesLoadTask::TileToLoad(scale*x + pos().x(),
                                                       scale*y + pos().y(),
                                                       scale,
                                                       tileExtent,
                                                       tileSize,
                                                       key2,
                                                       tileGroup);
                }
                else
                {
                    visibleTilesInCache << key2;
                }
            }
        }
    }
    // start thread pool
    if (!tiles.isEmpty())
    {
        // Maintain cache:
        maintainCache(visibleTilesInCache, tiles.size());

        // Cancel previous work & load new tiles:
        _task->setTilesToLoad(tiles);
        QThreadPool * pool = QThreadPool::globalInstance();
        if (pool->waitForDone())
        {
            // Start at most 'maxNbOfThreads' threads
            for (int i=0; i<qMin(_settings.MaxNbOfThreads, pool->maxThreadCount());i++)
            {
                pool->start(_task);
            }
        }
        else
        {
            SD_TRACE("GeoImageItem::updateItem : waitForDone returns false");
        }
    }
}

//******************************************************************************

//void GeoImageItem::onDataChanged(const QRect &extent)
//{
//    SD_TRACE("GeoImageItem::onDataChanged(const QRect &extent)");
//}

//******************************************************************************

void GeoImageItem::maintainCache(const QList<QString> &visibleTilesInCache, int nbOfTilesToAdd)
{
    int startIndex = 0;
    while (_tilesCacheHistory.size() + nbOfTilesToAdd > _settings.CacheSize)
    {
        for (int i=startIndex;i<_tilesCacheHistory.size();i++)
        {
            QString key = _tilesCacheHistory[i];
            if (!visibleTilesInCache.contains(key))
            {

#ifdef GEOIMAGEITEM_CACHE_VERBOSE
                SD_TRACE("---- Remove " + key);
#endif
                QGraphicsItem * tile = _tilesCache[key];
                _tilesCacheHistory.removeAt(i);
                scene()->removeItem(tile);
                _tilesCache.remove(key);
                delete tile;

#ifdef GEOIMAGEITEM_SHOW_CACHE_INFO
                showCacheInfo();
#endif

                break;
            }
            startIndex++;
        }
    }
}

//******************************************************************************

void GeoImageItem::onTileLoaded(QGraphicsPixmapItem * tile, QGraphicsItemGroup *tileGroup, const QString & key)
{

    // Do not need mutex here, because the slot is connected with Queued Connection
#ifdef GEOIMAGEITEM_CACHE_VERBOSE
    SD_TRACE("onTileLoaded : " + key);
#endif

    tileGroup->addToGroup(tile);
    _tilesCache.insert(key, tile);
    _tilesCacheHistory.append(key);


#ifdef GEOIMAGEITEM_DISPLAY_TILES
    SD_TRACE("Red : " + key);
    QRectF tileR = tile->boundingRect();
    QGraphicsRectItem * rr = new QGraphicsRectItem(tileR, tile);
    rr->setZValue(10.0);
    rr->setPen(QPen(Qt::black, 0));
    rr->setBrush(QColor(210,10,10,12));
    QGraphicsSimpleTextItem * tt = new QGraphicsSimpleTextItem(key, rr);
    tt->setPos(tileR.center());
    tt->setPen(QPen(Qt::white, 0));
    tt->setBrush(Qt::white);
#endif

#ifdef GEOIMAGEITEM_SHOW_CACHE_INFO
    showCacheInfo();
#endif

}

//******************************************************************************

void GeoImageItem::showCacheInfo()
{
    int n(1), a(0);
#ifdef GEOIMAGEITEM_DISPLAY_TILES
    n+=2;
#endif

#ifdef GEOIMAGEITEM_SHOW_VIEWPORT
    a=1;
#endif

    SD_TRACE(QString("===== CACHE INFO : tileCache = %1 | tileCacheHistory = %2 | TEST : 1 = %3")
             .arg(_tilesCache.size())
             .arg(_tilesCacheHistory.size())
             .arg(_tilesCache.size() <= _tilesCacheHistory.size()));
    SD_TRACE(QString("===== CACHE INFO : scene items = %1 | zoomTileGroups = %2 | TEST : 1 = %3")
             .arg(scene()->items().size())
             .arg(_zoomTileGroups.size())
             .arg(scene()->items().size() == _zoomTileGroups.size() + _tilesCacheHistory.size()*n + a)
             );
}

//******************************************************************************
//******************************************************************************

void TilesLoadTask::setTilesToLoad(const QList<TilesLoadTask::TileToLoad> &tileList)
{
    QMutexLocker locker(&_mutex);
    _canceled=true;
    _tilesToLoad.clear();
    _tilesToLoad = tileList;
}

//******************************************************************************

void TilesLoadTask::run()
{

#ifdef GEOIMAGEITEM_TIMER_ON
    StartTimer("Start load tiles");
#endif

    // !!! WHY THERE IS NO MUTEX LOCK FOR _canceled ???
    _canceled=false;
    int counter=0;
    while (counter<1000) {
        counter++;
        // Fetch task:
        _mutex.lock();
        if (_tilesToLoad.isEmpty() || _canceled)
        {

#ifdef GEOIMAGEITEM_CACHE_VERBOSE
            SD_TRACE("LoadTilesTask::run : no tiles to load OR cancelled -> exit");
#endif

#ifdef GEOIMAGEITEM_TIMER_ON
            StopTimer();
#endif
            _mutex.unlock();
            return;
        }


        TileToLoad t = _tilesToLoad.takeFirst();
#ifdef GEOIMAGEITEM_CACHE_VERBOSE
        SD_TRACE(QString("LoadTilesTask::run : load ") + t.cacheKey);
#endif
        _mutex.unlock();


        // PROCESS DATA LOCALLY: mutually exists {data, r} and {r,p}
        QGraphicsPixmapItem * tile = 0;
        {
            cv::Mat r;
            QPixmap p;
            {

                // Do work:
                // GDAL dataset access should be limited to only one thread per IO
//                _mutex.lock();
                cv::Mat data = _item->_dataProvider->getImageData(t.tileExtent, t.tileSize);
//                _mutex.unlock();
                if (data.empty())
                    continue;

                // Render data:
#ifdef RENDERER_TIMER_ON
                StartTimer("render");
#endif
                r = _item->_renderer->render(data, _item->_rconf, true);
#ifdef RENDERER_TIMER_ON
                StopTimer();
#endif
                if (r.empty())
                    continue;
            }
            // Here cv::Mat data is deleted

            // Data is copied into QPixmap
            p = QPixmap::fromImage(QImage(r.data, r.cols, r.rows, QImage::Format_ARGB32).copy());
            tile = new QGraphicsPixmapItem(p);
            tile->setTransform(
                        QTransform::fromScale(t.scale, t.scale) *
                        QTransform::fromTranslate(t.x, t.y)
                        );

        }
        // Here cv::Mat r and QPixmap p are released. Data is stored as QPixmap in the QGraphicsPixmapItem

        // Store :
        if (!_canceled)
            emit tileLoaded(tile, t.tileGroup, t.cacheKey);
        else
        {
            delete tile;
        }

    }

#ifdef LAYERLOADER_TIMER_ON
    StopTimer();
#endif

}

//******************************************************************************

}
