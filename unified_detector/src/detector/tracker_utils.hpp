/**
 * @file   tracker_utils.hpp
 * @brief  This file implements the tracker utils Interface
 *
 * This file implements tracker utils to be used by end api's
 */
 #ifndef __TRACKER_UTILS__
 #define __TRACKER_UTILS__
 #include <vector>
 #include <functional>
 #include "framework/bounding_box.hpp"
 namespace tracker_utils{
   double calculateIoU(const unified_detector::Rectangle& box1, const unified_detector::Rectangle& box2);
   float get_overlap_between_boxes(unified_detector::Rectangle box1, unified_detector::Rectangle box2);
   unified_detector::Rectangle update_smother_box(const unified_detector::Rectangle& newBox,unified_detector::Rectangle& smother_box);
 };
 #endif //__TRACKER_UTILS__