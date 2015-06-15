# Helper scripts to create geo-images using GDAL

# Python
import os

# Numpy
import numpy

# GDAL
import gdal
import osr
import ogr
from gdalconst import *

# Opencv
import cv2

# Project
import Global
import ImageCommon

def polygonizeToKML(image, filename, layerName="Layer_name"):
        """
        Method to polygonize image data into a vector file (.kml)
        Input image is polygonized using gdal polygonize and a band mask created from the input image with value equal 0.
        """
        ImageCommon.assertOneBand(image)
        assert image.dtype == np.uint8, Global.logPrint("Input image should be of type uint8",'error')

        # Create ogr dataset
        drv = ogr.GetDriverByName("KML")
        assert drv is not None, Global.logPrint("Failed to find KML driver to write vector file", 'error')

        if os.path.exists(filename):
            os.remove(filename)

##        outVectorDS = drv.CreateDataSource(filename)
##        assert outVectorDS is not None, Global.logPrint("Failed to create vector file : " + filename, 'error')
##
##        ogrLayer = outVectorDS.CreateLayer(layerName, None, ogr.wkbPolygon)
##        assert ogrLayer is not None, Global.logPrint("Failed to create layer in the vector file : " + filename, 'error')

        # Create bands from input image
            # create virtual dataset
        drv = gdal.GetDriverByName("VRT")
        assert drv is not None, Global.logPrint("Failed to find VRT driver to create a virtual dataset", 'error')
        vrtDS = drv.Create("", image.shape[1], image.shape[0], 1, convertNumpyTypeToGDAL(image.dtype))
        assert vrtDS is not None, Global.logPrint("Failed to create virtual dataset", 'error')

        band = vrtDS.GetRasterBand(1)
        assert band is not None, Global.logPrint("Failed to create virtual dataset band", 'error')
        band.WriteArray(image, 0, 0)
        band.SetNoDataValue(0)
        band.FlushCache()

        print image[30:40,430:435]

        im2 = band.ReadAsArray(0,0)
        print im2[30:40,430:435]
        print im2[430:435, 30:40]

        maskBand = band.GetMaskBand()
        assert maskBand is not None, Global.logPrint("Failed to create virtual dataset band mask", 'error')

##        gdal.Polygonize(band, maskBand, ogrLayer, 0)
        # close
##        outVectorDS = None
        vrtDS = None




def writeVectorFromMask(image, filename, layerName="Layer_name", layerFormat="ESRI Shapefile"):
        """
        Method to polygonize image data into a vector file (in layerFormat)
        Input image should be single band of type uint8.
        Input image is polygonized using opencv findContours method.
        """
        ImageCommon.assertOneBand(image)
        assert image.dtype == numpy.uint8, Global.logPrint("Input image should be of type uint8",'error')

        # Create ogr dataset
        drv = ogr.GetDriverByName(layerFormat)
        assert drv is not None, Global.logPrint("Failed to find driver : '" + layerFormat + "' to write vector file", 'error')

        if os.path.exists(filename):
            os.remove(filename)
        outVectorDS = drv.CreateDataSource(filename)
        assert outVectorDS is not None, Global.logPrint("Failed to create vector file : " + filename, 'error')

        ogrLayer = outVectorDS.CreateLayer(layerName, None, ogr.wkbPolygon)
        assert ogrLayer is not None, Global.logPrint("Failed to create layer in the vector file : " + filename, 'error')

        # Create contours :
        # - contours represents array of array of contour points
        # - hierarchy represents array of (next, prev, 1st child, parent) contour indices
        # e.g. if hierarchy[i]["parent"] = -1 -> outer contour
        # e.g. if hierarchy[i]["parent"] > 0 -> inner contour

        contours, hierarchy = cv2.findContours( image.copy(), cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
        if len(hierarchy) > 0:
            hierarchy = hierarchy[0]
            index = 0
            while (index >= 0):
                # if has parent -> skip because it has been already added
                if hierarchy[index][3] >= 0:
                    continue

                contour = contours[index]
                # create new feature
                feature = ogr.Feature( ogrLayer.GetLayerDefn() )
                geom = None
                if len(contour) == 1 and "KML" in layerFormat:
                    geom=ogr.Geometry(ogr.wkbPoint)
                    geom.AddPoint(contour[0][0][0],contour[0][0][1])
                else:
                    geom=ogr.Geometry(ogr.wkbPolygon)
                    ring = ogr.Geometry(ogr.wkbLinearRing)
                    for point in contour:
                        ring.AddPoint(point[0][0], point[0][1])
                    # close ring
                    ring.AddPoint(contour[0][0][0],contour[0][0][1])
                    geom.AddGeometry(ring)
                    # check if has children
                    i = hierarchy[index][2]
                    while (i >= 0):
                        # check if contour has the same parent
                        ring = ogr.Geometry(ogr.wkbLinearRing)
                        contour = contours[i]
                        for point in contour:
                            ring.AddPoint(point[0][0], point[0][1])
                        # close ring
                        ring.AddPoint(contour[0][0][0],contour[0][0][1])
                        geom.AddGeometry(ring)
                        # choose next contour of the same hierarchy
                        i = hierarchy[i][0]
                feature.SetGeometry(geom)
                assert ogrLayer.CreateFeature(feature) == 0, Global.logPrint("Failed to create feature in vector file")
                feature.Destroy()
                # choose next contour of the same hierarchy
                index = hierarchy[index][0]

        # Close vector dataset
        outVectorDS = None



def writeDataInGeoImage(filename, numpyArrayData, originAndStep=[[0.0,0.0],[1.0,-1.0]]):
    """
        Method to write a image file using GDAL. Output file will be in WGS84 coordinates.
        numpyArrayData - input data is given as numpy.array and should be in the following form:
            numpyArrayData.shape = (nbBands, height, width) or
            numpyArrayData.shape = (height, width)
        originAndStep - origin and step of the file, e.g.= [[originX,originY],[stepX,stepY]]
    """
    ImageCommon.assertImage(numpyArrayData)
    assert len(originAndStep) == 2 and len(originAndStep[0]) == 2 and len(originAndStep[1]), Global.logPrint("OriginAndStep argument should be of form [[originX,originY],[stepX,stepY]]", 'error')

    driver = gdal.GetDriverByName("GTiff")
    assert driver is not None, Global.logPrint("Failed to get GTiff driver", 'error')

    dataType=convertNumpyTypeToGDAL(numpyArrayData.dtype)
    assert dataType is not gdal.GDT_Unknown, Global.logPrint("Failed to convert input data type into gdal data type", 'error')

    if len(numpyArrayData.shape) == 2:
        nbBands = 1
        width = numpyArrayData.shape[1]
        height = numpyArrayData.shape[0]
    else:
        nbBands = numpyArrayData.shape[0]
        width = numpyArrayData.shape[2]
        height = numpyArrayData.shape[1]

    dstDataset=driver.Create(filename, width, height, nbBands, dataType)
    assert dstDataset is not None, Global.logPrint("Failed to create file : " + filename, 'error')

    _setWGS84GeoInfo(dstDataset, originAndStep[0], originAndStep[1])

    for bandIndex in range(1,nbBands+1):
        bandData = numpyArrayData[bandIndex-1,:,:] if nbBands > 1 else numpyArrayData
        dstBand = dstDataset.GetRasterBand(bandIndex)
        dstBand.WriteArray(bandData, 0, 0)


    Global.logPrint("- output file " + filename + " is written")
    dstDataset = None


def convertNumpyTypeToGDAL(dtype):
    """ Method to convert numpy data type to Gdal data type """
    if dtype == numpy.uint8:
        return gdal.GDT_Byte
    elif dtype == numpy.int16:
        return gdal.GDT_Int16
    elif dtype == numpy.int32:
        return gdal.GDT_Int32
    elif dtype == numpy.uint16:
        return gdal.GDT_UInt16
    elif dtype == numpy.uint32:
        return gdal.GDT_UInt32
    elif dtype == numpy.float32:
        return gdal.GDT_Float32
    elif dtype == numpy.float64:
        return gdal.GDT_Float64
    elif dtype == numpy.complex64:
        return gdal.GDT_CFloat32
    elif dtype == numpy.complex128:
        return gdal.GDT_CFloat64
    else:
        return gdal.GDT_Unknown



def _setWGS84GeoInfo(dstDataset, origin, step):
    sr = osr.SpatialReference()
    sr.ImportFromEPSG(4326)
    proj = sr.ExportToWkt()
    dstDataset.SetProjection(proj)
    geoTransform = [0.0]*6
    geoTransform[0] = origin[0]
    geoTransform[3] = origin[1]
    geoTransform[2] = 0.0
    geoTransform[4] = 0.0
    geoTransform[1] = step[0]
    geoTransform[5] = step[1]
    dstDataset.SetGeoTransform(geoTransform)


if __name__ == '__main__':


# Test polygonize image mask
    mask = numpy.zeros((2000, 2500), dtype=numpy.uint8)
    # 2 points
    mask[1935,638] = 1
    mask[1900,230] = 1

    # 4 Polygons
    mask[535:537,130:132] = 1

    mask[1535:1537,2130:2132] = 1

    mask[35:100,430:455] = 1
    mask[55:65,440:445] = 0

    mask[1135:1140,1430:1635] = 1

    mask[400:430,750:800] = 1
    mask[415:420,770:780] = 0
    mask[422:424,790:795] = 0


    contours, hierarchy = cv2.findContours( mask.copy(), cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
    if len(hierarchy) > 0:
        hierarchy = hierarchy[0]
##        print hierarchy
        # use polygon approximation
        index = 0
        while (index >= 0):
##            print index, hierarchy[index]
            # if has parent -> skip because it has been already added
            if hierarchy[index][3] >= 0:
                continue

            contour = contours[index]
##            contour = cv2.approxPolyDP(contours[index], 3, True)
            # create new feature
            geom = []

            if len(contours[index]) == 1:
                geom.append([contour[0][0][0],contour[0][0][1]])
            else:
                ring = []
                for point in contour:
                    ring.append([point[0][0], point[0][1]])
                # close ring
                ring.append([contour[0][0][0],contour[0][0][1]])
                geom.append(ring)
                # check if has children
                i = hierarchy[index][2]
                while (i >= 0):
                    # check if contour has the same parent
##                    print "inner :", i, hierarchy[i]
                    ring = []
##                    contour = cv2.approxPolyDP(contours[i], 3, True)
                    contour = contours[i]
                    for point in contour:
                        ring.append([point[0][0], point[0][1]])
                    # close ring
                    ring.append([contour[0][0][0],contour[0][0][1]])
                    geom.append(ring)
                    # choose next contour of the same hierarchy
                    i = hierarchy[i][0]
            print "geom:", geom
            # choose next contour of the same hierarchy
            index = hierarchy[index][0]


##    writeDataInGeoImage("test.tif", mask)
    writeVectorFromMask(mask, "test.shp", "ESRI Shapefile")

    exit()

# Test single band image:
    data = numpy.zeros((100, 100), dtype=numpy.int16)
    data[20:30, 30:40] = 1234
    data[70:95, 50:70] = 2345
    writeDataInGeoImage("C:/Temp/test_1Band.tif",data, [[-52, 5], [0.0001,-0.0001]])


# Test 3 bands image:
    data = numpy.zeros((3, 100, 100), dtype=numpy.int16)
    data[0, 20:30, 30:40] = 1234
    data[1, 20:30, 30:40] = 134
    data[1, 70:95, 50:70] = 2345
    data[2, 70:95, 50:70] = 245
    writeDataInGeoImage("C:/Temp/test_3Bands.tif",data, [[-52, 5], [0.0001,-0.0001]])

