#include "classifier_with_flip.hpp"

ClassifierWithFlip::ClassifierWithFlip(std::shared_ptr<BaseClassifier> classifier)
    :ClassifierDecorator(classifier){

}
ClassifierWithFlip::~ClassifierWithFlip(){

}

std::vector<float> ClassifierWithFlip::classify(cv::Mat image){
    std::vector<float> features = ClassifierDecorator::classify(image);
    
    cv::Mat image_flipped = get_image_flipped(image);
    std::vector<float> features_flipped_image = ClassifierDecorator::classify(image_flipped);
    #ifdef TESTING
        LOGV("Cached classifier from nnet from flip");
    #endif
    //Element-wise average values
    for(int i=0; i<features.size(); i++){
        features[i] = (features[i] + features_flipped_image[i]) / 2.0f;
    }
    return features;
}


cv::Mat ClassifierWithFlip::get_image_flipped(cv::Mat image){
    cv::Mat flipped_image;
    cv::flip(image, flipped_image, 1);
    return flipped_image;
}
