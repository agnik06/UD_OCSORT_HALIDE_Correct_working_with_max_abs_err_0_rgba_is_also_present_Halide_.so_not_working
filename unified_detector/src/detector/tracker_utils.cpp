/**
 * @file   tracker_utils.cpp
 * @brief  This file implements the tracker utils 
 *
 * This file implements tracker utils hederfile methods
 */
 #include "tracker_utils.hpp"
 #define PETFACE_ID 8
 #define PET_ID 11
double tracker_utils::calculateIoU(const unified_detector::Rectangle& box1, const unified_detector::Rectangle& box2) {
  int x1 = std::max(box1.left, box2.left);
  int y1 = std::max(box1.top, box2.top);
  int x2 = std::min(box1.right , box2.right);
  int y2 = std::min(box1.bottom, box2.bottom);
  if (x2 <= x1 || y2 <= y1) {
      return 0.0;
  }
  int intersectionArea = (x2 - x1) * (y2 - y1);
  // box1.width * box1.height + box2.width *box2.height - intersectionArea
  int totalArea = (box1.right-box1.left+1) * (box1.bottom-box1.top+1) + (box2.right-box2.left+1) * (box2.bottom-box2.top+1) - intersectionArea;
  return static_cast<double>(intersectionArea) / totalArea;
}

float tracker_utils::get_overlap_between_boxes(unified_detector::Rectangle box1, unified_detector::Rectangle box2) {
  //TOD0: Modify code to match get_iou_between_boxes style
  float box1_x1 = (float)(box1.left);
  float box1_y1 = (float)(box1.top);
  float box1_x2 = (float)(box1.right);
  float box1_y2 = (float)(box1.bottom);

  float box2_x1 = (float)(box2.left);
  float box2_y1 = (float)(box2.top);
  float box2_x2 = (float)(box2.right);
  float box2_y2 = (float)(box2.bottom);

  float x5 = std::max(box1_x1, box2_x1);
  float y5 = std::max(box1_y1, box2_y1);
  float x6 = std::min(box1_x2, box2_x2);
  float y6 = std::min(box1_y2, box2_y2);
  if ((x6 >= x5) && (y6 >= y5)) {
      float den = (float)((box2_x2 - box2_x1 + 1) * (box2_y2 - box2_y1 + 1));
      float overlap = ((float)((x6 - x5 + 1) * (y6 - y5 + 1))) / den;
      return overlap;
  }
  return 0.0;
}

unified_detector::Rectangle tracker_utils::update_smother_box(const unified_detector::Rectangle& newBox,unified_detector::Rectangle& smother_box){
  double alpha = 0.2;
  smother_box.left = alpha * newBox.left + (1 - alpha) * smother_box.left;
  smother_box.top = alpha * newBox.top + (1 - alpha) * smother_box.top;
  smother_box.right = alpha * newBox.right + (1 - alpha) * smother_box.right;
  smother_box.bottom = alpha * newBox.bottom + (1 - alpha) * smother_box.bottom;
  return smother_box;
}
