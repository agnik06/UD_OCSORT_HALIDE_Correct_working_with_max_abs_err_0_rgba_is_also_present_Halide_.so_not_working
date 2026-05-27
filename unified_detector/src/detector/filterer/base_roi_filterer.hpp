#ifndef __BASE_FILTERER__
#define __BASE_FILTERER__

#include "roi_filterer.hpp"
#include "core/logger/logger.hpp"

class BaseROIFilterer: public IROIFilterer{
public:
    BaseROIFilterer(std::shared_ptr<ROIDataHandler> data_handler, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    ~BaseROIFilterer();
    std::vector<unified_detector::BoundingBox> filter(std::vector<unified_detector::BoundingBox> boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    std::vector<std::string> overlap_check_categories;
    std::vector<float> thresholds;

protected:
    std::vector<unified_detector::BoundingBox> remove_invalid_boxes(std::vector<unified_detector::BoundingBox> boxes,int width,int height,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    std::vector<unified_detector::BoundingBox> remove_unsupported_categories(std::vector<unified_detector::BoundingBox> boxes);
    std::vector<unified_detector::BoundingBox> get_one_object_box(std::vector<unified_detector::BoundingBox> boxes,int width,int height);
};

#endif //__PETFACE_OVER_PERSON_ROI_FILTERER__