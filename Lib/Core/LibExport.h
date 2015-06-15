#ifndef LIBEXPORT_H
#define LIBEXPORT_H


/*!
 * TODO :
 * 1) HistogramImageRenderer & HistogramRendererView
 *   1 - problem with "all bands" : switch on/off -> histograms are not correctly shown
 *   2 - possibility to choose Bands to RGB mapping for multiband images
 *   2 - choice of RGB / GRAY mode for multiband images
 *   2 - complex images
 *   1 - when edit slider value, value should be limited by histogram values and not visible min/max range
 *   1 - use view/model architecture
 *
 *   !!! RECODE CONCURRENT USAGE OF ImageRenderer between AbstractRendererView and GeoImageItem
 *
 *
 * 2) Image loading/display
 *   + create overview file .ovr
 *   0 - support complex imagery  => GDAL does not work correctly with CSK L1A => pending
 *   + support subdataset imagery (netcdf,hdf5)
 *   + Message error
 * 3) Rendering stage
 *   + measure time for loading/rendering a tile => at Release rendering time is correct
 * 4) Zoom widget
 *   0 - add zoom buttons & slider
 * 5) Tools
 *   + Menu with tool buttons
 *   0 - Possibility to draw/edit/select vector layers: point,line,polygon
 *   + Possibility to select a zone
 * 6) Layer browser
 *   + layers view
 *   + display image info
 *   1 - action to save 'Image' layer into a file
 *      + use a simple image writer
 *      + geo info is not written
 *   2 - use view/model architecture
 * 7) Selection tool
 *   + select a region
 *   + create a layer from zone
 *   3 - tool to manually detect dark objects
 *      -- IHM : Circle to select dark object, slider to define a threshold, mouse wheel changes size of circle
 *      -- Threshold algorithm : blur + define threhold region (0.0, middle, histogram max value) + morpho close
 * 8) Layer pixel info
 *   1 - display pixel info : coordinates in pixels, geo, value
 * 9) Filters as plugins
 *   - Should apply filter on data ignoring NoDatValues
 *   + default blur filter
 *   + default filter dialog
 *   1 - 'lasso' filter to select a color uniform regions
 *   1 - algorithm to find threshold values
 *   1 - automatic segmentation filter based on histogram
 * 10) Save/Load project
 *   1 - save workspace in xml file
 *   1 - load workspace from xml file
 * 11) PropertyEditor
 *  - when variable is changed -> notify the property editor
 *
 */

//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined LIB_EXPORT
#  define GIV_DLL_EXPORT __declspec(dllexport)
#elif (defined WIN32 || defined _WIN32 || defined WINCE)
#  define GIV_DLL_EXPORT __declspec(dllimport) // This helps to resolve the problem with plugins link
#else
#  define GIV_DLL_EXPORT
#endif


#endif // LIBEXPORT_H
