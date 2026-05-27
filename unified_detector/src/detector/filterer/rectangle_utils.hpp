/**
 * @file   rectangle_utils.hpp
 * @brief  This file provides interfaces for different bounding box utilities
 *
 * This file provides interfaces for computing operations on bounding boxes detected
 */

#ifndef __RECTANGLE_UTILS__
#define __RECTANGLE_UTILS__

#include "framework/rectangle.hpp"
#include <vector>


#define IntVec std::vector<int>

class RectangleUtils{
    public:
        //computes the intersection between two rectangle boxes
        static int compute_intersection(unified_detector::Rectangle& box1, unified_detector::Rectangle& box2);
};

#endif //__RECTANGLE_UTILS__