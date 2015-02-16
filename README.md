GeoImageViewer
==============






TODO: 

1 Create zoom overlay

2 Memory leak with cache
	When a QGraphicsPixmapItem is removed from scene, it is also removed from QGraphicsItemGroup
	SOLUTION : Need delete explicitly QGraphicsPixmapItem 