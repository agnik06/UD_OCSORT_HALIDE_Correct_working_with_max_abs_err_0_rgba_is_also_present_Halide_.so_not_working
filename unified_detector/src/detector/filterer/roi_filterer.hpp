#ifndef __ROI_FILTERER_INTERFACE__
#define __ROI_FILTERER_INTERFACE__

#include "unified_detector.h"
#include "framework/bounding_box.hpp"
#include "../mapper/data/roi_data_handler.hpp"
#include "model_config.hpp"
#include "logger/logger.hpp"
#include <opencv2/opencv.hpp>
#include <memory>

class IROIFilterer{
public:
    IROIFilterer(std::shared_ptr<ROIDataHandler> data_handler,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    ~IROIFilterer();
    virtual std::vector<unified_detector::BoundingBox> filter(std::vector<unified_detector::BoundingBox> boxes, cv::Mat image, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode) = 0;
    void set_reference_frame(unified_detector::Rectangle reference_frame);
    void set_max_objects(int n);
    void setModelVersion(model::Version version);

protected:
    std::shared_ptr<ROIDataHandler> roi_data_handler;
    std::shared_ptr<unified_detector::Rectangle> reference_frame;
    int max_object_count = 1;
    model::Version modelVersion;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode;
    
};

#endif //__ROI_FILTERER_INTERFACE__