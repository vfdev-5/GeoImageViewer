# Basic Scripts for image IO and display

"""
Loader methods:
    loadImage() : loads image file and return an numpy array

Display methods:
    displayImage() : displays image array in a window
    displayHist()
    displayCumHist()
    displayFFT()

Computation methods:
    computeQuantiles()
    computeFFT()
    fftShift()
    computeNormHist()
    norm()
    downsample()

"""


# Python
import os
import time

# Numpy
import numpy as np

# Matplotlib
import matplotlib.pyplot as plt

# GDAL
import gdal
import ogr
import gdalconst

# Opencv
import cv2

# Project
import Global
import ImageCommon


# Loader methods
# ###############################################################################

def loadImage(filename):
    """
    Method to load image from filename
    Return imageArray
    - imageArray is a numpy array of shape (height,width,nbBands)

    """
    if not os.path.exists(filename):
        Global.logPrint("Filename is not found",'error')
        return None

    ds = gdal.Open(filename, gdalconst.GA_ReadOnly)
    if ds is None:
        Global.logPrint("Failed to open the file:" + filename, 'error')
        return None

    image = ds.ReadAsArray()
    assert image is not None, Global.logPrint("Failed to read data of the file : " + filename,'error')

    # gdal output array is of shape (nbBands, height, width) or (height, width)
    # reshape to (height, width, nbBands)
    if (len(image.shape) == 2):
        image = image.reshape((image.shape[0],image.shape[1],1))
    else:
        image = convertShape2(image)
    return image

# Display methods
# ###############################################################################

def displayImage(image0, showMinMax=True, name="image", waitKey=True, rescale=True):
    """
    Method to display image
    Input image should be of shape (height, width) or (height, width, nbBands)
    """
    ImageCommon.assertImage(image0)

    nbBands = 1 if len(image0.shape) == 2 else image0.shape[2]

    if showMinMax:
        Global.logPrint("Image \'" + name + "\' has size: " + str(image0.shape[0]) + ", " + str(image0.shape[1]) + " and nbBands= " + str(nbBands))


    # define mapping
    if nbBands == 1:
        mapping = [1,1,1]
        image = image0
    elif nbBands == 2:
        mapping = [1,1,1]
        # create abs band from Re and Im
        image = np.zeros((image0.shape[0], image0.shape[1]))
        image = cv2.magnitude(image0[:,:,0], image0[:,:,1])
        nbBands = 1
    elif nbBands >= 3:
        mapping = [3,2,1]
        image = image0

    # rescale if image is larger than a screen of 800 pixels
    if rescale:
        displayLimit = 800
        maxdim = max(image.shape[0], image.shape[1])
        if maxdim > displayLimit:
            Global.logPrint("displayImage : " + name + ", Image is rescaled")
            factor = maxdim * 1.0 / displayLimit
            image = cv2.resize(image, None, fx=1.0/factor,fy=1.0/factor)

    minValues=np.zeros((nbBands))
    maxValues=np.zeros((nbBands))
    outputImage = np.zeros((image.shape[0], image.shape[1], 3), dtype=np.uint8)

    for i in range(3):

        bandIndex = mapping[i]-1
        # compute min/max on the band
        bandData = image if nbBands == 1 else image[:,:,bandIndex]
        minValues[bandIndex] = bandData.min()
        maxValues[bandIndex] = bandData.max()
        if (maxValues[bandIndex] - minValues[bandIndex] > 1000):
            stdDev=bandData.std()
            mn=bandData.mean()
            nmin = mn - 3.0 * stdDev
            nmin = minValues[bandIndex] if nmin < minValues[bandIndex] else nmin
            nmax = mn + 3.0 * stdDev
            nmax = maxValues[bandIndex] if nmax > maxValues[bandIndex] else nmax
        else:
            nmin=minValues[bandIndex]
            nmax=maxValues[bandIndex]

        if showMinMax:
            Global.logPrint("Image \'" + name + "\', min/max : " + str(minValues[bandIndex]) + ", " + str(maxValues[bandIndex]) )
            Global.logPrint("Image \'" + name + "\', min/max using mean/std: " + str(nmin) + ", " + str(nmax) )

        bandData[bandData > nmax] = nmax
        bandData[bandData < nmin] = nmin
        outputImage[:,:,i] = (255.0 * (bandData - nmin) / (nmax - nmin)) if nmax > nmin else 0.0

    cv2.imshow(name, outputImage)

    if (waitKey):
        cv2.waitKey(0)


def displayHist(image, show=True, colors=['r','g','b','m','y'], uMinValues=None, uMaxValues=None):
    """
    Method to compute and display image histogram
    """

    [bandHists, minValues, maxValues] = computeNormHist(image, uMinValues=uMinValues, uMaxValues=uMaxValues)
    displayHist2(bandHists, minValues, maxValues, show, colors)

def displayCumHist(image, show=True, colors=['r','g','b','m','y']):
    """
    Method to compute and display image cumulative histogram
    """
    [bandHists, minValues, maxValues] = computeNormHist(image)
    cumBandHist = norm(np.cumsum(bandHists,1))
    displayHist2(cumBandHist, minValues, maxValues, show, colors)


def displayHist2(bandHists, minValues=None, maxValues=None, show=True, colors=['r','g','b','m','y']):
    """
    Method to display the histogram
    """

    ImageCommon.assertIsNPArray(bandHists)

    for bandIndex in range(bandHists.shape[0]):
        if minValues is None and maxValues is None:
            x = computeColorValues(0, bandHists.shape[1], bandHists.shape[1])
        else:
            x = computeColorValues(minValues[bandIndex], maxValues[bandIndex], bandHists.shape[1])
##            print "step", step, "len(x)", len(x)
##            print "maxValues[bandIndex]", maxValues[bandIndex], "minValues[bandIndex]", minValues[bandIndex]
##            print "x[0]", x[0], "x[-1]", x[-1]
        # display original histogram
        plt.plot(x, bandHists[bandIndex,:], colors[bandIndex] if bandIndex < len(colors) else 'b')

    if show:
        plt.show()


def displayFFT(image, name="image", waitKey=True):
    """
    Method to display fft magnitude of the input image
    Resulting fft image is fft shifted, e.g. zero frequencies are in the center

    Return fftImage 2-band numpy array

    """
    dftImage = computeFFT(image)
    displayImage(dftImage, True, "Abs FFT " + name, waitKey)


# Computation methods
# ###############################################################################

def convertShape(image):
    """
    Method to convert image shape from (h,w,nbBands) -> (nbBands,h,w)
    """
    ImageCommon.assertImage(image)
    if (len(image.shape) > 2):
        out = np.zeros((image.shape[2], image.shape[0],image.shape[1]))
        for i in range(image.shape[2]):
            out[i,:,:] = image[:,:,i]
        image = out
    return image

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

def computeQuantiles(nparray, lower=0.025, upper=0.975):
    """
    Method to compute quantile min/max indices when
    cumulative function equals lower and upper values

    cumFunc[qmin] == lower
    cumFunc[qmax] == upper

    return qmin, qmax

    """
    ImageCommon.assert1D(nparray)
    qmin = 0
    qmax = nparray.shape[0]-1

    cumArray = norm(np.cumsum(nparray))

    for i in range(cumArray.shape[0]):
        if cumArray[qmin] < lower:
            qmin = i
        else:
            break

    for i in range(cumArray.shape[0]):
        if cumArray[qmax] > upper:
            qmax = cumArray.shape[0]-1 - i
        else:
            break

    return [qmin, qmax]


def computeNormHist(image, size=1000, uMinValues=None, uMaxValues=None):

    ImageCommon.assertIsNPArray(image)

    nbBands = 1 if len(image.shape) == 2 else image.shape[2]

    minValues=np.zeros((nbBands))
    maxValues=np.zeros((nbBands))
    bandHistograms = np.zeros((nbBands, size))

    for i in range(nbBands):

        bandData = image if nbBands == 1 else image[:,:,i]

        if bandData.dtype is not np.float32:
            bandData32f = bandData.astype(np.float32)
        else:
            bandData32f = bandData

        if uMinValues is not None:
            mm = uMinValues[i]
        else:
            mm = bandData32f.min()

        if uMaxValues is not None:
            MM = uMaxValues[i]
        else:
            MM = bandData32f.max()

        minValues[i] = mm
        maxValues[i] = MM

        if int(mm) == int(MM):
            if int(mm) == 0:
                MM = 1
            elif int(mm) == 1:
                mm = 0
            else:
                mm = int(MM - 0.5*size)
                MM = int(MM + 0.5*size)

        r = np.array([mm, MM], dtype=np.int)

        hist = cv2.calcHist([bandData32f], channels=[0], mask=None, histSize=[size], ranges=r)

        mm = hist.min()
        MM = hist.max()

        bandHistograms[i,:] = ( hist[:,0] - mm ) / (MM - mm) if MM > mm else 0.0

    return [bandHistograms, minValues, maxValues]

def norm(a):
    mm = a.min()
    MM = a.max()
    return (a-mm)/(MM-mm)


def computeLocalMinMax(nparray0, windowSize=None):
    outMin = []
    outMax = []

    # smooth data
    w=np.ones(4)
    nparray = np.convolve(w/w.sum(),nparray0,mode='valid')

    size = windowSize
    if windowSize is None:
        size = nparray.shape[0]/10

    nbWindows = int(np.ceil(nparray.shape[0] * 1.0/size))

    for i in range(nbWindows):
        offset = i*size
        sz = size if offset + size < nparray.shape[0] else nparray.shape[0] - offset
        wData=nparray[offset:offset+sz]

        if abs(wData.min()-wData.max()) < 0.01:
            continue

##        plt.plot(wData, 'b-')

        locmin = wData.argmin()
        locmax = wData.argmax()

        if locmin != 0 and locmin != sz-1:
            outMin.append([offset + locmin, wData[locmin]])
##            plt.plot(locmin, wData[locmin], 'r*')

        if locmax != 0 and locmax != sz-1:
            outMax.append([offset + locmax, wData[locmax]])
##            plt.plot(locmax, wData[locmax], 'b*')

##        plt.show()

    return outMin, outMax


def equalizeHist(image):
    """
    Method to equalize histogram of the image
    return image with equalized histogram
    """
    ImageCommon.assertOneBand(image)

    [bandHist, minValues, maxValues] = computeNormHist(image)

    cumBandHist = norm(np.cumsum(bandHist))

    step = (maxValues[0] - minValues[0]) * 1.0 / (bandHist.shape[1])

    f = np.vectorize(lambda x: cumBandHist[int(x)] if x < 500.0 else 1)
    output = f((image - minValues[0]) * 1.0 / step)
    return output

def computeColorValues(minValue, maxValue, length):
    step = (maxValue-minValue)*1.0/length
    x = np.arange(minValue, maxValue, step)
    if len(x) > length:
        x=x[0:length]
    return x



def downsample(image, wsize=(10,10), func=lambda array : 1.0/len(array.ravel()) * sum(array.ravel()) ):

    ImageCommon.assertOneBand(image)

    width = int(np.ceil(image.shape[0] * 1.0/wsize[0]))
    height = int(np.ceil(image.shape[1] * 1.0/wsize[1]))

    out = np.zeros((width, height), dtype=np.float32)

    for i in range(width):
        xoffset = i*wsize[0]
        sx = wsize[0] if xoffset + wsize[0] < image.shape[0] else image.shape[0] - xoffset
        for j in range(height):
            yoffset = j*wsize[1]
            sy = wsize[1] if yoffset + wsize[1] < image.shape[1] else image.shape[1] - yoffset
            d = image[xoffset:xoffset+sx,yoffset:yoffset+sy]
            out[i][j] = func(d)

    return out;


def fftShift(fftImage):

    ImageCommon.assertTwoBands(fftImage)

    nbBands = fftImage.shape[2]
    w = fftImage.shape[1]
    h = fftImage.shape[0]

    r1 = fftImage[0:h/2,0:w/2].copy()
    r2 = fftImage[0:h/2,w/2:].copy()
    r3 = fftImage[h/2:,:w/2].copy()
    r4 = fftImage[h/2:,w/2:].copy()

    fftImage[:r4.shape[0],:r4.shape[1]] = r4[:,:]
    fftImage[:r3.shape[0],r4.shape[1]:] = r3[:,:]
    fftImage[r4.shape[0]:,:r2.shape[1]] = r2[:,:]
    fftImage[r4.shape[0]:,r2.shape[1]:] = r1[:,:]

    return fftImage

def computeFFT(image):
    """
    Method to compute fft output of the image
    Resulting fft image is fft shifted, e.g. zero frequencies are in the center
    """
    ImageCommon.assertOneBand(image)
    return fftShift(cv2.dft(image, flags=cv2.DFT_COMPLEX_OUTPUT))



if __name__ == "__main__":

    filenames = []
    filenames.append("C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\filtered\\img5.tif")
    filenames.append("C:\\VFomin_folder\\PISE_project\\Images\\DigiatalGlobe30cm\\CHN_Shanghai_Pudong_Dec18_2014_WV3_30cm_2.jpg")

    # # # TESTS # # #
    for filename in filenames:

        image = loadImage(filename)
        if image is None:
            Global.logPrint("Failed to load image", 'error')
            exit()
        nbBands = image.shape[2]
        Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))

        # # - Downsample image
        if nbBands == 1:
            image = downsample(image)

        # # - Display image
        displayImage(image, True, 'Original', False)

        # # - Display histogram
        displayHist(image, False)

        # # - Display cumulative histogram
        displayCumHist(image)

        # # - Equalize the histogram
        if nbBands == 1:
            image = equalizeHist(image)
            displayImage(image, True, 'equalizeHist')


    cv2.destroyAllWindows()
    exit()
    # # # END OF TESTS # # #


    [bandHists, minValues, maxValues] = computeNormHist(image)

    # compute max loc :
    step = (maxValues[0]-minValues[0])/(bandHists.shape[0])
    maxloc = minValues[0] + bandHists[:,0].argmax()*step
    Global.logPrint("Max loc: " + str(maxloc))

    # display original histogram
    x = np.arange(minValues[0],maxValues[0], step)
    plt.plot(x, bandHists[:,0],'b')

    # display original cumulative histogram
##    cumBandHist = norm(np.cumsum(bandHists[:,0]))
##    plt.plot(x, cumBandHist,'r')


    # extract max peak
##    hist2 = bandHists[:,0]
##    hist2[hist2 < 0.5] = 0.0
##    plt.plot(x, hist2, 'r')


    # display histogram * dist to max peak
##    hist2 = norm(x*bandHists[:,0])
##    plt.plot(x, hist2, 'r')

    # display inverted histogram
##    tol = 0.0005
##    hist2 = bandHists[:,0]
##    hist2[hist2 < tol] = tol
##    hist2 = tol/hist2;
##    plt.plot(x, hist2, 'g')

    s = sum(bandHists[:,0])
    mn = sum(x*bandHists[:,0])/s
    sig = np.sqrt(sum(x[:]*x[:]*bandHists[:,0])/s - mn*mn)
    print "Sum of hist: ", s
    print "Mean of hist: ", mn
    print "Sigma of hist: ", sig


    # extract max peak
##    indexL = int((mn - sig - minValues[0])/step)
##    indexR = int((mn + sig - minValues[0])/step)
##    hist2 = bandHists[:,0]
##    hist2[indexL:indexR] = 0.0
##    plt.plot(x, hist2, 'r')

    plt.show()

    cv2.destroyAllWindows()
    exit()



    locMinima, locMaxima = computeLocalMinMax(bandHists[:,0], 12)

    # display original histogram
    x = np.arange(minValues[0],maxValues[0], step)
    plt.plot(x, bandHists[:,0],'b')

    print locMinima
    print locMaxima

    xLocMinima = [minValues[0] + step*x[0] for x in locMinima]
    yLocMinima = [y[1] for y in locMinima]

    xLocMaxima = [minValues[0] + step*x[0] for x in locMaxima]
    yLocMaxima = [y[1] for y in locMaxima]


    plt.plot(xLocMinima, yLocMinima,'r*')
    plt.plot(xLocMaxima, yLocMaxima,'g.')

    plt.show()


    # threshold image :
    # test if two peaks found and one local min
    if len(locMinima) == 1 and len(locMaxima) == 2:
        print "Threshold image"
        fImage = image
        fImage[ fImage <= xLocMinima[0] ] = 255
        fImage[ fImage > xLocMinima[0] ] = 0
        displayImage(fImage, True, "threshold", False)

        kernel = np.ones((5,5), np.uint8)
        mfImage = cv2.dilate(fImage, kernel)
        displayImage(mfImage, True, "threshold+dilate")

    else:
        print "Can not threshold image. Failed to find two peaks of the histogram"



    cv2.destroyAllWindows()


