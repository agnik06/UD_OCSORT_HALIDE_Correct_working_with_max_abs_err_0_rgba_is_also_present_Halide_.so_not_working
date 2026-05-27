#include "classifier_with_verified_top_crop.hpp"

ClassifierWithVerifiedTopCrop::ClassifierWithVerifiedTopCrop(std::shared_ptr<BaseClassifier> classifier, std::vector<int> baseline_ids_to_verify)
    :ClassifierDecorator(classifier),
     baseline_ids_to_verify(baseline_ids_to_verify),
     margin_left(0.1f),
     margin_top(0.0f),
     margin_right(0.1f),
     margin_bottom(0.35f){

}
ClassifierWithVerifiedTopCrop::~ClassifierWithVerifiedTopCrop(){

}

std::vector<float> ClassifierWithVerifiedTopCrop::classify(cv::Mat image){
    std::vector<float> features = ClassifierDecorator::classify(image);
    int top_baseline_id = std::distance(features.begin(), std::max_element(features.begin(), features.end()));

    #ifdef TESTING
        LOGV("Cached classifier from nnet from crop");
    #endif
    if (std::find(baseline_ids_to_verify.begin(), baseline_ids_to_verify.end(), top_baseline_id) != baseline_ids_to_verify.end()) {
        cv::Mat image_top_crop = classifier_utils::get_top_cropped(image);
        features = ClassifierDecorator::classify(image_top_crop);;
    }
    return features;
}
