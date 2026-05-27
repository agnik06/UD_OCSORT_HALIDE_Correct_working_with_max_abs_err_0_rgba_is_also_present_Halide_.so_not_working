#ifndef __CLASSIFIER_WITH_VERIFIED_TOP_CROP__
#define __CLASSIFIER_WITH_VERIFIED_TOP_CROP__

#include "classifier_decorator.hpp"
#include "framework/rectangle.hpp"
#include "../classifier_utils.hpp"

class ClassifierWithVerifiedTopCrop: public ClassifierDecorator{
public:
    ClassifierWithVerifiedTopCrop(std::shared_ptr<BaseClassifier> clasifier, std::vector<int> baseline_ids_to_verify);
    ~ClassifierWithVerifiedTopCrop();
    std::vector<float> classify(cv::Mat image);

    

private:
    std::vector<int> baseline_ids_to_verify;
    //factor of width, height
    float margin_left;
    float margin_top;
    float margin_right;
    float margin_bottom;
    
    cv::Mat get_top_cropped(cv::Mat image);
};

#endif //__CLASSIFIER_WITH_VERIFIED_TOP_CROP__
