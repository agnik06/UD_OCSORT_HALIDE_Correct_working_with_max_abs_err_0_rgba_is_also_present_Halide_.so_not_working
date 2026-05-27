#ifndef __CLASSIFIER_INTERFACE__
#define __CLASSIFIER_INTERFACE__

#include <vector>
#include <opencv2/opencv.hpp>

class IClassifier{
public:
    virtual ~IClassifier(){};
    virtual std::vector<float> classify(cv::Mat image) = 0;
};

#endif //__CLASSIFIER_INTERFACE__