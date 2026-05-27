#ifndef __CLASSIFIER_DECORATOR__
#define __CLASSIFIER_DECORATOR__

#include "../classifier.hpp"
#include "../base_classifier.hpp"
#include <memory>

class ClassifierDecorator: public BaseClassifier{
public:
    ClassifierDecorator(std::shared_ptr<BaseClassifier> classifier);
    ~ClassifierDecorator();
    std::vector<float> classify(cv::Mat image);

   
private:
    std::shared_ptr<BaseClassifier> classifier;
};

#endif //__CLASSIFIER_DECORATOR__
