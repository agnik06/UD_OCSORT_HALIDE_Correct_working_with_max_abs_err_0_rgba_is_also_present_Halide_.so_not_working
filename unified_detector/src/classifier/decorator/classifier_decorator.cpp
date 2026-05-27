#include "classifier_decorator.hpp"

#include <assert.h>

ClassifierDecorator::ClassifierDecorator(std::shared_ptr<BaseClassifier> classifier)
    :classifier(classifier){

}
ClassifierDecorator::~ClassifierDecorator(){

}

std::vector<float> ClassifierDecorator::classify(cv::Mat image){
    assert(classifier != nullptr && "Expected initialized Base-Classifier");
    #ifdef TESTING
        LOGV("Cached classifier from nnet from textinvalid decorator");
    #endif
    return classifier->classify(image);
}


