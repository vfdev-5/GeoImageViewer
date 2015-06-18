#-------------------------------------------------------------------------------
# Name:        Geo Vector
# Purpose:      Class structure for reading geo vector files
#
# Author:      vfomin
#
# Created:     08/04/2015
# Copyright:   (c) vfomin 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------

"""
    class GeoVector defines a structure to handle geo shape files
    Methods to use :
        open(filename)

"""

# Python
import os
import time

# Numpy
import numpy as np

# GDAL/OGR
import ogr
import osgeo.osr

# Opencv
import cv2

# Project
import Global
import ImageCommon
import ImageTools
import ImageMetadataExtractor


# Geo Vector Class
# ###############################################################################

class GeoVector:

    class Feature:
        """
        Feature structure represents a wrap on OGRFeature
        Attributes to use :
            - self.geoPoints is a numpy array of all geo points of the feature geometry (we assume that 1 feature has 1 geometry, no multi-geometries)
                Points are ordered in the clock-wise direction. First and last points are the same for polygons
            - self.geoCenter is numpy array of type [cX, cY] representing the centroid of the geometry
            - self.geoBBox is numpy array of bounding box points : top-left, top-right, bottom-right, bottom-left
            - self.attributes is a dictionary of type {field_name, field_value}
            - self.ogrGeometry is ogr Geometry of the layer's feature
        """
        def __init__(self):
            self.geoPoints = None
            self.geoCenter = None
            self.geoBBox = None
            self.attributes = None
            self.ogrGeometry = None


    def __init__(self, filename=""):
        self._dataset = None
        self.features = None
        self.tableHeaders = None

        if len(filename) > 0:
            self.open(filename)


    def open(self, filename):
        """
            Method to load vector shape from filename
            The file should contain single layer (as .shp files)

            Attributes to use :
                - self.features is an array of Feature
                - self.tableHeaders is an array of field names

        """
        assert os.path.exists(filename), Global.logPrint("Filename is not found",'error')

        self._dataset = ogr.Open(filename)
        assert self._dataset is not None, Global.logPrint("Failed to open the file:" + filename, 'error')

        assert self._dataset.GetLayerCount() == 1, Global.logPrint("File should contain only one layer.", 'error')
        for layer in self._dataset:
            fd = layer.GetLayerDefn()
            size = fd.GetFieldCount()
            self.tableHeaders = [None]*size
            for index in range(size):
                f = fd.GetFieldDefn(index)
                self.tableHeaders[index] = f.GetName()

            self.features = [None]*layer.GetFeatureCount()
            for count, feature in enumerate(layer):
                f = GeoVector.Feature()
                f.attributes = feature.items()
                geom = feature.geometry()
                c = geom.Centroid()
                f.geoCenter = np.array([[c.GetX(), c.GetY()]])

                # GetGeometryCount > 0 in case of POLYGON:
                if geom.GetGeometryCount() != 0:
                    if geom.GetGeometryCount() > 1:
                        Global.logPrint("Input shape should not contain multi-geometries. Script takes only the first geometry")
                    geom = geom.GetGeometryRef(0)
                    f.geoPoints = GeoVector._getPointsFromGeometry(geom)
                    env = geom.GetEnvelope()
                    # env is (minX, maxX, minY, maxY) -> TL, TR, BR, BL
                    f.geoBBox = np.array([[env[0], env[3]], [env[1], env[3]], [env[1], env[2]], [env[0], env[2]]])
                    f.ogrGeometry = geom

                self.features[count] = f



    @staticmethod
    def _getPointsFromGeometry(ogrGeometry):
        nbPts = ogrGeometry.GetPointCount()
        if nbPts == 0:
            return None
        geoPoints = np.zeros((nbPts,2))
        for index in range(nbPts):
            geoPoints[index, 0] = ogrGeometry.GetX(index)
            geoPoints[index, 1] = ogrGeometry.GetY(index)
        return geoPoints



# Test methods
# ###############################################################################

if __name__ == '__main__':


    # TEST 1 :
    shpfilename="C:\\VFomin_folder\\PISE_project\\IRT\Data\\Projets_TPZ\\Gabon\\SHP\\Slick_Detection_2009-03-12_09-10-22.shp"
    vector = GeoVector(shpfilename)

    # check table header :
    print vector.tableHeaders

    # display features :
    for feature in vector.features:
        # attributes, geo points, geo bbox
        print feature.attributes
        print len(feature.geoPoints), feature.geoPoints
        print feature.geoBBox
        print feature.geoCenter

    print len(vector.features)







