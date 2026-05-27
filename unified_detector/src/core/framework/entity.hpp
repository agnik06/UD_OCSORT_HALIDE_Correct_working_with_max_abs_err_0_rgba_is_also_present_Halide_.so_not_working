/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ENTITY_HPP
#define ENTITY_HPP
#include <string>
#include <vector>
#include "unified_detector.h"
#include "framework/rectangle.hpp"

class Entity {
 public:
  Entity(unified_detector::UnifiedDetector::TagCategory scene_category, std::string display_tag,
         float display_tag_score, unified_detector::Rectangle roi_location);
  Entity();
  Entity(const Entity& other);
  Entity(std::string display_tag);

  std::string roi_tag;
  float roi_score;
  unified_detector::Rectangle roi_location;

  std::string baseline_tag;
  float baseline_tag_score;

  std::string display_tag;
  float display_tag_score;

  unified_detector::UnifiedDetector::TagCategory scene_category;
  float threshold;

  ~Entity();
};
#endif