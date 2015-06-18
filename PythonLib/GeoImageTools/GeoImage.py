#-------------------------------------------------------------------------------
# Name:        Geo Image
# Purpose:      Class structure for reading geo image files
#
# Author:      vfomin
#
# Created:     08/04/2015
# Copyright:   (c) vfomin 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------

"""
    class GeoImage defines a structure to handle geo images
    Methods to use :
        open()
        getImageResolutionInDegrees()
        showMetadata()
        fetchValues()
        transform()

    Attributes to use :
        GeoImage.array ( numpy.array of shape (height,width,nbBands) ) represents image data
        GeoImage.geoExtent is an array of 4 points (long, lat) : [[left,top], [right,top], [right,bottom], [left,bottom]]
        GeoImage.metadata is an array of image metadata
"""

# Python
import os
import time

# Numpy
import numpy as np

# GDAL
import gdal
import ogr
import osgeo.osr
import gdalconst

# Opencv
import cv2

# Project
import Global
import ImageCommon
import ImageTools
import ImageMetadataExtractor

# Geo Image Class
# ###############################################################################

class GeoImage:

    def __init__(self, filename=""):
        self._dataset = None
        self.array = None
        self.geoExtent = None
        self.metadata = None

        self._pix2geo = None

        if len(filename) > 0:
            self.open(filename)


    def open(self, filename):
        """
            Method to load image from filename
            - self.array is a numpy array of shape (height,width,nbBands)
            - self.geoExtent is a numpy array of 4 points (long, lat) : [[left,top], [right,top], [right,bottom], [left,bottom]]
            - self.metadata is an array of image metadata
        """
        assert os.path.exists(filename), Global.logPrint("Filename is not found",'error')

        self._dataset = gdal.Open(filename, gdalconst.GA_ReadOnly)
        assert self._dataset is not None, Global.logPrint("Failed to open the file:" + filename, 'error')

        image = self._dataset.ReadAsArray()
        assert image is not None, Global.logPrint("Failed to read data of the file : " + filename,'error')

        # gdal output array is of shape (nbBands, height, width) or (height, width)
        # reshape to (height, width, nbBands)
        if (len(image.shape) == 2):
            self.array = image.reshape((image.shape[0],image.shape[1],1))
        else:
            self.array = convertShape2(image)

        # store as float32 array (instead of float64)
        self.array = self.array.astype(np.float32)

        if self._setupGeoTransformers():
            # get geo extent of the image:
            self.geoExtent = self._computeGeoExtent()

        # get metadata
        self.metadata = self._fetchMetadata()

    def getImageResolutionInDegrees(self):
        assert self._dataset is not None, Global.logPrint("Dataset is None",'error')
        geotransform = self._dataset.GetGeoTransform()
        return abs(geotransform[1]), abs(geotransform[5])

    def showMetadata(self):
            assert self.metadata is not None, Global.logPrint("Metadata is None",'error')
            print "---- Metadata : ----"
            for name in self.metadata:
                    print name, " : ", self.metadata[name]

    def fetchValues(self, keys, firstOccurence=True):
        assert self.metadata is not None, Global.logPrint("Metadata is None",'error')
        values={}
        for fieldName in self.metadata:
            for key in keys:
                if key.lower() in fieldName.lower():
                    values[key]=self.metadata[fieldName]
                    if firstOccurence:
                        return values;
        return values

    def transform(self, points, option="pix2geo"):
        """
            Method to transform points a) from geo to pixels, b) from pixels to geo
            Option can take values : "pix2geo" and "geo2pix"
            points should be a numpy array of type [[x1,y1],[x2,y2],...]

            Return numpy array of transformed points.
            In case of 'geo2pix' output value can be [-1, -1] which means point is not in the image

        """
        assert self._pix2geo is not None, Global.logPrint("Geo transformer is None",'error')
        _assertNPPoints(points)

        if option is "pix2geo":
            out = np.zeros((len(points),2))
            for count, pt in enumerate(points):
                g = self._pix2geo.TransformPoint(False, pt[0], pt[1], 0.0)
                out[count, 0] = g[1][0]
                out[count, 1] = g[1][1]
            return out
        elif option is "geo2pix":
            out = np.zeros((len(points),2), dtype=np.int16)-1
            f = lambda x : abs(round(x))
            w = self.array.shape[1]
            h = self.array.shape[0]
            for count, pt in enumerate(points):
                g = self._pix2geo.TransformPoint(True, pt[0], pt[1], 0.0)
                x = f(g[1][0])
                y = f(g[1][1])
                if x >= 0 and x < w and y >= 0 and y < h:
                    out[count, 0] = x
                    out[count, 1] = y

            return out
        else:
            return None


    def _fetchMetadata(self):
        assert self._dataset is not None, Global.logPrint("Dataset is None",'error')
        # get global metadata:
        metadata=self._dataset.GetMetadata()
        # get geolocation metadata:
        geoloc = self._dataset.GetMetadata("GEOLOCATION")
        if geoloc:
            metadata = dict(metadata.items() + geoloc.items())

        # get metadata from each band:
        nbBands=self._dataset.RasterCount
        for i in range(1,nbBands+1):
            band = self._dataset.GetRasterBand(i)
            bandMetadata = band.GetMetadata()
            metadata = dict(metadata.items() + bandMetadata.items())
        return metadata

    def _setupGeoTransformers(self):
        assert self._dataset is not None, Global.logPrint("Dataset is None",'error')

        if self._pix2geo is not None:
            return False

        # Init pixel to geo transformer :
        srs = osgeo.osr.SpatialReference()
        srs.ImportFromEPSG( 4326 )
        dstSRSWkt = srs.ExportToWkt()
        options = [ 'DST_SRS=' + dstSRSWkt ]


        transformer = gdal.Transformer(self._dataset, None, options)
        if transformer.this is None:
            Global.logPrint("No geo transformer found")
            return False

        self._pix2geo = transformer

        return True

    def _computeGeoExtent(self):
        assert self._dataset is not None, Global.logPrint("Dataset is None",'error')

        if self._pix2geo is None:
            return None

        gpts = []

        # transform 4 image corners
        w = self._dataset.RasterXSize
        h = self._dataset.RasterYSize
        pts = np.array([[0,0], [w-1,0], [w-1,h-1],[0,h-1]])

        return self.transform(pts, "pix2geo")

def convertShape2(image):
    """
    Method to convert image shape from (nbBands,h,w) -> (h,w,nbBands)
    """
    ImageCommon.assertImage(image)
    if (len(image.shape) > 2):
        out = np.zeros((image.shape[1], image.shape[2],image.shape[0]))
        for i in range(image.shape[0]):
            out[:,:,i] = image[i,:,:]
        image = out
    return image

def pointsToEnvelope(points):
    """
    Method to convert array of points [[x1,y1],[x2,y2],...] into
    (minX,maxX,minY,maxY)
    """
    _assertNPPoints(points)
    return (points[:,0].min(), points[:,0].max(), points[:,1].min(), points[:,1].max())


def _assertNPPoints(points):
    assert isinstance(points, np.ndarray), Global.logPrint("Input should be a Numpy array",'error')
    assert len(points.shape) == 2 and points.shape[1] == 2, Global.logPrint("Points should be a Numpy array of shape : (nbPts,2)",'error')



# Tests
# ###############################################################################

if __name__ == '__main__':

    filename = "C:\\VFomin_folder\\PISE_project\\IRT\Data\\Projets_TPZ\\Gabon\\IMG\\WGS84_AOI_ASA_WSM_1PTDPA20090312_091022_000000732077_00136_36763_0410.N1.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\Images\\COSMO-SkyMed\\_DAX\\15F00334-4\\CSKS2_SCS_B_HI_18_HH_RD_SF_20150228174742_20150228174750.SBI.tif"

    image = GeoImage(filename)

    print image.geoExtent
#    print image.getImageResolutionInDegrees()
##    print image.showMetadata()
#    print image.fetchValues(ImageMetadataExtractor.ImageMetadataExtractor.getAcquisitionModeKeys())

    # check transformers :
    print image.transform(image.geoExtent, "geo2pix")
    print image.array.shape
    print image.transform(np.array([[0, 0], [5594, 0], [5594, 6394], [0, 6394]]), "pix2geo")

    pts = image.transform(10.0+image.geoExtent, "geo2pix")
    pts[1,:] = [1,2]
    pts[3,:] = [3,4]

##    # display image
##    ImageTools.displayImage(image.array, True, "Original")
##    cv2.destroyAllWindows()
