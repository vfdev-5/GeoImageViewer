
# Python
import random as rnd

# Numpy
import numpy as np

# Scipy
import scipy.optimize as sc

# Matplotlib
import matplotlib.pyplot as plt

# Opencv
import cv2

# Project
import Global
import ImageTools


def _computeIterThreshold():
    pass


def enhanceLow(nparray, center, delta=50.0):
    mm = np.arctan(-center/delta)
    MM = np.arctan((nparray[-1]-center)/delta)
    out = np.arctan((nparray-center)/delta) - mm
    out = out / (MM - mm)
    return out

##def meanStdVec3(x, nparray):
##    ImageTools.assert1D(nparray)
##    ImageTools.assert1D(x)
##    assert nparray.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")
##
##    meanVec = np.zeros(x.shape)
##    stdVec = np.zeros(x.shape)
##
##    for index in range(1, x.shape[0]+1):
##
##        sL = sum(nparray[0:index])
##        meanVec[index-1] = sum(x[0:index]*nparray[0:index])/sL
##        stdVec[index-1] = sum(x[0:index]*x[0:index]*nparray[0:index])/sL - meanVec[index-1]*meanVec[index-1]
##
##    stdVec = np.sqrt(stdVec)
##    return meanVec, stdVec



def meanStdVec22(x, nparray):
    """
    Method to compute mean and std vectors

    meanVec[i] = sum(x[j] * nparray[j], {j=0,i}) / sum(nparray)
    stdVec[i] = sqrt(sum(x[j]**2 * nparray[j], {j=0,i}) / sum(nparray) - meanVec[i]**2)

    """
    ImageTools.assert1D(nparray)
    ImageTools.assert1D(x)
    assert nparray.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")

    meanVec = np.zeros(x.shape)
    stdVec = np.zeros(x.shape)

    s = sum(nparray[:])
    meanVec[0] = x[0]*nparray[0] / s
    stdVec[0] = x[0]**2 * nparray[0] / s - meanVec[0]
    for index in range(1, x.shape[0]):

        meanVec[index] = meanVec[index-1] + x[index]*nparray[index] / s
        stdVec[index] = stdVec[index-1] + (x[index] - 1) * x[index] *nparray[index] / s

    stdVec = np.sqrt(stdVec)
    return meanVec, stdVec


def meanStdVec2(x, nparray):
    ImageTools.assert1D(nparray)
    ImageTools.assert1D(x)
    assert nparray.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")

    meanVec = np.zeros(x.shape)
    stdVec = np.zeros(x.shape)

    sL = nparray[0]
    meanVec[0] = x[0]
    stdVec[0] = 0.0
    for index in range(1, x.shape[0]):

        meanVec[index] = meanVec[index-1]*sL + x[index]*nparray[index]
        stdVec[index] = (stdVec[index-1] + meanVec[index-1]**2)*sL + (x[index]**2)*nparray[index]

        sL += nparray[index]

        meanVec[index] = meanVec[index]/sL if sL > 0 else meanVec[index-1] + x[index] if meanVec[index] == 0 else 1e10
        stdVec[index] = stdVec[index]/sL if sL > 0 else 0.0 if stdVec[index] == 0 else 1e10
        stdVec[index] -= (meanVec[index]**2)

    stdVec = np.sqrt(stdVec)
    return meanVec, stdVec

def meanStdVec(image):
    ImageTools.assertOneBand(image)
    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image)

    step = (maxValues[0]-minValues[0])/(bandHists.shape[0])
    x = np.arange(minValues[0],maxValues[0], step)
    nparray = bandHists[:,0]
    meanVec, stdVec = meanStdVec22(x, nparray)
    return meanVec, stdVec, x


def grayLevelHistogram(image):

    """
    Method to automatically threshold image.
    The threshold is computed by minimizing the function
        eta(t) = sigma2(t) / totalSigma2

    where sigma2(t) = w0(t) * w1(t) * ( mean0(t) * mean1(t) )^2
    w0(t) = sum(hist(i),{i,0,t-1}) / sum(hist(i),{i,0,histSize-1})
    w1(t) = 1 - w0(t)

    mean0(t) = mean(t) / w0(t)
    mean1(t) = ( totalMean - mean(t) ) / (1 - w0(t) )
    mean(t) = sum( v(i) * hist(i),{i,0,t-1} ) / sum(hist(i),{i,0,histSize-1})

    """
    ImageTools.assertOneBand(image)

    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image)

    # compute total mean and sigma
    step = (maxValues[0]-minValues[0])/(bandHists.shape[0])
    x = np.arange(minValues[0],maxValues[0], step)
    s = sum(bandHists[:,0])
    totalMean = sum(x[:]*bandHists[:,0])/s
    totalSigma2 = sum(x[:]*x[:]*bandHists[:,0])/s - totalMean*totalMean
    print "Mean of hist: ", totalMean
    print "Sigma2 of hist: ", totalSigma2

    tMin=-1;
    etaMin=1e10;
    tMax=-1;
    etaMax=-1;

    w0 = 0
    mn = 0
    for index in range(2,bandHists.shape[0]-2):

        w0 += bandHists[index,0] / s
        mn += x[index]*bandHists[index,0] / s

        w1 = 1.0 - w0
        mean0 = mn / w0
        mean1 = ( totalMean - mn )/ w1
        sigma2 = w0 * w1 * (mean0 * mean1)**2
        eta = sigma2 / totalSigma2;
        if (eta < etaMin):
            print "index: ", index, ", etaMin -> ", eta
            etaMin = eta
            tMin = index
        elif (eta > etaMax):
            print "index: ", index, ", etaMax -> ", eta
            etaMax = eta
            tMax = index

    print "Eta min : ", etaMin
    print "Eta max : ", etaMax

    # Compute threshold value:
    t = minValues[0] + step * tMin
    print "Threshold value (min): ", t
    t2 = minValues[0] + step * tMax
    print "Threshold value (max): ", t2

    tImage = image.copy()
    tImage[ tImage <= t2 ] = 0
    tImage[ tImage > t2 ] = 255

    return tImage


def iterativeSelection(image):

    """
    Method to automatically threshold image.
    The threshold is computed iteratively.
    At k iteration it is :
    threshold(k) = 0.5 * (t0(k) + t1(k))
    t0(k) = sum( v(i) * hist(i),{i,0,threshold(k-1)-1} ) / sum(hist(i),{i,0,threshold(k-1)-1})
    t1(k) = sum( v(i) * hist(i),{i,threshold(k-1),histSize-1} ) / sum(hist(i),{i,threshold(k-1),histSize-1})

    """
    ImageTools.assertOneBand(image)

    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image)

##    print "Histogram size", bandHists.shape[0]
    x = np.arange(bandHists.shape[0])
    s = sum(bandHists[:,0])
    mn = sum(x*bandHists[:,0])/s
##    print "Mean index : ", mn
    t = int(mn);

    count=50
    while (count > 0):

##        print count, " : Threshold index= ", t

        sL = sum(bandHists[0:t,0])
        mnL = sum(x[0:t]*bandHists[0:t,0])/sL
        sR = sum(bandHists[t:,0])
        mnR = sum(x[t:]*bandHists[t:,0])/sR
##        print "Mean index Left/Right: ", mnL, mnR

        newT = int(0.5*(mnL + mnR))
##        print count, " : New threshold= ", newT

        if (t != newT):
            t=newT
        else:
            break;
        count-=1

    # Compute threshold value:
    step = (maxValues[0]-minValues[0])/(bandHists.shape[0])
    t = minValues[0] + step * t
    print "Threshold value : ", t
    tImage = image.copy()
    tImage[ tImage <= t ] = 0
    tImage[ tImage > t ] = 255

    return tImage


def kmeans(image):

    ImageTools.assertOneBand(image)

##    nparray = np.zeros((nparray0.shape[0], 2), dtype=np.float32)
##    nparray[:,0]=nparray0[:];

##    term_crit = (cv2.TERM_CRITERIA_EPS, 30, 0.1)
##    retval, bestLabels, centers = cv2.kmeans(nparray, 3, term_crit, 10, 0)
##    print "bestLabels", bestLabels
##    print "centers", centers

    pass


def gaussian(x, mu, sig, a0=None):
    a = 1.0/(np.sqrt(2*np.pi) * sig);
    if a0 is not None:
        a = a0
    return a * np.exp(-(x - mu)**2 / (2 * sig**2))


def inverse(value, table):
    """
    Binary search method to inverse function given by a table
    return index in the table
    """
    iMin = 0
    iMax = len(table) - 1

    if value <= table[iMin]:
        return iMin

    if value >= table[iMax]:
        return iMax

    limit=50
    tol = 1e-8
    index = (iMax - iMin) / 2
    while (limit > 0):

##        print "table[index]", table[index], "table[iMin]", table[iMin], "table[iMax]", table[iMax], "index", index, "iMin", iMin, "iMax", iMax, "value", value

        if abs(table[index] - value) < tol :
            return index

        if iMax - iMin < 2:
            if abs(table[iMin] - value) < abs(table[iMax] - value):
                return iMin
            else:
                return iMax

        if value < table[index]:
            iMax = index
        else:
            iMin = index

        index = iMin + (iMax - iMin) / 2
        limit-=1

    print "Index is not found"
    return -1


def invTransfSampling(x, nhist, n=1000):
    """
    Method to generate random variables (number of values = n)
    using the histogram as PDF

    return n randomly distributed values in range of x
    """
    ImageTools.assert1D(nhist)
    ImageTools.assert1D(x)
    assert nhist.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")

    cdf = ImageTools.norm(np.cumsum(nhist))
    out = np.zeros(n)
    count = 0
    while (count < n):
        u = rnd.random()
        # find inverse value of cdf for u
        y = inverse(u, cdf)
        if y < 0:
            continue
        out[count] = x[y]
        count+=1

    return out

def invTransfSampling2(x, nhist, n=1000):
    """
    Method to generate not random variables (number of values = n)
    using the histogram as PDF

    return n randomly distributed values in range of x
    """
    ImageTools.assert1D(nhist)
    ImageTools.assert1D(x)
    assert nhist.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")

    cdf = ImageTools.norm(np.cumsum(nhist))
    out = np.zeros(n)
    for i in range(n):
        u = i * 1.0 / n
        # find inverse value of cdf for u
        y = inverse(u, cdf)
        if y < 0:
            continue
        out[i] = x[y]

    return out


def estimateGMM(x, nhist):
    """
    Method to estimate GMM parameters from the normalized histogram

    1) Generate points from the normalized histogram
        pts
    2)

    """
    ImageTools.assert1D(nhist)
    ImageTools.assert1D(x)
    assert nhist.shape[0] == x.shape[0], Global.logPrint("Input arrays should have same length")

    # generate points from the histogram :
    points = invTransfSampling2(x, nhist, 1000)

    nbClusters=2

    em = cv2.EM(nbClusters)
    em.train(points)
    means = np.array(em.getMat('means')).ravel()
    covs = np.array(em.getMatVector('covs')).ravel()

##    print means, covs

    # remove gaussians if means are too close
    nmeans= []
    ncovs = []
    nbc = len(means)

    if nbc < 2:
        return means, covs
    elif nbc == 2:
        dist = np.abs(means[1] - means[0])
        sig = min(np.sqrt(covs[1]), np.sqrt(covs[0]))
        if dist < sig:
            nmeans.append( 0.5*(means[1] + means[0]) )
            ncovs.append( 0.5*(covs[1] + covs[0]) )
        else:
            return means, covs
        return nmeans, ncovs
    else:
        for i in range(1, nbc-1):

            dist01 = np.abs(means[i-1] - means[i])
            dist12 = np.abs(means[i+1] - means[i])
            sig01 = min(np.sqrt(covs[i]), np.sqrt(covs[i+1]))
            sig12 = min(np.sqrt(covs[i]), np.sqrt(covs[i-1]))

            if dist01 < sig01:
                nmeans.append( 0.5*(means[i] + means[i-1]) )
                ncovs.append( 0.5*(covs[i] + covs[i-1]) )
            else:
                nmeans.append( means[i-1] )
                ncovs.append( covs[i-1] )

            if dist12 < sig12:
                nmeans.append( 0.5*(means[i+1] + means[i]) )
                ncovs.append( 0.5*(covs[i+1] + covs[i]) )
            else:
                nmeans.append( means[i] )
                ncovs.append( covs[i] )
                nmeans.append( means[i+1] )
                ncovs.append( covs[i+1] )



    return nmeans, ncovs

##    found_distrs = zip(means, covs)
##    for m, cov in found_distrs:
##        print "m", m, "cov", cov
##    return means, covs


def separateTwoPeaks(image):
    """
    Method to threshold image computing the histogram and choosing the threshold between two peaks
    return thresholded image if two peaks are identified and None otherwise
    """
    ImageTools.assertOneBand(image)

    # compute histogram :
    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image, 1000)
    nhist = bandHists[0,:]
    minValue = minValues[0]
    maxValue = maxValues[0]
    # compute quantile values to reject near-zero histogram values
    [qmin, qmax] = ImageTools.computeQuantiles(nhist, 0.0, 0.990)
    print qmin, qmax
    minValue = minValues[0] + qmin * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
    maxValue = minValues[0] + qmax * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
    nhist = nhist[qmin:qmax]

    # Estimate peaks as two gaussians
    x = ImageTools.computeColorValues(minValue, maxValue, nhist.shape[0])
    means, covs = estimateGMM(x, nhist)

##    print "means", means
##    print "stds", np.sqrt(covs)
##    plt.plot(x, nhist,'-g')
##    # test :
##    pts = invTransfSampling2(x, nhist, 1000)
##    plt.hist(pts, x.shape[0])
##    for m, c in zip(means, covs):
##        g1 = gaussian(x, m, np.sqrt(c))
##        plt.plot(x,g1,'-r')
##    plt.show()


    # Compute threshold value as middle between two peaks
    if len(means) == 2:
        a = 0.4
        t = means[0] * a + means[1] * (1.0 - a)
        print "Threshold value : ", t
        tImage = image.copy()
        tImage[ tImage <= t ] = 0
        tImage[ tImage > t ] = 255
        return tImage
    else:
        return None


def separateTwoPeaks2(image):
    """
    Method to threshold image computing the histogram and choosing the threshold between two peaks
    return thresholded image if two peaks are identified and None otherwise
    """
    ImageTools.assertOneBand(image)

    # compute histogram :
    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image, 1000)
    nhist = bandHists[0,:]
    minValue = minValues[0]
    maxValue = maxValues[0]

    x = ImageTools.computeColorValues(minValue, maxValue, nhist.shape[0])

    # Fit peaks:
    params = gaussFitData(x, nhist)
    if params is None:
        return None

    # Found two peaks
    mu1, std1, a1, mu2, std2, a2 = params
    # assert that a1 and a2 are positive
    if a1 < 0 or a2< 0:
        return None

    # Compute threshold value as middle between two peaks
    a = 0.6
    t = mu1 * a + mu2 * (1.0 - a)
    print "Threshold value : ", t
    tImage = image.copy()
    tImage[ tImage <= t ] = 0
    tImage[ tImage > t ] = 255
    return tImage


def gaussFitData(x, nparray):
    ImageTools.assert1D(nparray)
    ImageTools.assert1D(x)


    # Initial guess :
    mu1 = x[int(np.argmax(nparray) * 0.1)]
    std1 = 1.0
    a1 = 0.5
    mu2 = x[np.argmax(nparray)]
    std2 = 0.9
    a2 = 1.0
    params = [mu1,std1,a1,mu2,std2,a2]

    # define residual function :
    def res(params, y, x):
        mu1, std1, a1, mu2, std2, a2 = params
        yFit = gaussian(x,mu1,std1,a1) + gaussian(x,mu2,std2,a2)
        err = y - yFit
        return err

    sol = sc.leastsq(res, params, args=(nparray, x))

    if sol[1] > 4 :
        Global.logPrint("Solution is not found", 'error')
        return None

    print sol
    return sol[0]

if __name__ == '__main__':


##    x=np.arange(10,110,0.05)
####    h=np.arange(0.1,1.0,0.01)
##    h=np.zeros(100)
##    h[15:25] = 0.3
##    h[47:82] = 1.0
##
##    h = gaussian(x, 30, 7) + gaussian(x, 70, 5)
##    means, covs = estimateGMM(x,h)
##
##    plt.plot(x,h,'-g')
##    for m, c in zip(means, covs):
##       g1 = gaussian(x, m, np.sqrt(c))
##       plt.plot(x,g1,'-r')
##    plt.show()

##    kmeans(x,h)
##    mv, stdv = meanStdVec22(x,h)
##    plt.plot(x, mv, '-b')
##    plt.plot(x, stdv, '-r')
##    plt.show()
##    exit()



    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\filtered\\img5.tif"
    image = ImageTools.loadImage(filename)
    if image is None:
        Global.logPrint("Failed to load image", 'error')
        exit()
    nbBands = 1 if len(image.shape) == 2 else image.shape[2]
    Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))

    ImageTools.displayImage(image, True, "Original", False)

##    # compute histogram :
##    [bandHists, minValues, maxValues] = ImageTools.computeNormHist(image, 1000)
##    nhist = bandHists[0,:]
##    minValue = minValues[0]
##    maxValue = maxValues[0]



##    # compute quantile values to reject near-zero histogram values
##    [qmin, qmax] = ImageTools.computeQuantiles(nhist, 0.0, 0.990)
##    print qmin, qmax
##    minValue = minValues[0] + qmin * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
##    maxValue = minValues[0] + qmax * (maxValues[0] - minValues[0]) * 1.0/nhist.shape[0]
##    nhist = nhist[qmin:qmax]

##    x = ImageTools.computeColorValues(minValue, maxValue, nhist.shape[0])
##
##    mu1, std1, a1, mu2, std2, a2 = gaussFitData(x, nhist)
##
##    plt.plot(x, nhist,'-g')
##
##    g1 = gaussian(x, mu1, std1, a1)
##    plt.plot(x,g1,'-r')
##    g2 = gaussian(x, mu2, std2, a2)
##    plt.plot(x,g2,'-b')


##    # test :
##    pts = invTransfSampling2(x, nhist, 1000)
##    plt.hist(pts, x.shape[0])
##    for m, c in zip(means, covs):
##        g1 = gaussian(x, m, np.sqrt(c))
##        plt.plot(x,g1,'-r')
##    plt.show()


    sImage = separateTwoPeaks2(image)
    if sImage is not None:
        ImageTools.displayImage(sImage, True, "Tresholded")
    else:
        print "Failed to threshold image"


##    meanVec, stdVec, x = meanStdVec(image)
##
##    ImageTools.displayHist(image, False, ['g'])
##    dImage = ImageTools.downsample(image, (10, 10))
##    estimateGMM(dImage)
##    ImageTools.displayHist(dImage)




##    kmeans(x, bandHists[:,0])
##    plt.plot(x, ImageTools.norm(meanVec), '-b')
##    plt.plot(x, ImageTools.norm(stdVec), '-r')
##    plt.show()


##    tImage = grayLevelHistogram(image)
##    ImageTools.displayImage(tImage, True, "gray level hist")
##
##    ImageTools.displayHist(image, False, ['b'])
##    ImageTools.displayCumHist(image)

    cv2.destroyAllWindows()

##    tImage = iterativeSelection(image)
##    ImageTools.displayImage(tImage, True, "iterative threshold")
