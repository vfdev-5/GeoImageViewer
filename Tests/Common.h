#ifndef COMMON_H
#define COMMON_H

// Qt
#include <QVector>
#include <QPolygonF>
#include <QColor>

// Opencv
#include <opencv2/core/core.hpp>

namespace Tests {

//*************************************************************************

bool compareVectors(const QVector<double> & v1, const QVector<double> & v2, double tol = 1e-8);

bool comparePolygons(const QPolygonF & v1, const QPolygonF & v2, double tol = 1e-5);

void printGeoExtent(const QPolygonF & poly);

inline bool testRGBAColor(const cv::Vec4b & p, int r, int g, int b, int a)
{
    return p[0] == r && p[1] == g && p[2] == b && p[3] == a;
}

inline bool testBGRAColor(const cv::Vec4b & p, int r, int g, int b, int a)
{
    return p[2] == r && p[1] == g && p[0] == b && p[3] == a;
}

inline bool testRGBAColor2(const QColor & p, int r, int g, int b, int a)
{
    return p.red() == r && p.green() == g && p.blue() == b && p.alpha() == a;
}

//*************************************************************************

}


#endif // COMMON_H
