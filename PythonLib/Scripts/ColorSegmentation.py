
# Numpy
import numpy as np

# Matplotlib
import matplotlib.pyplot as plt

# Opencv
import cv2

# Project
import Global
import ImageTools


def testIsEqual(tol=0.01, isEqual=lambda x,y,tol : (x-y)*(x-y)*1.0/(x + tol*0.001) *1.0/(y + tol*0.001) < tol):

    x = np.arange(0.0, 1000.0, 1.0)
    res = np.zeros((len(x),len(x)))

    for i, line in enumerate(res):
        y = x[i]
        res[i,:] = isEqual(x[:],y,tol)

    plt.pcolor(res)
    plt.show()


def segment2(image, nbClasses=2):
    pass



def segment(image, tol=0.01, isEqual=lambda x,y,tol : (x-y)*(x-y)*1.0/x *1.0/y < tol):

    """
    Segmentation method consists of coloring image pixels if they are 'connected'.

    Image is seen as a graph. Pixel correspond to vertices. Edges are defined if
    pixel are 'same' (under a condition).

    """

    output=np.zeros(image.shape)-1;

    # compute edge connections
    edgeConnections={}
    n = len(image)
    for i, line in enumerate(image):
        m = len(line)
        for j, pixel in enumerate(line):
            edgeConnections[m*i+j]=[]
            # check 8-connectivity : check 3 pixels below and one on the right
##            if i-1 >= 0 and j-1 >= 0 and isEqual(pixel, image[i-1][j-1],tol):
##                edgeConnections[m*i+j].append((i-1)*m+j-1)
##            if i-1 >= 0 and isEqual(pixel, image[i-1][j],tol):
##                edgeConnections[m*i+j].append((i-1)*m+j)
##            if i-1 >= 0 and j+1 < m and isEqual(pixel, image[i-1][j+1],tol):
##                edgeConnections[m*i+j].append((i-1)*m+j+1)
##
##            if j-1 >= 0 and isEqual(pixel, image[i][j-1],tol):
##                edgeConnections[m*i+j].append(i*m+j-1)
            if j+1 < m and isEqual(pixel, image[i][j+1],tol):
                edgeConnections[m*i+j].append(i*m+j+1)

            if i+1 < n and j-1 >= 0 and isEqual(pixel, image[i+1][j-1],tol):
                edgeConnections[m*i+j].append((i+1)*m+j-1)
            if i+1 < n and isEqual(pixel, image[i+1][j],tol):
                edgeConnections[m*i+j].append((i+1)*m+j)
            if i+1 < n and j+1 < m and isEqual(pixel, image[i+1][j+1],tol):
                edgeConnections[m*i+j].append((i+1)*m+j+1)


    print "Edge connections size", len(edgeConnections)
##    print edgeConnections

    # color vertices
    color=0;
    for i, line in enumerate(output):
        m = len(line)
        for j, pixel in enumerate(line):
            if pixel >= 0:
                continue
            stack=[i*m+j]
            while (len(stack) > 0):
                v=stack.pop()
                if output.ravel()[v] < 0:
                    output.ravel()[v] = color
                    stack.extend(edgeConnections[v])

            color+=1
    return output




if __name__ == '__main__':


##    testIsEqual()
    tol=12.0
    isEqual = lambda x,y,tol: np.sqrt((x-y)*(x-y)) < tol
##
##    testIsEqual(tol, isEqual)
##    exit()

    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\filtered\\img1.tif"
    image = ImageTools.loadImage(filename)
    if image is None:
        Global.logPrint("Failed to load image", 'error')
        exit()
    nbBands = 1 if len(image.shape) == 2 else image.shape[2]
    Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))
##    ImageTools.displayImage(image)

##    dImage = np.zeros((3,3))
##    dImage[0,0]=10.5
##    dImage[0,1]=10.5
##    dImage[1,0]=10.5
##
##    dImage[0,2]=1.5
##    dImage[2,0]=5.2
##
##    dImage[1,1]=15.2
##    dImage[1,2]=15.2
##    dImage[2,1]=15.2
##    dImage[2,2]=15.2

    dImage = ImageTools.downsample(image, (10, 10))
##    dImage = dImage[0:10,0:10]
##    print dImage
    ImageTools.displayImage(dImage)

    cImage = segment(dImage, tol, isEqual)
##    print cImage
    ImageTools.displayImage(cImage, True, "segmented")


    cv2.destroyAllWindows()
