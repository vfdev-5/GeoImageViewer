# Script to extract metadata from satelite imagery

# Python
import datetime
import re
import os
import math
import shutil

# numpy
import numpy

# scipy
import scipy
from scipy.stats.mstats import mquantiles

# GDAL
import gdal
import osgeo.osr
from gdalconst import *


# MatplotLib
##import matplotlib.pyplot
##import matplotlib.colors

# local
import Global


class ImageMetadataExtractor:

        def __init__(self):
            """ empty constructor """

        @staticmethod
        def getDataTypeMinMax(filename):
            dtmin=0
            dtmax=0
            ds=gdal.Open(filename, GA_ReadOnly)
            if ds is None:
                Global.logPrint("Problem with the file : " + filename, level='error')
                return dtmin, dtmax

            band = ds.GetRasterBand(1)
            dataType = band.DataType
            depth = gdal.GetDataTypeSize(dataType)
            if (dataType == gdal.GDT_Byte) or (dataType == gdal.GDT_UInt16) or (dataType == gdal.GDT_UInt32):
                dtmin=0
                dtmax=pow(2.0, depth)
            else:
                dtmin=-pow(2.0, depth/2)
                dtmax=pow(2.0, depth/2)

            return dtmin, dtmax


        @staticmethod
        def getSpatialExtentBBox(inputFilename):
            se = ImageMetadataExtractor.getSpatialExtent(inputFilename)
            bb_ul_x=361.0
            bb_ul_y=-181.0
            bb_lr_x=-361.0
            bb_lr_y=181.0
            for gpt in se:
                bb_ul_x = gpt[0] if bb_ul_x > gpt[0] else bb_ul_x
                bb_ul_y = gpt[1] if bb_ul_y < gpt[1] else bb_ul_y
                bb_lr_x = gpt[0] if bb_lr_x < gpt[0] else bb_lr_x
                bb_lr_y = gpt[1] if bb_lr_y > gpt[1] else bb_lr_y

            return bb_ul_x, bb_ul_y, bb_lr_x, bb_lr_y

        @staticmethod
        def getSpatialExtent(inputFilename):
            gpts = []
            ds = gdal.Open(inputFilename, GA_ReadOnly)
            if ds is None:
                Global.logPrint("Problem with the file : " + inputFilename, level='error')
                return gpts

            # transform 4 image corners
            w = ds.RasterXSize
            h = ds.RasterYSize
            pts = [[0,0], [w,0], [w,h],[0,h]]

            #"DST_SRS", dstSRSWkt
            srs = osgeo.osr.SpatialReference()
            srs.ImportFromEPSG( 4326 )
            dstSRSWkt = srs.ExportToWkt()
            options = [ 'DST_SRS=' + dstSRSWkt ]

            transformer = gdal.Transformer(ds, None, options)

            for pt in pts:
                g = transformer.TransformPoint(False, pt[0], pt[1], 0.0)
                gpts.append([g[1][0], g[1][1]])

            return gpts

        @staticmethod
        def fetchLatLonMinMaxInfo(dataset):
            md = ImageMetadataExtractor.fetchMetadata(dataset)
            keys = ImageMetadataExtractor.getLatLonMinMaxKeys()
            values=ImageMetadataExtractor.fetchValues(md, keys, False)
            lonMin = None
            lonMax = None
            latMin = None
            latMax = None
            for key in keys:
                if "lon" in key and "min" in key:
                    lonMin = float(values[key])
                    lonMin = lonMin - 360 if lonMin > 180 else lonMin
                elif "lon" in key and "max" in key:
                    lonMax = float(values[key])
                    lonMax = lonMax - 360 if lonMax > 180 else lonMax
                elif "lat" in key and "min" in key:
                    latMin = float(values[key])
                elif "lat" in key and "max" in key:
                    latMax = float(values[key])

            return lonMin, lonMax, latMin, latMax


        @staticmethod
        def hasSubsets(filename, subsetNames=[], subsetDesriptions=[]):
            ds=gdal.Open(filename, GA_ReadOnly)
            if ds is None:
                Global.logPrint("Problem with the file : " + filename)
                return False;

            subsets = ds.GetMetadata("SUBDATASETS")
            if not subsets:
                return False

            for item in subsets.keys():
                if not ("QLK" in subsets[item]):
                    if "NAME" in item:
                        subsetNames.append(subsets[item])
                        itemDescr = item[:-4] + "DESC"
                        subsetDesriptions.append(subsets[itemDescr])

            return subsetNames > 0

        @staticmethod
        def getImageResolution(filename):
            ds=gdal.Open(filename, GA_ReadOnly)
            if ds is None:
                Global.logPrint("Problem with the file : " + filename,'error')
            geotransform = ds.GetGeoTransform()
            return abs(geotransform[1]), abs(geotransform[5])

        def extractVarName(self, md):
            keys = ImageMetadataExtractor.getVarNameKeys()
            values = ImageMetadataExtractor.fetchValues(md, keys, True)
            if len(values) == 0:
                Global.logPrint("No variable name info in the metadata on keys : " + keys[0] + ' ...', 'error')
                return []
            k = values.keys()[0]
            return values[k]


        def extractTimeValues(self, md, refTime, units):
            if not refTime or not units:
                return []
            keys = ImageMetadataExtractor.getAcquisitionTimeKeys()
            values = ImageMetadataExtractor.fetchValues(md, keys, True)
            if len(values) == 0:
                Global.logPrint("No time info in the metadata", 'error')
                return []
            k = values.keys()[0]
            t = values[k]
            return self.computeTime(refTime, t, units)


        def computeTime(self, refTime, time, units):
            dt=datetime.datetime.strptime(refTime,"%Y%m%d-%H%M%S")
            ct=float(time)
            if "seconds" in units:
                return dt + datetime.timedelta(0, ct)
            elif "minutes" in units:
                return dt + datetime.timedelta(0, 60.0*ct)
            elif "hours" in units:
                return dt + datetime.timedelta(ct/24.0)
            elif "days" in units:
                return dt + datetime.timedelta(ct)
            return dt


        def extractZLevelValues(self, md):
            keys = ImageMetadataExtractor.getZLevelValuesKeys()
            values = ImageMetadataExtractor.fetchValues(md, keys, True)
            if len(values) == 0:
                #print "No Z-level values in the metadata"
                return []
            k = values.keys()[0]
            return values[k]


        def getReferenceTime(self, ds):
            md = ImageMetadataExtractor.fetchMetadata(ds)
            keys = ImageMetadataExtractor.getReferenceTimeKeys()
            values=ImageMetadataExtractor.fetchValues(md, keys, True)
            # get reference time :
            #print "values", values
            if len(values) == 0:
                Global.logPrint("No reference time and units in the metadata",'error')
                return [], []

            rTime = values.items()[0][1]
            if not ":" in rTime:
                rTime += " 00:00:00"

            #print rTime
            pattern='(\S+)\s(\S+\s)(\d\d\d\d-\d\d-\d\d)\s(\d\d:\d\d:\d\d)'
            m=re.search(pattern, rTime)
            if not m is None:
                units=m.group(1)
                ymd=m.group(3)
                hms=m.group(4)
                ymd=ymd.replace("-","")
                hms=hms.replace(":","")
                referenceTime=ymd+"-"+hms
                return referenceTime, units
            else:
                Global.logPrint("Failed to fetch reference time and units",'error')
                return [], []

        @staticmethod
        def getLatLonMinMaxKeys():
            keys = []
            keys.append("lon#valid_min")
            keys.append("lon#valid_max")
            keys.append("lat#valid_min")
            keys.append("lat#valid_max")
            return keys

        @staticmethod
        def getVarNameKeys():
            keys = []
            keys.append("NETCDF_VARNAME")
            return keys

        @staticmethod
        def getZLevelValuesKeys():
            keys = []
            keys.append("NETCDF_DIM_depth")
            return keys

        @staticmethod
        def getReferenceTimeKeys():
            keys = []
            keys.append('time#units')
            keys.append('time_counter#units')
            keys.append('julian_day_unit')
            return keys

        @staticmethod
        def getAcquisitionTimeKeys():
                keys=[]
                keys.append('ACQUISITION_TIME')
                keys.append('Azimuth First Time')
                keys.append('SENSING_START')
                keys.append('TIFFTAG_DATETIME')
                keys.append('FIRST_LINE_TIME')
                keys.append('NETCDF_DIM_time')
                keys.append('NETCDF_DIM_time_counter')
                keys.append('DATE_ACQUIRED')
                keys.append('SCENE_CENTER_TIME')
                return keys

        @staticmethod
        def getAcquisitionModeKeys():
                keys=[]
                keys.append('Acquisition Mode')
                keys.append('Acquisition_Mode')
                keys.append('SPH_SPH_DESCRIPTOR')
                return keys

        @staticmethod
        def getAcquisitionProductTypeKeys():
                keys=[]
                keys.append('Product Type')
                return keys

        @staticmethod
        def getIncidenceAngleKeys():
                keys=[]
                keys.append('Near_Incidence_Angle')
                keys.append('Far_Incidence_Angle')
                keys.append('SPH_SWATH')
                return keys

        @staticmethod
        def getPolarizationKeys():
                keys=[]
                keys.append('SPH_MDS1_TX_RX_POLAR')
                keys.append('Polarisation')
                return keys

        @staticmethod
        def getSpacingKeys():
                keys=[]
                keys.append('Column_Spacing')
                keys.append('Line_Spacing')
                return keys

        @staticmethod
        def getResolutionsKeys():
                keys=[]
                keys.append('Ground_Range_Geometric_Resolution')
                keys.append('Azimuth_Geometric_Resolution')
                keys.append('SPH_AZIMUTH_SPACING')
                keys.append('SPH_RANGE_SPACING')
                return keys

        # Method to convert time string into datetime
        def getDatetime(self,time):
                patterns=[]
                patterns.append('(\d\d).(\S+).(\d\d\d\d) (\d\d):(\d\d):(\d\d)')
                patterns.append('(\d\d\d\d).(\d\d).(\d\d) (\d\d):(\d\d):(\d\d)')
                patterns.append('(\d\d\d\d).(\S+).(\d\d) (\d\d):(\d\d):(\d\d)')
                for pattern in patterns:
                        m=re.search(pattern, time)

                        if not m is None:
                                hours=m.group(4)
                                minutes=m.group(5)
                                seconds=m.group(6)
                                month=m.group(2)
                                if not len(month) == 2:
                                        try:
                                                d = datetime.datetime.strptime(month,"%b")
                                                month=d.strftime("%m")
                                        except:
                                                Global.logPrint("Failed to convert the month : " + str(month))
                                                month='01'


                                year=None
                                day=None
                                if len(m.group(1)) == 2:
                                        year=m.group(3)
                                        day=m.group(1)
                                else:
                                        year=m.group(1)
                                        day=m.group(3)

                                return datetime.datetime(int(year),int(month),int(day), int(hours), int(minutes), int(seconds))


        # Method to extract acquisition mode from the satellite imageries
        def fetchDataOnKeys(self, path, files, type, keys, firstOccurence=True):
                data={}
                for f in files:

                        if (type == "Reference"):
                                # fetch time from metadata:
                                if not path:
                                        filename = f
                                else:
                                        filename = path + "/" + f

                                ds=gdal.Open(filename, GA_ReadOnly)
                                if ds is None:
                                        Global.logPrint("Problem with the file : " + filename,'error')
                                        continue
                                metadata=ImageMetadataExtractor.fetchMetadata(ds)
                                #self.showMetadata(metadata)

                                # extract acquisition data:
                                # print "Keys : ", keys
                                values=ImageMetadataExtractor.fetchValues(metadata, keys, firstOccurence)
                                if not len(values) is 0:
                                        if firstOccurence:
                                                key, val = values.items()[0]
                                                data[f]=val
                                        else:
                                                data[f]=[]
                                                for key, value in values.items():
                                                        data[f].append(value)


                return data


        # Method to extract acquisition time from the satellite imageries
        def fetchTimes(self, path, files, type):
                times={}
                for f in files:
                    if not path:
                       filename = f
                    else:
                        filename = path + "/" + f
                    if (type == "Reference") or (type == "MultiSpectral"):
                                # fetch time from metadata:

                                ds=gdal.Open(filename, GA_ReadOnly)
                                if ds is None:
                                        Global.logPrint("Problem with the file : " + filename,'error')
                                        continue
                                metadata=ImageMetadataExtractor.fetchMetadata(ds)
                                # self.showMetadata(metadata)

                                # extract acquisition time:
                                keys=ImageMetadataExtractor.getAcquisitionTimeKeys();
                                values=ImageMetadataExtractor.fetchValues(metadata, keys)
                                if not len(values) is 0:
                                        key, time = values.items()[0]
                                        try:
                                            dt=self.getDatetime(time)
                                        except:
                                            Global.logPrint("Failed to parse the date and time from : " + f, 'error')
                                            continue
                                        times[f]=dt

                    elif (type == "Wind") or (type == "Metoc_tif"):
                                # fetch time from the file name:
                                # filename is XXXXX_YYYYMMDD-HHMMSS.tif
                                time=os.path.basename(f)[-19:-4]
                                try:
                                        dt=datetime.datetime.strptime(time,"%Y%m%d-%H%M%S")
                                except:
                                        Global.logPrint("Failed to parse the date and time from : " + f, 'error')
                                        continue
                                times[f]=dt

                    elif type == "CSK":
                                # fetch time from the file name:
                                # filename is CSKS<i>_<YYY_Z >_<MM>_<SS>_<PP>_<s><o>_<D><G>_<YYYYMMDDhhmmss>_<YYYYMMDDhhmmss>.h5
                                time=os.path.basename(f).split('.')[0].split('_')[-2]
                                try:
                                        dt=datetime.datetime.strptime(time,"%Y%m%d%H%M%S")
                                except:
                                        Global.logPrint("Failed to parse the date and time from : " + f, 'error')
                                        continue
                                times[f]=dt
                    elif type == "Landsat":
                            # fetch date and time from mtl text file
                            p = os.path.dirname(filename)
                            txtFiles = Global.getFilesFromPath2(p,['.txt'])
                            mtlfile=""
                            for tf in txtFiles:
                                if "MTL" in tf:
                                    mtlfile=tf
                                    break
                            if not mtlfile:
                                Global.logPrint("Failed to find MTL file", 'error')
                                return
                            md = ImageMetadataExtractor.fetchMetadataFromMTLFile(mtlfile)
                            # extract acquisition time:
                            keys=['DATE_ACQUIRED', 'SCENE_CENTER_TIME'];
                            values=ImageMetadataExtractor.fetchValues(md, keys, False)
                            if not len(values) is 0:
                                dtStr=values[keys[0]] + " " + values[keys[1]][0:8]

                            try:
                                dt=self.getDatetime(dtStr)
                            except:
                                Global.logPrint("Failed to parse the date and time from : " + f, 'error')
                                continue
                            times[f]=dt



                return times

        @staticmethod
        def fetchValues(metadata, keys, firstOccurence=True):
                values={}
                for fieldName in metadata:
                        for key in keys:
                                if key.lower() in fieldName.lower():
                                        values[key]=metadata[fieldName]
                                        if firstOccurence:
                                                return values;

                return values

        @staticmethod
        def fetchMetadataFromFile(filename):
                ds=gdal.Open(filename, GA_ReadOnly)
                if ds is None:
                        Global.logPrint("Problem with the file : " + filename,'error')
                        return
                return ImageMetadataExtractor.fetchMetadata(ds)


        @staticmethod
        def fetchMetadataFromMTLFile(filename):
            metadata={}
            mtlFile = open(filename, 'r')
            for line in mtlFile:
                line = line.replace(" ","")
                line = line.replace("\n","")
                splt = line.split("=")
                if len(splt) > 1:
                    metadata[splt[0]] = splt[1]
            return metadata


        @staticmethod
        def fetchMetadata(dataset):
                # get global metadata:
                metadata=dataset.GetMetadata()
                # get geolocation metadata:
                geoloc = dataset.GetMetadata("GEOLOCATION")
                if geoloc:
                    metadata = dict(metadata.items() + geoloc.items())

                # get metadata from each band:
                nbBands=dataset.RasterCount
                for i in range(1,nbBands+1):
                        band = dataset.GetRasterBand(i)
                        bandMetadata = band.GetMetadata()
                        metadata = dict(metadata.items() + bandMetadata.items())
                return metadata

        def showMetadata(self, metadata):
                print "---- Metadata : ----"
                for name in metadata:
                        print name, " : ", metadata[name]


        def searchFiles(self, refTime, files, condition):
                selection={}
                for f in files:
                        t=files[f]
                        if condition(t, refTime):
                                selection[f]=t
                return selection

        def displayResult(self, out):
                for o in out:
                        print o[0], o[1], o[2], o[3]


        def createPairs(self, inputTimesWind):
                # create list of North-component & East-component files:
                northComponentFiles = [ f for f in inputTimesWind if "northward_wind" in f ]
                eastComponentFiles = [ f for f in inputTimesWind if "eastward_wind" in f ]

                if len(northComponentFiles) != len(eastComponentFiles):
                        Global.logPrint("Warining : The number of North-component files and East-component files is not the same")

                # create pairs :
                pairs=[]
                for ncf in northComponentFiles:
                        nncf = ncf.replace('northward_wind_', '')
                        for ecf in eastComponentFiles:
                            necf = ecf.replace('eastward_wind_', '')
                        if nncf == necf:
                                pairs.append([ncf,ecf])

                if len(pairs) != len(northComponentFiles):
                        Global.logPrint("Warining : Number of found pairs is not equal to the number of north-component files")
                return pairs

        def appendMinMaxParams(self, path, inputTimesWind):
                pairs = self.createPairs(inputTimesWind)
                outputTimesMinMaxParamsWind={}
                for pair in pairs:
##                        print pair[0], pair[1]
                        filenameX = path + "/" + pair[0]
                        filenameY = path + "/" + pair[1]
                        minMax=self.computeAbsMinMaxFromTwoFiles(filenameX, filenameY)
                        outputTimesMinMaxParamsWind[pair[0]]=[inputTimesWind[pair[0]], minMax]
                        outputTimesMinMaxParamsWind[pair[1]]=[inputTimesWind[pair[1]], []]
                return outputTimesMinMaxParamsWind



        def computeMinMaxFromFile(self, filename):
                minMax=[]
                ds=gdal.Open(filename, GA_ReadOnly)
                if ds is None:
                        Global.logPrint("Problem with the file : " + filename, 'error')
                        return minMax
                nbBands=ds.RasterCount
                for i in range(1,nbBands+1):
                        band = ds.GetRasterBand(i)
                        min = band.GetMinimum()
                        max = band.GetMaximum()
                        if min is None or max is None:
                                (min,max)=band.ComputeRasterMinMax(1)
                                minMax.append([min,max])
                        ##print 'Min=%.3f, Max=%.3f' % (min,max)
                return minMax

        def computeMinMaxFromArray(self, array, p):
            lb=p
            ub=1.0-p
            m,v,M=scipy.stats.mstats.mquantiles(array, [lb, 0.5, ub],limit=(0.1,1e5))
            return m, M

        def computeMeanStdFromArray(self, array):
            mn = numpy.mean(array)
            std = numpy.std(array)
            return mn, std


        def computeAbsMinMaxFromTwoFiles(self, filenameX, filenameY):
                minMax=[]
                dsX=gdal.Open(filenameX, GA_ReadOnly)
                if dsX is None:
                        Global.logPrint("Problem with the file : " + filenameX, 'error')
                        return minMax
                dsY=gdal.Open(filenameY, GA_ReadOnly)
                if dsX is None:
                        Global.logPrint("Problem with the file : " + filenameY, 'error')
                        return minMax

                nbBandsX=dsX.RasterCount
                nbBandsY=dsY.RasterCount
                if nbBandsX != nbBandsY:
                        Global.logPrint("Error: Files have different number of bands", 'error')
                        return minMax
                if nbBandsX != 1:
                        Global.logPrint("Error: Files should have number of bands equal one", 'error')
                        return minMax

                bandX = dsX.GetRasterBand(1)
                bandY = dsY.GetRasterBand(1)

                if (bandX.XSize != bandY.XSize) or (bandX.YSize != bandY.YSize):
                        Global.logPrint("Error: Rasters has not equal dimensions", 'error')
                        return minMax


                dataX=bandX.ReadAsArray(0,0,bandX.XSize, bandX.YSize).astype(numpy.float)
                dataY=bandY.ReadAsArray(0,0,bandY.XSize, bandY.YSize).astype(numpy.float)

                noDataValue=bandX.GetNoDataValue()
                if not noDataValue is None:
                        mask=(dataX != noDataValue)
                        dataX*=mask
                        dataY*=mask

                dataAbs=numpy.sqrt(dataX*dataX + dataY*dataY)

                # compute min/max values:
                m, M = self.computeMinMaxFromArray(dataAbs, 0.07)
                return [m,M]


        def fetchPolarAndIncidAngles(self, path, files, type):
                polarIncidAnlges={}
                for f in files:
                        if (type == "Reference") :
                                # fetch time from metadata:
                                filename = path + "/" + f
                                ds=gdal.Open(filename, GA_ReadOnly)
                                if ds is None:
                                        Global.logPrint("Problem with the file : " + filename, 'error')
                                        continue
                                metadata=ImageMetadataExtractor.fetchMetadata(ds)
                                #self.showMetadata(metadata)
                                polarIncidAnlges[f] = [None]*2

                                # extract polarizations :
                                keys=ImageMetadataExtractor.getPolarizationKeys();
                                values=ImageMetadataExtractor.fetchValues(metadata, keys)
                                if not len(values) is 0:
                                        key, polarization = values.items()[0]
                                        polarIncidAnlges[f][1] = polarization

                                # extract incidence angle :
                                keys=ImageMetadataExtractor.getIncidenceAngleKeys();
                                values=ImageMetadataExtractor.fetchValues(metadata, keys)
                                if not len(values) is 0:
                                        key, angle = values.items()[0]
                                        polarIncidAnlges[f][0] = angle


                return polarIncidAnlges


def windCondition(testTime, refTime):
        upperbound = testTime + datetime.timedelta(0.5)
        if (refTime >= testTime) and (refTime < upperbound):
                return True
        return False

def multiSpectralCondition(testTime, refTime):
        return standardTimeCondition(testTime, refTime, 0.5)

def standardTimeCondition(testTime, refTime, ndays):
        lowerbound=testTime - datetime.timedelta(ndays)
        upperbound=testTime + datetime.timedelta(ndays)
        if (refTime >= lowerbound) and (refTime < upperbound):
                return True
        return False


def compareTimes(currTime, refTime, varname=""):
    if varname:
       if "wind" in varname:
          return windCondition(currTime, refTime)
       else:
           return standardTimeCondition(currTime, refTime, 0.5)
    else:
        return standardTimeCondition(currTime, refTime, 1.0)
    return False


def compareWithRefTimes(currTime, refTimes, varname=""):
    isOk=not refTimes
    for rt in refTimes:
        rdt = datetime.datetime.strptime(rt,"%Y-%m-%d %H:%M:%S")
        isOk = compareTimes(currTime, rdt, varname)
        if isOk:
            break
    return isOk

def compareWithRefTimes2(currTime, refTimes, varname=""):
    isOk=not refTimes
    rdt=None
    for rt in refTimes:
        rdt = datetime.datetime.strptime(rt,"%Y-%m-%d %H:%M:%S")
        isOk = compareTimes(currTime, rdt, varname)
        if isOk:
            break
    return isOk, rdt


if __name__ == "__main__":

    filename="C:/VFomin_folder/PISE_project/Images/COSMO-SkyMed/4_Stripmap_HIMAGE/L1B/13F00446-1/CSKS4_DGM_B_HI_23_HH_LA_FF_20130318084536_20130318084543.h5"
    subsetNames=[]
    subsetDescriptions=[]
    ImageMetadataExtractor.hasSubsets(filename, subsetNames, subsetDescriptions)

    print subsetNames
    print subsetDescriptions

    inputFilename = subsetNames[0]
    print ImageMetadataExtractor.getSpatialExtentBBox(inputFilename)

##        ## TESTS :
##        path="H:/IT_Tools/Image_total"
##        inputRefDataPath=path + "/allAOIs"
##        inputAuxDataPathWind=path + "/Extractions_reduced"
##        inputAuxDataPathMS=path+"/Projected_images_MERIS"
##        outputPath=path + "/AOI_WIND_MS_classified"
##
##        inputRefImages=['ASA_WSM_1PNPDE20050604_015919_000001462037_00461_17048_5284.N1_reprojected_aoi.tif', 'ASA_WSM_1PNPDE20050605_141212_000001282037_00483_17070_5533.N1_reprojected_aoi.tif']
##        inputAuxImagesMS=['MER_RR__1PNBCM20060713_020119_000001152049_00232_22831_0001.N1_reprojected.tif', 'MER_RR__1PNBCM20060719_021240_000001212049_00318_22917_0003.N1_reprojected.tif']
##        inputAuxImagesWind=['CERSAT-GLO-CLIM_WIND_L4-OBS_FULL_TIME_SERIE_1385478987550_eastward_wind_20070501-000000.tif', 'CERSAT-GLO-CLIM_WIND_L4-OBS_FULL_TIME_SERIE_1385478987550_northward_wind_20110401-000000.tif', 'GLO-BLENDED_WIND_L4-OBS_FULL_TIME_SERIE_1385632487733_northward_wind_20081203-000000.tif']
##
##        ime = ImageMetadataExtractor()
##        #refTimes = ime.fetchTimes(inputRefDataPath,inputRefImages,"Reference")
##        #auxDataTimesMS = ime.fetchTimes(inputAuxDataPathMS,inputAuxImagesMS,"MultiSpectral")
##        #auxDataTimesWind = ime.fetchTimes(inputAuxDataPathWind,inputAuxImagesWind,"Wind")
##
##        refTimes={'ASA_WSM_1PNPDE20050604_015919_000001462037_00461_17048_5284.N1_reprojected_aoi.tif': datetime.datetime(2005, 6, 4, 1, 59, 20), 'ASA_WSM_1PNPDE20050605_141212_000001282037_00483_17070_5533.N1_reprojected_aoi.tif': datetime.datetime(2005, 6, 5, 14, 12, 13)}
##        auxDataTimesMS={'MER_RR__1PNBCM20060719_021240_000001212049_00318_22917_0003.N1_reprojected.tif': datetime.datetime(2006, 7, 19, 2, 12, 40), 'MER_RR__1PNBCM20060713_020119_000001152049_00232_22831_0001.N1_reprojected.tif': datetime.datetime(2005, 6, 5, 4, 1, 19)}
##        auxDataTimesWind={'GLO-BLENDED_WIND_L4-OBS_FULL_TIME_SERIE_1385632487733_northward_wind_20081203-000000.tif': datetime.datetime(2005, 6, 4, 0, 0), 'GLO-BLENDED_WIND_L4-OBS_FULL_TIME_SERIE_1385632487733_eastward_wind_20081203-000000.tif': datetime.datetime(2005, 6, 4, 0, 0), 'CERSAT-GLO-CLIM_WIND_L4-OBS_FULL_TIME_SERIE_1385478987550_northward_wind_20110401-000000.tif': datetime.datetime(2011, 4, 1, 0, 0), 'CERSAT-GLO-CLIM_WIND_L4-OBS_FULL_TIME_SERIE_1385478987550_eastward_wind_20110401-000000.tif': datetime.datetime(2011, 4, 1, 0, 0), 'CERSAT-GLO-CLIM_WIND_L4-OBS_FULL_TIME_SERIE_1385478987550_eastward_wind_20070501-000000.tif': datetime.datetime(2005, 6, 5, 0, 0)}
##
##
####        out=[]
####        for f in refTimes:
####                selectedFilesWind=ime.searchFiles(refTimes[f], auxTimesWind, windCondition)
####                selectedFilesWind = ime.appendMinMaxParams(inputAuxDataPathWind, selectedFilesWind)
####                print selectedFilesWind
####                selectedFilesMS=ime.searchFiles(refTimes[f], auxTimesMS, multiSpectralCondition)
####                out.append([f, refTimes[f], selectedFilesWind, selectedFilesMS])
##
##
##        out=[]
##        for f in refTimes:
##                selectedFilesWind=ime.searchFiles(refTimes[f], auxDataTimesWind, windCondition)
##                # Compute aux data parameters : min/max for wind
##                selectedFilesWind = ime.appendMinMaxParams(inputAuxDataPathWind, selectedFilesWind)
##
##                selectedFilesMS=ime.searchFiles(refTimes[f], auxDataTimesMS, multiSpectralCondition)
##
##               # classification: 0 - if there is only ref.data,
##               # 1 - if there is ref.data with aux. wind data,
##               # 2 - if there is ref.data with aux. wind & MS data
##                cls=0
##                if selectedFilesWind:
##                    cls=1
##                if selectedFilesWind and selectedFilesMS:
##                    cls=2
##                out.append([f, cls, refTimes[f], selectedFilesWind, selectedFilesMS])
##
##        ime.displayResult(out)
##
##        # copy data :
##        if not os.path.exists(outputPath):
##                os.makedirs(outputPath)
##
##        print "Output folder is ", outputPath
##        print "Copy data ..."
##        for o in out:
##                if o[1]>0:
##                        # create directory:
##                        os.chdir(outputPath)
##                        nd=o[2].strftime("%Y-%m-%d")
##                        #print nd
##                        os.makedirs(nd)
##                        dst=outputPath+'/'+nd
##                        # copy ref data file:
##                        srcfile=inputRefDataPath+'/'+o[0]
##                        shutil.copy(srcfile,dst)
##                        # copy aux.data files : wind
##                        for it in o[3]:
##                                srcfile=inputAuxDataPathWind+'/'+it
##                                shutil.copy(srcfile,dst)
##                        # copy aux.data files : ms (if exists)
##                        if o[4]:
##                                for it in o[4]:
##                                        srcfile=inputAuxDataPathWind+'/'+it
##                                        shutil.copy(srcfile,dst)
##
##
##        os.chdir(path)
##
##
##        #print "Ref.times : ", refTimes
##        #print "MS times : ", auxTimesMS
##        #print "Wind times : ", auxTimesWind

##        filename='resultInfo.csv'
##        outfile = open(filename, 'w')
##        # write the header :
##        outfile.write('Ref_file_name;Ref_file_time;Aux_data_file_wind;Aux_data_time_wind;Aux_data_file_MS;Aux_data_time_MS\n')
##        #print('Ref_file_name;Ref_file_time;Aux_data_file_wind;Aux_data_time_wind;Aux_data_file_MS;Aux_data_time_MS')
##        for o in out:
##               rfn=o[0]
##               rft=o[1]
##               adfw=''
##               adtw=''
##               for it in o[2]:
##                       adfw+=(it + ";")
##                       adtw+=(str(o[2][it]) + ";")
##               adfw=adfw[:-1]
##               adtw=adtw[:-1]
##
##               adfms=''
##               adtms=''
##               for it in o[3]:
##                       adfms+=(it + ";")
##                       adtms+=(str(o[3][it]) + ";")
##               adfms=adfms[:-1]
##               adtms=adtms[:-1]
##               outfile.write('%10s;%10s;%10s;%10s;%10s;%10s\n' % (rfn,rft,adfw,adtw,adfms,adtms))
##               #print('%10s;%10s;%10s;%10s;%10s;%10s\n' % (rfn,rft,adfw,adtw,adfms,adtms))
##
##        outfile.close()
##        if os.path.exists(filename):
##                print "File is written successfully"
##        else:
##                print "Failed to write the output file"



