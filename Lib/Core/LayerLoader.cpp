
// Qt
#include <qmath.h>
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QThreadPool>

// Project
#include "LayerTools.h"
#include "LayerLoader.h"
#include "ImageLayer.h"
#include "LayerRenderer.h"


namespace Core
{

#ifdef LAYERLOADER_DISPLAY_VIEWPORT
    QGraphicsRectItem * VIEWPORT = 0;
#endif

//******************************************************************************
/*!
  * \class LayerLoader
  * \brief It is responsible to load image data to graphicsscene asynchronously.
  *  Data is loaded by tiles and stored in a cache.
  *
  * Class instance should have a ImageLayer, configured LayerRenderer and QGraphicsScene.
  * In method updateLayer() we compute the tiles to load and start the asynchronous task.
  * Tile size and cache size (in number of tiles) can be configured in Settings.
  *
  * Cache is represented by two Hash maps: 1) groups of tiles at one zoom level and 2) tiles
  * Thus, QGraphicsScene has QGraphicsItemGroups with QGraphicsPixmapItem.
  *
  */
//******************************************************************************

LayerLoader::LayerLoader(QObject *parent) :
    QObject(parent),
    _layer(0),
    _renderer(0),
    _scene(0),
    _nbXTiles(0),
    _nbYTiles(0)
{
    _loadTilesTask = new LoadTilesTask(this);
    connect(_loadTilesTask, SIGNAL(tileLoaded(QGraphicsPixmapItem*,QGraphicsItemGroup*,QString)),
            this, SLOT(onTileLoaded(QGraphicsPixmapItem*,QGraphicsItemGroup*,QString)));
    // We do not use Qt::BlockingQueuedConnection because of deadlocks :
    // when calls _loadTilesTask->setTilesToLoad(tiles) in Main Thread and
    // onTileLoaded() in the same thread


}

//******************************************************************************

LayerLoader::~LayerLoader()
{
    // wait until last processed work is done
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
#endif
    if (pool->waitForDone())
    {
        // scene is already cleared -> tiles and cache also
        delete _loadTilesTask;
    }
}

//******************************************************************************
/*!
 * Method to reset loader : clear cache, remove layer etc
 */
void LayerLoader::clear()
{
    if (_layer)
    {
        clearCache();
        _layer = 0;
        _nbXTiles = 0;
        _nbYTiles = 0;
    }

}

//******************************************************************************
/*!
 * Method to clear cache
 */

void LayerLoader::clearCache()
{

#ifdef LAYERLOADER_CACHE_VERBOSE
    SD_TRACE("---- Clear Cache : ")
#endif

    // wait until last processed work is done
    QThreadPool * pool = QThreadPool::globalInstance();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    pool->clear();
    _loadTilesTask->cancel();
#endif
    if (pool->waitForDone())
    {

#ifdef LAYERLOADER_CACHE_VERBOSE
        SD_TRACE("---- Clear Cache : start clearing")
#endif
        // Clean layer dependant data
        foreach(QGraphicsItemGroup* item, _zoomTileGroups.values())
        {
            _scene->removeItem(item);
            delete item;
        }
        _zoomTileGroups.clear();
        _tilesCache.clear();
        _tilesCacheHistory.clear();
#ifdef LAYERLOADER_CACHE_VERBOSE
        SD_TRACE("---- Clear Cache : end clearing")
#endif

    }
    else
    {
        SD_TRACE("LayerLoader::clearCache : waitForDone returns false");
    }
}

//******************************************************************************
/*!
 * \brief LayerLoader::computeZoomMinLevel
 * \return Min zoom level. It is a negative value computed using the condition :
 *  Image at min zoom level <= TileSize <-> max(W,H) * 2^(zMin) <= TileSize
 */
int LayerLoader::computeZoomMinLevel()
{
    if (!_layer)
        return 0;
    int zoomMinLevel= -qCeil(qLn( qMin(_nbXTiles,_nbYTiles) )/qLn(2.0));
    zoomMinLevel = (zoomMinLevel > 0) ? 0 : zoomMinLevel;
    return zoomMinLevel;
}

//******************************************************************************

void LayerLoader::setLayer(ImageLayer * layer)
{
    _layer = layer;
    _nbXTiles = qCeil(_layer->getWidth()*1.0/_settings.TileSize);
    _nbYTiles = qCeil(_layer->getHeight()*1.0/_settings.TileSize);
}

//******************************************************************************

void LayerLoader::updateLayer(int nZoomLevel, const QRect &nVisiblePixelExtent)
{

    int zoomLevel = (nZoomLevel > 0) ? 0 : nZoomLevel;
    int nbXTilesAtZ=qCeil(_nbXTiles*qPow(2.0,zoomLevel));
    int nbYTilesAtZ=qCeil(_nbYTiles*qPow(2.0,zoomLevel));
    int tileSize = _settings.TileSize;

    // enlarge nVisibleSceneRect
    QRect visibleSceneRect = nVisiblePixelExtent.adjusted(-10.0,
                                                         -10.0,
                                                         +10.0,
                                                         +10.0);

#ifdef LAYERLOADER_DISPLAY_VIEWPORT
    if (VIEWPORT)
    {
        _scene->removeItem(VIEWPORT);
        delete VIEWPORT;
    }
    VIEWPORT = _scene->addRect(visibleSceneRect);
    VIEWPORT->setZValue(10.0);
    VIEWPORT->setPen(QPen(Qt::black, 0));
    VIEWPORT->setBrush(QColor(10,10,180,22));
#endif


#ifdef LAYERLOADER_SHOW_CACHE_INFO
    showCacheInfo();
#endif


    // Clear previous tiles :
    foreach (QGraphicsItemGroup * group, _zoomTileGroups)
    {
        group->setVisible(false);
    }

    // Show current zoom tile group:
    QString key1=QString("TileGroup_%3").arg(zoomLevel);
    QGraphicsItemGroup * tileGroup=0;
    if (_zoomTileGroups.contains(key1))
    {
        tileGroup=_zoomTileGroups[key1];
        tileGroup->setVisible(true);
    }
    else
    {
        tileGroup = new QGraphicsItemGroup();
        _scene->addItem(tileGroup);
        _zoomTileGroups.insert(key1, tileGroup);
    }

    // Prepare runnable task to load tiles:
    QList<LoadTilesTask::TileToLoad> tiles;
    QList<QString> visibleTilesInCache;
    double scale = qPow(2.0, -1.0*zoomLevel);
    for (int i=0;i<nbXTilesAtZ;i++)
    {
        double x = i*tileSize;
        for (int j=0;j<nbYTilesAtZ;j++)
        {
            double y = j*tileSize;
            QRect tileExtent = QRect(scale*x,
                                     scale*y,
                                     scale*tileSize,
                                     scale*tileSize);

            if (visibleSceneRect.intersects(tileExtent))
            {
                QString key2=QString("Tile_%1_%2_%3")
                        .arg(i)
                        .arg(j)
                        .arg(zoomLevel);

#ifdef LAYERLOADER_CACHE_VERBOSE
                SD_TRACE("---- Load : " + key2);
#endif

                if (!_tilesCache.contains(key2))
                {
                    tiles << LoadTilesTask::TileToLoad(x,
                                                       y,
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
        _loadTilesTask->setTilesToLoad(tiles);
        QThreadPool * pool = QThreadPool::globalInstance();
        if (pool->waitForDone())
        {
            // Only one thread is possible due to GDAL reader (e.g. TIFF)
            pool->start(_loadTilesTask);
        }
        else
        {
            SD_TRACE("LayerLoader::updateLayer : waitForDone returns false");
        }
    }
}

//******************************************************************************

void LayerLoader::maintainCache(const QList<QString> &visibleTilesInCache, int nbOfTilesToAdd)
{
    int startIndex = 0;
    while (_tilesCacheHistory.size() + nbOfTilesToAdd > _settings.CacheSize)
    {
        for (int i=startIndex;i<_tilesCacheHistory.size();i++)
        {
            QString key = _tilesCacheHistory[i];
            if (!visibleTilesInCache.contains(key))
            {

#ifdef LAYERLOADER_CACHE_VERBOSE
                SD_TRACE("---- Remove " + key);
#endif
                QGraphicsItem * tile = _tilesCache[key];
                _tilesCacheHistory.removeAt(i);
                _scene->removeItem(tile);
                _tilesCache.remove(key);
                delete tile;

#ifdef LAYERLOADER_SHOW_CACHE_INFO
                showCacheInfo();
#endif

                break;
            }
            startIndex++;
        }
    }
}

//******************************************************************************

void LayerLoader::onTileLoaded(QGraphicsPixmapItem * tile, QGraphicsItemGroup *tileGroup, const QString & key)
{

    // Do not need mutex here, because the slot is connected with Queued Connection
#ifdef LAYERLOADER_CACHE_VERBOSE
    SD_TRACE("onTileLoaded : " + key);
#endif
    tileGroup->addToGroup(tile);
    _tilesCache.insert(key, tile);
    _tilesCacheHistory.append(key);


#ifdef LAYERLOADER_DISPLAY_TILES
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

#ifdef LAYERLOADER_SHOW_CACHE_INFO
    showCacheInfo();
#endif


}

//******************************************************************************

void LayerLoader::showCacheInfo()
{
    int n(1), a(0);
#ifdef LAYERLOADER_DISPLAY_TILES
    n+=2;
#endif

#ifdef LAYERLOADER_SHOW_VIEWPORT
    a=1;
#endif

    SD_TRACE(QString("===== CACHE INFO : tileCache = %1 | tileCacheHistory = %2 | TEST : 1 = %3")
             .arg(_tilesCache.size())
             .arg(_tilesCacheHistory.size())
             .arg(_tilesCache.size() <= _tilesCacheHistory.size()));
    SD_TRACE(QString("===== CACHE INFO : scene items = %1 | zoomTileGroups = %2 | TEST : 1 = %3")
             .arg(_scene->items().size())
             .arg(_zoomTileGroups.size())
             .arg(_scene->items().size() == _zoomTileGroups.size() + _tilesCacheHistory.size()*n + a)
             );
}


//******************************************************************************
//******************************************************************************

void LoadTilesTask::setTilesToLoad(const QList<LoadTilesTask::TileToLoad> &tileList)
{
    QMutexLocker locker(&_mutex);
    _canceled=true;
    _tilesToLoad.clear();
    _tilesToLoad = tileList;
}

//******************************************************************************

void LoadTilesTask::run()
{

#ifdef LAYERLOADER_TIMER_ON
    StartTimer("Start load tiles");
#endif

    _canceled=false;
    int counter=0;
    while (counter<1000) {
        counter++;
        // Fetch task:
        _mutex.lock();
        if (_tilesToLoad.isEmpty() || _canceled)
        {

#ifdef LAYERLOADER_CACHE_VERBOSE
            SD_TRACE("LoadTilesTask::run : no tiles to load OR cancelled -> exit");
#endif

#ifdef LAYERLOADER_TIMER_ON
            StopTimer();
#endif
            _mutex.unlock();
            return;
        }


        TileToLoad t = _tilesToLoad.takeFirst();
#ifdef LAYERLOADER_CACHE_VERBOSE
        SD_TRACE(QString("LoadTilesTask::run : load ") + t.cacheKey);
#endif
        _mutex.unlock();


        // Do work:
            // GDAL dataset access should be limited to only one thread per IO
        _mutex.lock();
        cv::Mat data = _loader->_layer->getImageData(t.tileExtent, t.tileSize);
        _mutex.unlock();
        if (data.empty())
            continue;

        // Render data:
#ifdef RENDERER_TIMER_ON
    StartTimer("render");
#endif
    cv::Mat r = _loader->_renderer->render(data);
#ifdef RENDERER_TIMER_ON
    StopTimer();
#endif

        // Data is copied in QPixmap
        QPixmap p = QPixmap::fromImage(QImage(r.data, r.cols, r.rows, QImage::Format_RGB888));
        QGraphicsPixmapItem * tile = new QGraphicsPixmapItem(p);
        tile->setTransform(
                    QTransform::fromTranslate(t.x, t.y) *
                    QTransform::fromScale(t.scale, t.scale)
                    );

        // Store :
        if (!_canceled)
            emit tileLoaded(tile, t.tileGroup, t.cacheKey);

    }

#ifdef LAYERLOADER_TIMER_ON
    StopTimer();
#endif

}

//******************************************************************************

}
