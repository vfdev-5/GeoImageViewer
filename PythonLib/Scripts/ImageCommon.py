# Some image common methods
"""
Image common module contains useful functions ...

Assert methods:
    assertIsNPArray() : asserts if input is numpy array
    assertImage() : asserts if input is numpy array and an image with 2 or 3 dimensions
    assert1D() : asserts if input is numpy 1D array
    assertTwoBands() : asserts if input is 2 bands image
    assertOneBand() : asserts if input is 1 bands image
"""

# Numpy
import numpy as np

# Project
import Global


# Assert methods
# ###############################################################################
def assertIsNPArray(image):
    assert isinstance(image, np.ndarray), Global.logPrint("Input should be a Numpy array",'error')

def assertImage(image):
    assertIsNPArray(image)
    assert len(image.shape) == 2 or len(image.shape) == 3, Global.logPrint("Input should be a Numpy array of shape : (H,W,nbBands) or (H,W)",'error')

def assert1D(x):
    assertIsNPArray(x)
    assert len(x.shape) == 1, Global.logPrint("Input should have 1-D array",'error')

def assertTwoBands(image):
    assertIsNPArray(image)
    assert len(image.shape) == 3 and image.shape[2] == 2, Global.logPrint("Image should have 2 channel",'error')

def assertOneBand(image):
    assertIsNPArray(image)
    assert len(image.shape) == 2 or image.shape[2] == 1, Global.logPrint("Image should have 1 channel",'error')



# Main
# ###############################################################################
if __name__ == '__main__':

    pass

