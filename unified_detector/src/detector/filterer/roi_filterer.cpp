#include "roi_filterer.hpp"

IROIFilterer::IROIFilterer(std::shared_ptr<ROIDataHandler> data_handler,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode)
    : roi_data_handler(data_handler), 
      mode(mode),
      max_object_count(1), 
      modelVersion(model::Version::V1) {}
IROIFilterer::~IROIFilterer(){

}

void IROIFilterer::set_max_objects(int n){
    this->max_object_count = n;
}

void IROIFilterer::set_reference_frame(unified_detector::Rectangle reference_frame){
    this->reference_frame = std::make_shared<unified_detector::Rectangle>(reference_frame);
}

void IROIFilterer::setModelVersion(model::Version version){
    LOGV("VZ Debug: roi_filter setModelVersion model version :%d", version);
    this->modelVersion = version;
}

