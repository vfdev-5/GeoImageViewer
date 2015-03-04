#ifndef LIBEXPORT_H
#define LIBEXPORT_H


/*!
 * TODO :
 * 1) HistogramLayerRenderer & HistogramRendererView
 *   - problem with "all bands" : switch on/off -> histograms are not correctly shown
 *   - possibility to choose Bands to RGB mapping for multiband images
 *   - choice of RGB / GRAY mode for multiband images
 * 2) Image loading/display
 *   + create overview file .ovr
 *   - support complex imagery  => GDAL does not work correctly with CSK L1A => pending
 *   + support subdataset imagery (netcdf,hdf5)
 *   + Message error
 * 3) Rendering stage
 *   + measure time for loading/rendering a tile => at Release rendering time is correct
 * 4) Zoom widget
 *   - add zoom buttons & slider
 * 5) Tools
 *   + Menu with tool buttons
 *   - Possibility to draw/edit/select vector layers: point,line,polygon
 *   + Possibility to select a zone
 * 6) Layer browser
 *   + layers view
 *   + display image info
 *   - action to save 'Image' layer into a file
 *   - use view/model architecture
 * 7) Selection tool
 *   + select a region
 *   + create a layer from zone
 * 8) Layer pixel info
 *   - display pixel info : coordinates in pixels, geo, value
 * 9) Filters as plugins
 *   - default blur filter
 *   - default filter dialog
 * 10) Save/Load project
 *   - save workspace in xml file
 *   - load workspace from xml file
 *
 */

//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined LIB_EXPORT
#  define GIV_DLL_EXPORT __declspec(dllexport)
#elif (defined WIN32 || defined _WIN32 || defined WINCE)
#  define GIV_DLL_EXPORT __declspec(dllimport) // This helps to resolve the problem with Plugin link with AbstractTool plugin interface
#else
#  define GIV_DLL_EXPORT
#endif


#endif // LIBEXPORT_H
