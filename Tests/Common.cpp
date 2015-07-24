
// Qt
#include <QPointF>
#include <qmath.h>

// Tests
#include "Common.h"
#include "Core/Global.h"

namespace Tests {

//*************************************************************************

bool compareVectors(const QVector<double> & v1, const QVector<double> & v2, double tol)
{
    if (v1.isEmpty() && v2.isEmpty())
    {
        SD_TRACE("Compared vectors are empty");
    }

    if (v1.size() != v2.size())
        return false;

    for (int i=0;i<v1.size();i++)
    {
        if (qAbs(v1[i] - v2[i]) > tol)
            return false;
    }
    return true;
}

//*************************************************************************

bool comparePolygons(const QPolygonF & v1, const QPolygonF & v2, double tol)
{
    if (v1.isEmpty() && v2.isEmpty())
    {
        SD_TRACE("Compared polygons are empty");
    }


    if (v1.size() != v2.size())
        return false;

    for (int i=0;i<v1.size();i++)
    {
        QPointF p = v1[i] - v2[i];
        if (qSqrt(p.x()*p.x() + p.y()*p.y()) > tol)
            return false;
    }
    return true;
}

//*************************************************************************

void printGeoExtent(const QPolygonF & poly)
{
    SD_TRACE("---- Print GeoExtent ----");
    foreach (QPointF p, poly)
    {
        SD_TRACE(QString("Point : %1, %2").arg(p.x()).arg(p.y()));
    }
    SD_TRACE("---- END Print GeoExtent ----");
}

//*************************************************************************

}

