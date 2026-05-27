#ifndef __TEXT_ROI_FILTERER__
#define __TEXT_ROI_FILTERER__

#include "base_roi_filterer.hpp"

class TextROIFilterer: public BaseROIFilterer{
public:
    TextROIFilterer(std::shared_ptr<ROIDataHandler> data_handler, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);
    ~TextROIFilterer();
    std::vector<unified_detector::BoundingBox> filter(std::vector<unified_detector::BoundingBox> boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode);

private:
std::vector<unified_detector::BoundingBox> get_one_text_box(std::vector<unified_detector::BoundingBox> boxes,int image_width,int image_height);
//bool get_text_box_based_on_large_area(unified_detector::BoundingBox box,int img_w, int img_h);
std::vector<unified_detector::BoundingBox> merge_text_boxes_inside_wine(std::vector<unified_detector::BoundingBox> boxes);
unified_detector::BoundingBox create_merged_low_conf_text_box(const std::vector<unified_detector::BoundingBox> boxes, bool text_box_added);
};

#endif //__TEXT_ROI_FILTERER__