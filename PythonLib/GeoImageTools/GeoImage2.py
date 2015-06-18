#-------------------------------------------------------------------------------
# Name:        Geo Image 2
# Purpose:      Class structure for 'tiled' reading of geo image files
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
        # GeoImage.array ( numpy.array of shape (height,width,nbBands) ) represents image data
        GeoImage.shape is an array (height,width,nbBands) and represents image data properties
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
        self.shape = None
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
        self._dataset = gdal.Open(filename, gdalconst.GA_ReadOnly)
        assert self._dataset is not None, Global.logPrint("Failed to open the file:" + filename, 'error')

        self.shape = (self._dataset.RasterYSize, self._dataset.RasterXSize, self._dataset.RasterCount)

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
            w = self.shape[1]
            h = self.shape[0]
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


    def getData(self, srcRect=None, dstWidth=None, dstHeight=None):
        """
        Method to read data from image
        srcRect is source extent in pixels : [x,y,w,h] where (x,y) is top-left corner. Can be None and whole image extent is used.
        dstWidth is the output array width. Can be None and srcRect[2] (width) is used.
        dstHeight is the output array heigth. Can be None and srcRect[3] (height) is used.
        Returns a numpy array
        """
        srcReqExtent=[]
        srcExtent=[]
        if srcRect is None:
            srcReqExtent = [0,0,self.shape[1],self.shape[0]]
            srcExtent = srcReqExtent
        else:
            srcReqExtent = intersection(srcRect, [0,0,self.shape[1],self.shape[0]])
            srcExtent = srcRect

        if srcReqExtent is None:
            return None


        dstExtent = []
        if dstWidth is None and dstHeight is None:
            dstExtent = [srcExtent[2], srcExtent[3]]
        elif dstHeight is None:
            h = int(dstWidth * srcExtent[3] * 1.0 / srcExtent[2])
            dstExtent = [dstWidth, h]
        elif dstWidth is None:
            w = int(dstHeight * srcExtent[2] * 1.0 / srcExtent[3])
            dstExtent = [w, dstHeight]
        else:
            dstExtent= [dstWidth, dstHeight]

        scaleX = dstExtent[0] * 1.0 / srcExtent[2]
        scaleY = dstExtent[1] * 1.0 / srcExtent[3]
        reqScaledW = int( min( np.ceil( scaleX * srcReqExtent[2] ), dstExtent[0] ) )
        reqScaledH = int( min( np.ceil( scaleY * srcReqExtent[3] ), dstExtent[1] ) )

        r = [int( np.floor( scaleX * (srcReqExtent[0] - srcExtent[0]) ) ),
             int( np.floor( scaleY * (srcReqExtent[1] - srcExtent[1]) ) ),
             reqScaledW,
             reqScaledH ]


        nbBands = self.shape[2]
        if gdal.DataTypeIsComplex(self._dataset.GetRasterBand(1).DataType):
            datatype = np.complex64
        else:
            datatype = np.float32

        out = np.zeros((dstExtent[1],dstExtent[0],nbBands), dtype=datatype)

        for i in range(nbBands):
            band = self._dataset.GetRasterBand(i+1)
            data = band.ReadAsArray(srcReqExtent[0], srcReqExtent[1], srcReqExtent[2], srcReqExtent[3], r[2], r[3])
            out[r[1]:r[1]+r[3], r[0]:r[0]+r[2], i] = data[:,:]

        return out



def intersection(r1, r2):
    """
    Helper method to obtain intersection of two rectangles
    r1 = [x1,y1,w1,h1]
    r2 = [x2,y2,w2,h2]
    """
    assert len(r1) == 4 and len(r2) == 4, Global.logPrint("Rectangles should be defined as [x,y,w,h]", 'error')

    rOut = [0,0,0,0]
    rOut[0] = max(r1[0], r2[0])
    rOut[2] = min(r1[0]+r1[2]-1, r2[0]+r2[2]-1) - rOut[0]
    rOut[1] = max(r1[1], r2[1])
    rOut[3] = min(r1[1]+r1[3]-1, r2[1]+r2[3]-1) - rOut[1]

    if rOut[2] <= 0 or rOut[3] <= 0:
        return None
    return rOut


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



##    r1 = [0,0,5,5]
##    r2 = [10,10,5,5]
##    r3 = [2,2,5,5]
##    r4 = [-2,-2,5,5]
##    r5 = [-2, 2,5,5]
##    r6 = [2,-2,5,5]
##
##    print intersection(r1,r2), intersection(r2,r1)
##    print intersection(r1,r3), intersection(r3,r1)
##    print intersection(r3,r2), intersection(r2,r3)
##    print intersection(r1,r4), intersection(r4,r1)
##    print intersection(r3,r4), intersection(r4,r3)
##    print intersection(r1,r5), intersection(r5,r1)
##    print intersection(r1,r6), intersection(r6,r1)
##    exit()

##    filename = "C:\\VFomin_folder\\PISE_project\\IRT\Data\\Projets_TPZ\\Gabon\\IMG\\WGS84_AOI_ASA_WSM_1PTDPA20090312_091022_000000732077_00136_36763_0410.N1.tif"
##    filename = "C:/temp/Stack_2013-09-30_2014-02-01.tif"
##    filename= "C:\\VFomin_folder\\PISE_project\\Images\\GEOEYE\\Carterra\\po_3800868_0000010000\\po_3800868_rgb_0000010000.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\Images\\COSMO-SkyMed\\_DAX\\15F00334-4\\CSKS2_SCS_B_HI_18_HH_RD_SF_20150228174742_20150228174750.SBI.tif"

    # TEST CSK RAW
##    filename= "HDF5:\"C:\\VFomin_folder\\PISE_project\\Images\\COSMO-SkyMed\\_DAX\\15F00334-6\\CSKS1_SCS_B_HI_0A_HH_RD_SF_20150304181147_20150304181154.h5\"://S01/SBI"
    filename= "HDF5:\"C:\\Users\\vfomin\\Downloads\\CSK_RAW_Hawaii\\raw\\CSKS2_RAW_B_HI_10_HH_RD_SF_20140105040332_20140105040340.h5\"://S01/QLK"

    image = GeoImage(filename)

    print image.shape
    print image.geoExtent
#    print image.getImageResolutionInDegrees()
##    print image.showMetadata()
#    print image.fetchValues(ImageMetadataExtractor.ImageMetadataExtractor.getAcquisitionModeKeys())

    # check transformers :
##    print image.transform(image.geoExtent, "geo2pix")
##    print image.shape
##    print image.transform(np.array([[0, 0], [5594, 0], [5594, 6394], [0, 6394]]), "pix2geo")
##
##    pts = image.transform(10.0+image.geoExtent, "geo2pix")
##    pts[1,:] = [1,2]
##    pts[3,:] = [3,4]

    exit()

    # display image
##    data = image.getData(None, 512)
    data = image.getData([2000,2200,2000,2010], 512)
##    data2 = np.zeros((data.shape[0],data.shape[1], 3), data.dtype)
##    data2[:,:,0] = data[:,:,0]
##    data2[:,:,1] = data[:,:,1]
##    data2[:,:,2] = data[:,:,1]
    ImageTools.displayImage(data, True, "Original")
    cv2.destroyAllWindows()

