/**
 * @file   rectangle_utils.cpp
 * @brief  This file implements different utilities for ROI box
 *
 * Different utilities are implemented for ROI box
 */
#include "rectangle_utils.hpp"


/**
 * @brief compute_intersection computes the intersection between two rectangle boxes
 * 
 * @param[in] box1, box2 refers to boxes between which intersection is calculated
 *
 * @return int representing the area of intersection
 */
int RectangleUtils::compute_intersection(unified_detector::Rectangle& box1, unified_detector::Rectangle& box2){
    int left = std::max(box1.left, box2.left);
    int top = std::max(box1.top, box2.top);
    int right = std::min(box1.right, box2.right);
    int bottom = std::min(box1.bottom, box2.bottom);
    return unified_detector::Rectangle(left, top, right, bottom).area();
}
