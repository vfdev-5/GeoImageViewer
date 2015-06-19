
// Qt
#include <QVector>

// Opencv
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>


// Project
#include "HistogramThresholdFilterPlugin.h"
#include "Core/Global.h"
#include "Core/LayerUtils.h"


namespace Plugins
{

//******************************************************************************
/*!
 * \brief HistogramThresholdFilterPlugin::filter method to threshold input image using the histogram
 * \param inputMat
 * \return thresholded image or empty image
 *
 * Algorithm is intended to separate two peaks of each band histogram
 * 1) compute the histogram of the input image (or image channel)
 * 2) compute quantile values to reject near-zero histogram values -> more precise estimation of histogram peaks
 * 3) compute peaks of the histogram
 * 4) compute the threshold in between two peaks
 *
 *
 */
cv::Mat HistogramThresholdFilterPlugin::filter(const cv::Mat & inputMat) const
{


//    // 1) compute the band histograms of the input image
//    QVector<double> minValues;
//    QVector<double> maxValues;
//    QVector< QVector<double> > bandHistograms;
//    Core::computeNormalizedHistogram(inputMat,
//                                     minValues, maxValues, bandHistograms);


    // for each band :
    int nbBands = inputMat.channels();
    std::vector<cv::Mat> iChannels(nbBands);
    std::vector<cv::Mat> oChannels(nbBands);
    cv::split(inputMat, &iChannels[0]);
    for (int i=0;i<nbBands;i++)
    {


//        QVector<double> & bandHistogram = bandHistograms[i];
//        // 2) compute quantile values to reject near-zero histogram values -> more precise estimation of histogram peaks
//        int qminIndex, qmaxIndex;
//        if (!Core::computeQuantileMinMaxValue(bandHistogram, 0.0, 99.0, &qminIndex, &qmaxIndex))
//        {
//            SD_TRACE("HistogramThresholdFilterPlugin::filter : failed to compute quantile indices");
//            return cv::Mat();
//        }

        // 3) compute peaks of the histogram

        // TEST using EM directly on image band:
        cv::Mat data = iChannels[i];
        data.reshape(0, data.rows * data.cols);
        int nbClusters=2;
        cv::EM em(nbClusters);
        em.train(data);
        cv::Mat means = em.getMat("means");
        cv::Mat covs = em.getMat("covs");

        Core::printMat(means, "means");
        Core::printMat(covs, "covs");


        // threshold :
//        double t = minValues[i] + step * minLocs.first();
//        cv::Mat iChannels32F;
//        iChannels[i].convertTo(iChannels32F, CV_32F);
//        cv::threshold(iChannels32F, oChannels[i], t, 255, CV_THRESH_BINARY_INV);

    }

    cv::Mat out;
    cv::merge(oChannels, out);
    return out;
}

//******************************************************************************

}
