#ifndef __PET_ROI_FILTERER__
#define __PET_ROI_FILTERER__

#include "base_roi_filterer.hpp"

class PetROIFilterer: public BaseROIFilterer{
public:
    PetROIFilterer(std::shared_ptr<ROIDataHandler> data_handler, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    ~PetROIFilterer();
    std::vector<unified_detector::BoundingBox> filter(std::vector<unified_detector::BoundingBox> boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    std::vector<std::string> overlap_check_categories;
    std::vector<float> thresholds;

private:
    std::vector<unified_detector::BoundingBox> remove_petface_over_person_boxes(std::vector<unified_detector::BoundingBox> boxes);
    std::vector<unified_detector::BoundingBox> ensure_petface_over_pet_boxes(std::vector<unified_detector::BoundingBox> boxes);
    std::vector<unified_detector::BoundingBox> ensure_petface_with_pet_boxes(std::vector<unified_detector::BoundingBox> boxes);
    bool is_small_petface_boxes(unified_detector::BoundingBox b1 ,  unified_detector::BoundingBox b2);
};

#endif //__PET_ROI_FILTERER__