#ifndef __CLASSIFIER_WITH_FLIP__
#define __CLASSIFIER_WITH_FLIP__

#include "classifier_decorator.hpp"

class ClassifierWithFlip: public ClassifierDecorator{
public:
    ClassifierWithFlip(std::shared_ptr<BaseClassifier> clasifier);
    ~ClassifierWithFlip();
    std::vector<float> classify(cv::Mat image);

private:
    cv::Mat get_image_flipped(cv::Mat image);
};

#endif //__CLASSIFIER_WITH_FLIP__
