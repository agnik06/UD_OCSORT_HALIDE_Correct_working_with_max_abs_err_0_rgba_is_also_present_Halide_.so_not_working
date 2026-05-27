/**
 * @file   detector_utils.hpp
 * @brief  This file implements the detector utils Interface
 *
 * This file implements image detector Interface to be used by end api's
 */
 #ifndef __DETECTOR_UTILS__
 #define __DETECTOR_UTILS__
 #include "unified_detector.h" 
 #include "framework/tensor.hpp"
 #include "framework/bounding_box.hpp"
 #include "framework/rectangle.hpp"
 #include "opencv2/opencv.hpp"
 #include <vector>
 #include <functional>
 #include "../utils/image_utils.hpp"
 #include "../utils/image_utils_neon.hpp"
 #include "core/logger/logger.hpp"
 #include <numeric>
 #include <unordered_set>
 #include <arm_neon.h>
 
 namespace detector_utils{
    cv::Mat preprocess_image(char* image_buffer,int width,int height,int target_width,int target_height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format); 
    std::vector<unified_detector::BoundingBox> correct_roi_boxes(std::vector<unified_detector::BoundingBox> result_boxes,int target_width,int target_height,const int angular_rotation_clockwise,int width,int height);
    std::vector<unified_detector::BoundingBox> get_boxes_from_2D_tensors(std::vector<std::vector<float> > roi, cv::Mat input_image);
    bool od_classes_detected(std::vector<unified_detector::BoundingBox> result_boxes);
    std::vector<unified_detector::BoundingBox> get_top_object_boxes(std::vector<unified_detector::BoundingBox> result_boxes,int target_width, int target_height);
    std::vector<unified_detector::BoundingBox> remove_smaller_boxes(std::vector<unified_detector::BoundingBox> entities, int img_w, int img_h, float ratio_threshold, int num_boxes);
    std::vector<unified_detector::BoundingBox> sort_entities(std::vector<unified_detector::BoundingBox> entities, std::vector<float> scores);
    std::vector<unified_detector::BoundingBox> get_top_boxes(std::vector<unified_detector::BoundingBox>& boxes, int img_w, int img_h, int num_boxes);
    std::vector<unified_detector::BoundingBox> remove_object_boxes(std::vector<unified_detector::BoundingBox> result_boxes);
    int resize_uniform(const cv::Mat &src, cv::Mat &dst, cv::Size dst_size);
    bool dump_output_raws(float* output, char* file_name, int n, int h, int w, int c);
    std::vector<unified_detector::BoundingBox> get_saliency_boxes_to_center(std::vector<unified_detector::BoundingBox> result_boxes,int width,int height);
    float calculateDistance(int x1, int y1, int x2, int y2);
    std::vector<std::pair<float, unified_detector::BoundingBox>> findNearestBoundingBox(const std::vector<unified_detector::BoundingBox>& detector_boxes,int imageWidth, int imageHeight);

    //std::unordered_set<int> od_class_ids = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    //int max_object_count = 1;
 };
 #endif //__DETECTOR_UTILS__