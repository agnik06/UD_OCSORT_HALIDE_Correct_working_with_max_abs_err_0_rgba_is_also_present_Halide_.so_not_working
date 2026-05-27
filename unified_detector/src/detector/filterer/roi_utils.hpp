#ifndef __ROI_UTILS__
#define __ROI_UTILS__

#include "framework/bounding_box.hpp"
#include "framework/rectangle.hpp"
#include "logger/logger.hpp" 
#include "rectangle_utils.hpp"
#include <opencv2/opencv.hpp>


//Static Class
class ROIUtils{
public:
    static float get_overlap_between_boxes(unified_detector::Rectangle box1, unified_detector::Rectangle box2);
    static float get_iou_between_boxes(unified_detector::Rectangle box1, unified_detector::Rectangle box2);

    static std::vector<unified_detector::BoundingBox> remove_overlapping_boxes(std::vector<unified_detector::BoundingBox> secondary_boxes, const std::vector<unified_detector::BoundingBox> primary_boxes, float iou_threshold=0.8f);
    static std::vector<unified_detector::BoundingBox> remove_smaller_boxes(std::vector<unified_detector::BoundingBox> boxes, int img_w, int img_h, float ratio_threshold, int num_boxes = 1);
    static std::vector<unified_detector::BoundingBox> remove_bigger_boxes(std::vector<unified_detector::BoundingBox> boxes, int img_w, int img_h);
    static std::vector<unified_detector::BoundingBox> remove_smaller_boxes_for_Specific_categories(std::vector<unified_detector::BoundingBox> boxes, int img_w, int img_h, int num_boxes = 1);

    static std::vector<unified_detector::BoundingBox> sort_boxes(std::vector<unified_detector::BoundingBox> boxes, std::vector<float> scores);

    static cv::Mat getGaussianMatrix_HQ(cv::Mat image_src, int width, int height, double var, int mode);
    static std::vector<unified_detector::BoundingBox> get_top_boxes(std::vector<unified_detector::BoundingBox>& boxes, int img_w, int img_h, int num_boxes = 3);

    static double getVariance_HQ(cv::Mat image_src, int width, int height, double var /*= 0.0*/, int mode);
    static double getReverseGaussianScore_HQ(double variance, int width, int height, int left, int top, int right, int bottom);
    static std::vector<unified_detector::BoundingBox> get_top_boxes_HQ( std::vector<unified_detector::BoundingBox>& boxes, int img_w, int img_h, cv::Mat image_src,int num_boxes=3);

    //ROI-Tag utils
    static int check_overlap_box(unified_detector::Rectangle r1,unified_detector::Rectangle r2,float area_threshold);
    static bool isCentered(unified_detector::Rectangle roi_location, int width, int height, float margin=0.3f);
    static std::vector<unified_detector::BoundingBox> soft_non_max_suppression_fast(std::vector<unified_detector::BoundingBox>& boxes,
                                                  int imgW, int imgH);
    static std::vector<unified_detector::BoundingBox> remove_other_boxes_in_category_box(std::vector<unified_detector::BoundingBox> boxes,std::vector<std::string> &categories,std::vector<float> &thresholds);
};



#endif //__ROI_UTILS__