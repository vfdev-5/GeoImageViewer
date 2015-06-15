#-------------------------------------------------------------------------------
# Name:        Dark Object Detector
# Purpose:
#
# Author:      vfomin
#
# Created:     24/03/2015
# Copyright:   (c) vfomin 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------

# Python
import sys
import time
import os

# Opencv
import cv2

# Local
import Global
import ImageTools
import Threshold
import GeoImageWriter



class DarkObjectDetector:
    """
    Class to detect dark objects on the image and store result as shapefile
    """
    def __init__(self):
        self.outputFolder=""
        self.lowPassBandSize = [4.0, 4.0]
        self.enhanceContrastParams = [0.55, 95.0]
        self.stackThresholdingParams = [20, 1.0]
        self.seedsThreshold = 0.6
        self.verbose = False
        self.saveMask = False



    def run(self, filename):
        # Open image :
        image = ImageTools.loadImage(filename)
        if image is None:
            Global.logPrint("Failed to load image", 'error')
            return False

        if self.verbose:
            nbBands = 1 if len(image.shape) == 2 else image.shape[2]
            Global.logPrint("Image info: " + str(image.shape[0]) + ", " + str(image.shape[1]) + ", nbBands=" + str(nbBands))
            ImageTools.displayImage(image, True, "Original", False)

        # Pre-Filter image
        #   FFT Gauss filtering :
        filteredImage = ImageTools.norm(Threshold.gaussianLowPassFreqFilter(image, self.lowPassBandSize[0], self.lowPassBandSize[1]))

        #   Enhance contrast :
        filteredImage = Threshold.enhanceContrast(filteredImage, self.enhanceContrastParams[0], self.enhanceContrastParams[1])

        if self.verbose:
            ImageTools.displayImage(filteredImage, True, "pre-filtered image", False)
            ImageTools.displayHist(filteredImage, colors=['r'])

        # Stack Thresholding :
        n = self.stackThresholdingParams[0]
        sig = self.stackThresholdingParams[1]
        uMin = filteredImage.min()
        uMax = filteredImage.mean() - sig*filteredImage.std()
        stackThresholdedImage = Threshold.stackThresholding(filteredImage, n, sig, uMin, uMax)

        if self.verbose:
            ImageTools.displayImage(stackThresholdedImage, True, "stackThresholdedImage", False)


        # Flood fill
            # get seeds
        seedsThreshold = self.seedsThreshold
        seedsImage = ImageTools.norm(stackThresholdedImage)
        seedsImage[ seedsImage >= seedsThreshold ] = 1.0
        seedsImage[ seedsImage < seedsThreshold ] = 0.0

        if self.verbose:
            ImageTools.displayImage(seedsImage, True, "seedsImage", True)

        if self.verbose:
            start_time = time.time()

        mask = Threshold.floodFill(ImageTools.norm(stackThresholdedImage),seedsImage, lowDiff=0.5, upDiff=0.5)

        if self.verbose:
            elapsed = time.time() - start_time
            Global.logPrint("Elapsed time : " + str(elapsed) + " seconds")
            ImageTools.displayImage(mask, True, "Mask image")

        if len(self.outputFolder) > 0:
            opath = self.outputFolder
        else:
            opath = os.path.dirname(filename)

        if self.saveMask:
            ofilename = opath +  "/mask_" + Global.getBasename(filename) + ".tif"
            GeoImageWriter.writeDataInGeoImage(ofilename, mask)

        ofilename = opath +  "/mask_" + Global.getBasename(filename) + ".shp"
        GeoImageWriter.writeVectorFromMask(mask, ofilename, "Dark_Objects")

        return True


if __name__ == '__main__':

    detector = DarkObjectDetector()
    filename = None

    args = sys.argv[1:]
    skipOnce=False
    for (count, arg) in enumerate(args):
        if skipOnce:
            skipOnce=False
            continue
        if arg == '--help' or arg == '-h':
            print "Usage: DarkObjectDetector.py C:\test\image.tif -o C:\output_folder [-v] [--save_mask]"
        elif arg == '-o':
            detector.outputFolder = args[count+1]
            skipOnce=True
        elif arg == '-v':
            detector.verbose = True
        elif arg == '--save_mask':
            detector.saveMask = True
        else:
            filename = arg


    if filename is not None:
        start_time = time.time()
        detector.run(filename)
        elapsed = time.time() - start_time
        Global.logPrint("DarkObjectDetector run time : " + str(elapsed) + " seconds")
        cv2.destroyAllWindows()
        exit()


    # Tests :
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4219.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_1189.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_3784.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4218.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4219_1.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4219_2.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4225.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\AOI\\AOI_Slick_seep_4255.tif"

##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\filtered\\img5.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_1.tif"
    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_4.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_5.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_6.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_7.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_9.tif"
##    filename = "C:\\VFomin_folder\\PISE_project\\MyExamples\\Qt_GeoImageViewer_test\\Test_Image_Data\\orig\\test_10.tif"

    detector.verbose = True
    detector.outputFolder = "C:\\VFomin_folder\\PISE_project\\Slick_Image_Database\\Seeps\\ASAR\\GIV_python_test"

    start_time = time.time()
    detector.run(filename)
    elapsed = time.time() - start_time
    Global.logPrint("DarkObjectDetector run time : " + str(elapsed) + " seconds")
    cv2.destroyAllWindows()



