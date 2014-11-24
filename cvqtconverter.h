#ifndef CVQTCONVERTER_H
#define CVQTCONVERTER_H

#include <opencv2/core/core.hpp>
#include <QImage>
#include <QPixmap>

using namespace Qt;
using namespace cv;

class cvQTConverter
{
public:
    cvQTConverter();
    static QImage CvMat2QImg(const Mat& inMat);
};

#endif // CVQTCONVERTER_H
