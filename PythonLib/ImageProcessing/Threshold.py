
# Python
import random as rnd
import time

# Numpy
import numpy as np

# Scipy
import scipy.optimize as sc

# Matplotlib
import matplotlib.pyplot as plt

# Opencv
import cv2

# Project
import GeoImageTools.Global as Global
import GeoImageTools.ImageTools as ImageTools
import GeoImageTools.ImageCommon as ImageCommon



def nclasses(image, n, uMin=None, uMax=None):
    """
    Thresholds image into n color equivalent classes starting from a min value until a max value.
    Parameters uMin, uMax allows to specify this min/max values, otherwise real image min/max values
    are used.
    """
    ImageCommon.assertIsNPArray(image)
    mm = image.min() if uMin is None else uMin
    MM = image.max() if uMax is None else uMax
    step = (MM - mm) *1.0 / n
    x = np.arange(mm,MM+1,step)
    outImage = np.zeros(image.shape, dtype=np.float32)
    for i in range(n):
        tImage = np.zeros(image.shape)
        tImage[ image >= x[i] ] = i+1;
        tImage[ image > x[i+1] ] = 0;
        outImage += tImage

    return outImage




def thresholdModule(image):
    ImageCommon.assertOneBand(image)

    mm = image.min()
    MM = image.max()

    def update(dummy=None):
        t = cv2.getTrackbarPos('threshold', name) * 0.01 * (MM - mm)
        print t
        tImage = image.copy()
        tImage[tImage > t] = 1.0
        tImage[tImage < t] = 0.0
        ImageTools.displayImage(tImage, True, name, False)

    name = "Threshold Module"
    update()
    cv2.createTrackbar('threshold', name, 50, 100, update)

    while True:
        ch = 0xFF & cv2.waitKey()
        if ch == 27:
            break




if __name__ == '__main__':


    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_10.tif"

    image = ImageTools.loadImage(filename)
    if image is None:
        Global.logPrint("Failed to load image", 'error')
        exit()
    nbBands = 1 if len(image.shape) == 2 else image.shape[2]
    Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))

    ImageTools.displayImage(image, True, "Original")




    # Test n-classes
    i2 = nclasses(image, 5, 10, 200)
    ImageTools.displayImage(i2, True, "nclasses")


    # ----------------
    cv2.destroyAllWindows()