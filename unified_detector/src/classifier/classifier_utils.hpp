/**
 * @file   classifier_utils.hpp
 * @brief  This file implements the classifer utils Interface
 *
 * This file implements image detector Interface to be used by end api's
 */
 #ifndef __CLASSIFIER_UTILS__
 #define __CLASSIFIER_UTILS__
 #include "core/logger/logger.hpp"
 #include "scene_info_mapper.hpp"
 #include<math.h>
 #include <opencv2/opencv.hpp>
 #include <framework/entity.hpp>
 #include <unordered_set>
 
 namespace classifier_utils{
   std::vector<float> custom_softmax(std::vector<float> feature);
   cv::Mat get_top_cropped(cv::Mat image);
   void assign_scene_category(std::vector<Entity>& entities);
   cv::Mat get_expanded_image_crop(const cv::Mat& image, unified_detector::Rectangle crop_rect, cv::Mat& crop_image);
   std::vector<float> apply_custom_softmax(std::vector<float> feature);
   std::vector<Entity> remove_unsupported_classes(std::vector<Entity> entities);
   bool dump_output_raws(std::vector<float> output, char* file_name);
 };
 #endif //__CLASSIFIER_UTILS__