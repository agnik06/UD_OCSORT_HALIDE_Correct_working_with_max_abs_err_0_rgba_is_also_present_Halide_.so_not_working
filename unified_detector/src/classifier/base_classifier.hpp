#ifndef __BASE_CLASSIFIER_WITH_TEXTINVALID__
#define __BASE_CLASSIFIER_WITH_TEXTINVALID__

#include "classifier.hpp"
#include "net/NNet.hpp"

class BaseClassifier: public IClassifier{
public:
    BaseClassifier(std::shared_ptr<NNet> net_classifier);
    BaseClassifier();
    ~BaseClassifier();
    std::vector<float> classify(cv::Mat image);
    std::vector<float> get_softmax(const char* layerName, int &b, int &c, int &h, int &w);

    std::vector<float> custom_softmax(std::vector<float> feature);

protected:
    std::shared_ptr<NNet> net_classifier;
};

#endif //__BASE_CLASSIFIER__
