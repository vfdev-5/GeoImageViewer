
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



def gaussian(x, mu, sig, a0=None):
    a = 1.0/(np.sqrt(2*np.pi) * sig);
    if a0 is not None:
        a = a0
    return a * np.exp(-(x - mu)**2 / (2 * sig**2))

def freqFilter(image, mask):
    """
    Method to filter image using fft mask
    return filtered image
    """
    ImageCommon.assertOneBand(image)
    ImageCommon.assertOneBand(mask)
    assert image.shape[0] == mask.shape[0] and image.shape[1] == mask.shape[1], Global.logPrint('Image and mask should have same size', level='error')

    dftImage = ImageTools.computeFFT(image)

    filteredDftImage = dftImage
    for i in range(2):
        filteredDftImage[:,:,i] = filteredDftImage[:,:,i] * mask

    # inverse transform dft
    invFilteredDftImage = cv2.idft(filteredDftImage, flags=cv2.DFT_COMPLEX_OUTPUT | cv2.DFT_SCALE)
    filteredImage = cv2.magnitude(invFilteredDftImage[:,:,0],invFilteredDftImage[:,:,1])
    return filteredImage


def gaussianLowPassFreqFilter(image, sigmax=3.0, sigmay=3.0):
    """
    Method to filter image using its fft and low-pass gaussian filter on frequencies
    parameters sigmax, sigmay define gaussian filter and low-pass band
    return filtered image
    """
    ImageCommon.assertOneBand(image)

    # Generate mask
    mask = np.zeros((image.shape[0], image.shape[1]))
    # Clear all frequencies except those in a box
    h = mask.shape[0]
    w = mask.shape[1]
    # as a 2D gauss curve
    szH = int(6*sigmax*2*np.pi*np.sqrt(sigmax))
    szW = int(6*sigmay*2*np.pi*np.sqrt(sigmay))
    for i in range(2*szW):
        x = i - szW
        indx = w/2-szW + i
        indx = 0 if indx < 0 else w-1 if indx >= w else indx
        for j in range(2*szH):
            y = j - szH
            indy = h/2-szH + j
            indy = 0 if indy < 0 else h-1 if indy >= h else indy
            mask[indy, indx] = np.exp(-(x**2 + y**2)/(4.0 * sigmax**2 * sigmay**2))

    filteredImage = freqFilter(image, mask)
    return filteredImage



def enhanceContrast(image, low=1.5, high=97.5):
    """
    Method to enhance image contrast using quantiles
    """
    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image, 1000)

    nbBands = 1 if len(image.shape) == 2 else image.shape[2]
    out = image.copy()
    for i in range(nbBands):

        bandData = out if nbBands == 1 else out[:,:,bandIndex]
        nhist = bandHists[i,:]
        minValue = minValues[i]
        maxValue = maxValues[i]
            # Compute quantile values to reject near-zero histogram values
        [qmin, qmax] = ImageTools.computeQuantiles(nhist, low * 0.01, high * 0.01)
##        print qmin, qmax
        minValue = minValues[0] + qmin * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
        maxValue = minValues[0] + qmax * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
            # Theshold
        bandData[bandData < minValue] = minValue
        bandData[bandData > maxValue] = maxValue

    return out


def sieve2(image, size):
    """
    Filter removes small objects of 'size' from binary image
    Input image should be a 1D numpy array of type np.uint8

    Idea : use Opencv findContours

    """
    ImageCommon.assertOneBand(image)
    assert image.dtype == np.uint8, Global.logPrint("Input should be a Numpy array of type np.uint8",'error')

    sqLimit = size**2
    linLimit = size*4

    outImage = image.copy()
    contours, hierarchy = cv2.findContours(image.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    if len(hierarchy) > 0:
            hierarchy = hierarchy[0]
            index = 0
            while (index >= 0):
                contour = contours[index]
                p = cv2.arcLength(contour, True)
                s = cv2.contourArea(contour)
                r = cv2.boundingRect(contour)
##                print "Contour perimeter : ", p
##                print "Contour area : ", s
##                print "Contour bbox : ", r
                if s <= sqLimit and p <= linLimit:
##                    print " > remove bbox", r
                    outImage[r[1]:r[1]+r[3],r[0]:r[0]+r[2]] = 0
                # choose next contour of the same hierarchy
                index = hierarchy[index][0]
    return outImage


def sieve(image, size):
    """
    Filter removes small objects of 'size' from binary image

    Idea :
    Loop on all pixels
    For a white pixel count number of connected white pixels
    if this number is smaller than size**2 than remove all these pixels
    """
    ImageCommon.assertOneBand(image)

    sqLimit = size**2
    linLimit = size*2

    h,w=image.shape[:2]
    pdImage = np.zeros((h+2,w+2),dtype=image.dtype)
    histImage = np.zeros((h+2,w+2),dtype=np.uint8)
    pdImage[1:h+1,1:w+1] = image[:,:,0] if len(image.shape) == 3 else image

    def connx(p):
        cnx = [(0,1),(0,-1),[1,0],[-1,0],[1,1],[-1,-1],[-1,1],[1,-1]]
        for c in cnx:
            yield (p[0]+c[0],p[1]+c[1])

    counter = 1
    indices = pdImage.nonzero()
    for pIndex in zip(indices[0],indices[1]):
##        print "Nonzero index", pIndex
        if histImage[pIndex] > 0:
            continue
        stack = [pIndex]
        i = 1
        remove=True
##        print "counter : ", counter
        histImage[pIndex] = counter
        while len(stack) > 0:
            p = stack.pop()
##            print "seed : ", p
            for c in connx(p):
##                print c
                if pdImage[c] > 0 and histImage[c] == 0:
##                    print "mark"
                    stack.append(c)
                    histImage[c] = counter
                    i+=1
            if i > sqLimit:
                remove=False

        # remove
        if remove:
            histImage[histImage == counter] = 0

##        print "counter ++"
        counter+=1

    return histImage[1:h+1,1:w+1]




if __name__ == '__main__':

    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_10.tif"

    image = ImageTools.loadImage(filename)
    if image is None:
        Global.logPrint("Failed to load image", 'error')
        exit()
    nbBands = 1 if len(image.shape) == 2 else image.shape[2]
    Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))

    ImageTools.displayImage(image, True, "Original", False)

    i2 = cv2.blur(image, (7,7))

    t = i2.min() + 0.010*(i2.max() - i2.min())
    i2[i2 < t] = 0.0
    i2[i2 > t] = 1.0
    i0 = -1.0*(i2 - 1.0)

    ImageTools.displayImage(i0, True, "Thresholded")



    # Synthetic images :
##    i0 = np.zeros((50,60),dtype=np.uint8)
##    i0[1:2,2:3] = 1
##    i0[10:12,22:23] = 1
##    i0[1:4,32:35] = 1
##    i0[21:32,17:19] = 1
##    i0[41:44,20:27] = 1
##    i0[11:14,52:55] = 1
##    i0[12:14,54:57] = 1

    i2 = sieve2(i0.astype(np.uint8), 5.0)

    ImageTools.displayImage(i0, True, "i0", False)
    ImageTools.displayImage(i2, True, "i2")

##    plt.subplot(121)
##    plt.imshow(i0, interpolation='none')
##    plt.subplot(122)
##    plt.imshow(i2, interpolation='none')
##    plt.show()

##    # Pre-processing :
##    #   FFT Gauss filtering :
##    filteredImage = ImageTools.norm(gaussianLowPassFreqFilter(image, 4.0, 4.0))
##
##    #   Enhance contrast :
##    filteredImage = enhanceContrast(filteredImage, 0.55, 95.0)
##
##    ImageTools.displayImage(filteredImage, True, "pre-filtered image")
####    ImageTools.displayHist(filteredImage, colors=['r'])

    cv2.destroyAllWindows()

