#-------------------------------------------------------------------------------
# Name:        Geo Image Tools
# Purpose:      Methods for reading, extracting, writing geo image files
#
# Author:      vfomin
#
# Created:     08/04/2015
# Copyright:   (c) vfomin 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------

"""

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
import GeoImageWriter
import GeoImage


# Geo Image Tools
# ##############################################################################

def _assertGeoImage(geoImage):
    assert isinstance(geoImage, GeoImage.GeoImage), Global.logPrint("Input should be a GeoImage instance",'error')


def displayGeoImage(geoImage, windowName="GeoImage", showMetadata=False):
    """
    Method to display image data and info
    """
    print "Geo Extent : ", geoImage.geoExtent
    if showMetadata:
        print "Metadata"
        print geoImage.metadata
    ImageTools.displayImage(geoImage.array, True, windowName)


def chooseRoi(geoImage):
    """
    Method to choose interactively image roi
    Return a GeoImage
    """
    _assertGeoImage(geoImage)
    [offset, size] = ImageTools.chooseRoi(geoImage.array, False)
    if offset is not None:
        return extractRoi(geoImage, [offset[0], offset[1], size[0], size[1]])
    return None


def extractRoi(inputGeoImage, roi):
    """
    Method to extract image roi from a GeoImage instance
    - inputGeoImage should be a GeoImage instance
    - roi should be an array list of type [offsetX, offsetY, width, height] in pixels

    Return a GeoImage (which has no gdal dataset associated)
    """
    _assertGeoImage(inputGeoImage)
    assert len(roi) == 4, Global.logPrint("Roi should be an array list of type [offsetX, offsetY, width, height] in pixels",'error')

    h = inputGeoImage.array.shape[0]
    w = inputGeoImage.array.shape[1]

    assert roi[0] >= 0 and roi[2] >= 0 and roi[0] + roi[2] < w and \
            roi[1] >= 0 and roi[3] >= 0 and roi[1] + roi[3] < h, \
            Global.logPrint("Roi rectangle in pixels should be in the image pixel extent rectangle",'error')

    out = GeoImage.GeoImage()

    out.metadata = inputGeoImage.metadata
    out.array = inputGeoImage.array[ roi[1]:roi[1]+roi[3], roi[0]:roi[0]+roi[2] ].copy()
    out.geoExtent = inputGeoImage.transform(np.array([[roi[0], roi[1]], [roi[0]+roi[2], roi[1]], \
                                 [roi[0]+roi[2], roi[1]+roi[3]], [roi[0], roi[1]+roi[3]]]), "pix2geo")

    return out

def saveImage(geoImage, outputFilename):
    """
    Method to save geo image into a TIF file
    GeoImage should be have an associated gdal dataset
    """
    _assertGeoImage(geoImage)
    assert geoImage._dataset is None, Global.logPrint("GeoImage should be have an associated gdal dataset",'error')
    if os.path.exists(outputFilename):
        os.remove(outputFilename)

    w = geoImage.array.shape[1]
    h = geoImage.array.shape[0]
    step=(geoImage.geoExtent[2] - geoImage.geoExtent[0])/np.array([w,h])
    originAndStep = [geoImage.geoExtent[0], step]
    GeoImageWriter.writeDataInGeoImage(outputFilename,geoImage.array,originAndStep,geoImage.metadata)



# Tests
# ##############################################################################

if __name__ == '__main__':

    filename = "C:/temp/RGB_Stack_HI_2013-09-30_2014-02-01_pwr_geo_rgb.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\IRT\Data\\Projets_TPZ\\Gabon\\IMG\\WGS84_AOI_ASA_WSM_1PTDPA20090312_091022_000000732077_00136_36763_0410.N1.tif"
    geoimage = GeoImage.GeoImage(filename)

##    displayGeoImage(geoimage, "Original")
    roiGeoImage = chooseRoi(geoimage)

    geoimage = None
    displayGeoImage(roiGeoImage, "Roi")
##    roiGeoImage.array[:,:,2] = np.arctan2(roiGeoImage.array[:,:,0], roiGeoImage.array[:,:,1])
##    displayGeoImage(roiGeoImage, "Roi")

    outfile = "C:/temp/roi_test.tif"
    saveImage(roiGeoImage, outfile)
    roiGeoImage = None

    roiGeoImage2 = GeoImage.GeoImage(outfile)
    displayGeoImage(roiGeoImage2, "Roi2")


    # -----
    cv2.destroyAllWindows()

