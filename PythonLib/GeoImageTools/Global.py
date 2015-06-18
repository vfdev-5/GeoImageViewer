#-------------------------------------------------------------------------------
# Name: Global
# Purpose: Collection of some useful global methods
#
# Author:      vfomin
#
# Created:     09/12/2014
# Copyright:   (c) vfomin 2015
# Licence:     <your licence>
#-------------------------------------------------------------------------------

# Python
import os
import sys
import subprocess
import logging
import glob
import datetime
import time
from functools import wraps

# setup enviroment if not yet done (case of usage of 7z in UnitTest)
path = os.path.dirname(__file__)
if not sys.path.count(os.path.abspath(path+"/../Utils")) > 0: sys.path.append(os.path.abspath(path+"/../Utils"))

def getSARProductsExtensions():
    return ['.tif','.h5','.N1', '.E2', '.E1'];

def getZippedExtensions():
    return ['.gz', '.tar','.tar.gz','.7z','.rar','.zip', '.tgz']

def existsPath(path):
    if os.path.exists(path):
        logPrint("Input file or folder is " + path)
        return True
    else:
        return False


def getBasename(filepath):
    bfn = os.path.basename(filepath)
    return '.'.join(bfn.split('.')[:-1])


def getFilesFromPath2(path, extensions=[]):
    files = []
    # test if path is a file
    if os.path.isfile(path):
        files.append(path)
        return files

    for dp, dn, filenames in os.walk(path):
        for f in filenames:
            if not extensions:
                files.append(os.path.join(dp, f))
            else:
                for ext in extensions:
                    fileExt='.'+'.'.join(f.split('.')[-ext.count('.'):])

                    if fileExt == ext or fileExt.lower() == ext or fileExt.upper() == ext:
                        files.append(os.path.join(dp, f))
                        break
    return files


def findFilesFromPath(path, expression):
    return glob.glob(path + '/' + expression)

def createWindDataPairs(ew_files, nw_files):
    # create pairs :
    pairs=[]
    for ncf in nw_files:
        nncf = ncf.replace('northward_wind_', '')
        for ecf in ew_files:
            necf = ecf.replace('eastward_wind_', '')
            if nncf == necf:
                pairs.append([ncf,ecf])

    if len(pairs) != len(nw_files):
        logPrint("Warining : Number of found pairs is not equal to the number of north-component files")
    return pairs



def unzipFiles(files, outputFolder, inSepFolders=False):
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)

    program=["7z", "e", "none", "-y"]
    program.append("-o"+outputFolder)
    for f in files:
        program[2]=f
        splt = os.path.basename(f).split(".")
        if inSepFolders:
            program[4] = "-o"+outputFolder+'/'+splt[0]
        errCode = subprocess.call(program, env={'PATH': ';'.join(sys.path)}, shell=True)
        if errCode > 0:
            logPrint("Failed to unzip the file : " + f, level='error')
            continue

        if len(splt) > 2 and isArchive(splt):
            if inSepFolders:
                nf = outputFolder+'/'+splt[0] + "/" + ".".join(splt[:-1])
            else:
                nf = outputFolder + "/" + ".".join(splt[:-1])
            program[2] = nf
            errCode = subprocess.call(program, env={'PATH': ';'.join(sys.path)}, shell=True)
            if errCode > 0:
                logPrint("Failed to unzip the file : " + f, level='error')

            os.remove(nf)

def isArchive(splt):
    if splt[1].lower() == 'tar':
        return True
    return False

def unzipFiles2(files, outputFolder, inSepFolders=False):
    """ Iterative unzip procedure : input files -> generate unzipped files in output folder """
    """ Loop unzipFiles on output files until no compressed files is found """
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
    for f in files:
        unzipFile(f, outputFolder, inSepFolders)

    compressedFiles=getFilesFromPath2(outputFolder, getZippedExtensions())
    while compressedFiles:
        for cf in compressedFiles:
            outf = os.path.dirname(cf)
            unzipFile(cf, outf, False)
            ## delete source compressed files
            os.remove(cf)
        compressedFiles=getFilesFromPath2(outputFolder, getZippedExtensions())


def unzipFile(filename, outputFolder, inSepFolders=False):
    program=["7z", "e", filename, "-y"]
    program.append("-o"+outputFolder)

    if inSepFolders:
        foldername=os.path.basename(filename)
        for ext in getZippedExtensions():
            foldername = foldername.replace(ext,"")
        program[4] = "-o"+outputFolder+'/'+foldername

    if not startProcess(program):
        logPrint("Failed to unzip the file : " + filename, level='error')



def filterFiles(files, catchKeys, excludeKeys):
    filteredFiles = []
    if catchKeys:
        if excludeKeys:
            for f in files:
                append=True
                for ek in excludeKeys:
                    if (f.find(ek) > 0):
                        append=False
                        break
                if append:
                    for ck in catchKeys:
                        if (f.find(ck) > 0):
                            filteredFiles.append(f)
                            break

        else:
            for f in files:
                for ck in catchKeys:
                    if (f.find(ck) > 0):
                        filteredFiles.append(f)
                        break
    else:
        if excludeKeys:
            for f in files:
                append=True
                for ek in excludeKeys:
                    if (f.find(ek) > 0):
                        append=False
                        break
                if append:
                    filteredFiles.append(f)

    return filteredFiles



def writeCsvFile(outputfilename, header, data):
    nbf = len(header)
    if len(data[0]) != nbf:
        logPrint("Number of field in data is not equal to the length of the header", level='error')
        return False

    outfile = open(outputfilename, 'w')
    # write the header :
    h=';'.join(header)
    h=h+'\n'
    outfile.write(h)

    # write data:
    for line in data:
        if len(line) != nbf:
            logPrint("Number of field in data is not equal to the length of the header", level='error')
            return False
        l=';'.join(line)
        l=l+'\n'
        outfile.write(l)

    outfile.close()
    if os.path.exists(outputfilename):
            logPrint("CSV file is written successfully")
            return True
    else:
            logPrint("Failed to write the output file",level='error')
            return False


def readCsvFileColumns(inputfilename, headerFilter, data):
    nbCols = len(headerFilter)
    hasHeaderFilter=False
    if nbCols > 0:
        hasHeaderFilter=True

    try:
        inFile = open(inputfilename, 'r')
    except:
        logPrint("Input file is not found : " + inputfilename,level='error')
        return False
    header = inFile.readline()
    headerItems = header.split(';')
    headerItems[-1]=headerItems[-1].replace('\n','')

    if not hasHeaderFilter:
        for item in headerItems:
            headerFilter.append(item)

    indices=[]
    for hf in headerFilter:
        try:
            index = headerItems.index(hf);
        except:
            index = -1
        indices.append( index )

    for line in inFile:
        lineItems = line.split(';')
        lineItems[-1]=lineItems[-1].replace('\n','')
        dataLine=[]
        for index in indices:
            if index >= 0:
                dataLine.append(lineItems[index])
        data.append(dataLine)
    return True


def convertAttribsXmlToGDALXml(attribXmlFiles):
    program=["convertL1BCSKXmlMDToGDAL", 'filename']
    for f in attribXmlFiles:
        program[1]=f
        splt = os.path.basename(f).split(".")
        errCode = subprocess.call(program, env={'PATH': ';'.join(sys.path)}, shell=True)
        if errCode > 0:
            logPrint("Failed to convert attribs.xml file : " + f, level='error')
            continue


def DegreeToDecimal(degrees, minutes, seconds):
    return degrees + minutes/60.0 + seconds/3600.0

def DegreeToDecimal2(degrees, minutes):
    return degrees + minutes/60.0


def convertToTypeFromValueAsString(item):
    """ Test if the item is float, datetime in a specific format or string """
    try:
        res=float(item)
        return res, type(res)
    except ValueError:
        # item is not a float
        pass

##    try:
##        res=datetime.datetime.strptime(item, "%Y/%m/%d %H:%M:%S")
##        return res, type(res)
##    except ValueError:
##        # item is not a float
##        pass

    return item, type(item)


##def displayData(numpyArray, showMinMax=False):
##    if not HAS_MATPLOTLIB:
##        print "Not implemented"
##        return


def logPrint(msg, level='info'):
    print msg
    if level == 'info':
        logging.info(msg)
    elif level == 'error':
        logging.error(msg)

def startProcess(program, verbose=False):
    locEnv=os.environ;
    locEnv['PATH']=str(';'.join(sys.path))
    proc = subprocess.Popen(program, env=locEnv, shell=True, stderr=subprocess.PIPE)
    (stdout, stderr) = proc.communicate()

    if verbose and stdout:
        print stdout

    if stderr:
        logPrint(stderr, level='error')
        return False
    return True


# ########################################################################################################
# PROFILING : http://stackoverflow.com/questions/3620943/measuring-elapsed-time-with-the-time-module
# Usage :
# @profile
#    def your_function(...):
#    ...
#
# print_prof_data()
# ########################################################################################################

PROF_DATA = {}
def profile(fn):
    @wraps(fn)
    def with_profiling(*args, **kwargs):
        start_time = time.time()

        ret = fn(*args, **kwargs)

        elapsed_time = time.time() - start_time

        if fn.__name__ not in PROF_DATA:
            PROF_DATA[fn.__name__] = [0, []]
        PROF_DATA[fn.__name__][0] += 1
        PROF_DATA[fn.__name__][1].append(elapsed_time)

        return ret

    return with_profiling

def print_prof_data():
    for fname, data in PROF_DATA.items():
        max_time = max(data[1])
        avg_time = sum(data[1]) / len(data[1])
        print "Function %s called %d times. " % (fname, data[0]),
        print 'Execution time max: %.3f, average: %.3f' % (max_time, avg_time)

def clear_prof_data():
    global PROF_DATA
    PROF_DATA = {}
# ########################################################################################################

##@profile
##def test_foo():
##    data=[0]*1000*1000
##    data[1] = 1
##    for c in range(len(data)-1):
##        data[c] = data[c-1] * 0.5

if __name__ == "__main__":

# Test profiling
##    for i in range(5):
##        test_foo()
##    print_prof_data()


##    p="C:/VFomin_folder/PISE_project/Images/ERS/TestData"
##    f="C1F.18649_11_3_0001_0_20140606130948.tar"
##    f1 = p + "/" + f
##    unzipFile(f1, p)


    pass
