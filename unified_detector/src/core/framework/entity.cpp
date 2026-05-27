/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "entity.hpp"

Entity::Entity(unified_detector::UnifiedDetector::TagCategory scene_category, std::string display_tag,
               float display_tag_score, unified_detector::Rectangle roi_location)
    : scene_category(scene_category),
      display_tag(display_tag),
      display_tag_score(display_tag_score),
      roi_location(roi_location),
      roi_score(0.0f),
      baseline_tag_score(0.0f),
      threshold(0.0f) {}

Entity::Entity(const Entity& other) {
  this->roi_tag = other.roi_tag;
  this->roi_location = {other.roi_location.left, other.roi_location.top,
                     other.roi_location.right, other.roi_location.bottom};
  this->roi_score = other.roi_score;

  this->baseline_tag = other.baseline_tag;
  this->baseline_tag_score = other.baseline_tag_score;

  this->display_tag = other.display_tag;
  this->display_tag_score = other.display_tag_score;

  this->scene_category = other.scene_category;
  this->threshold = other.threshold;
}


Entity::Entity(){
  this->roi_tag = "none";
  this->roi_location = {0, 0, 0, 0};
  this->roi_score = -1;

  this->baseline_tag = "none";
  this->baseline_tag_score = -1;
  this->display_tag = "none";
  this->display_tag_score = -1;

  this->scene_category = unified_detector::UnifiedDetector::TagCategory::TAG_INVALID;
  this->threshold = 1.0f;
}

Entity::Entity(std::string display_tag){
  this->roi_tag = "none";
  this->roi_location = {0, 0, 0, 0};
  this->roi_score = -1;

  this->baseline_tag = "none";
  this->baseline_tag_score = -1;
  this->display_tag = display_tag;
  this->display_tag_score = -1;

  this->scene_category = unified_detector::UnifiedDetector::TagCategory::TAG_INVALID;
  this->threshold = 1.0f;
}

Entity::~Entity() {}