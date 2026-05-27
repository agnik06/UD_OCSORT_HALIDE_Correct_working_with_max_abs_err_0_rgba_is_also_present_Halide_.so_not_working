#include "scene_info_mapper.hpp"

unified_detector::UnifiedDetector::TagCategory get_scene_info_from_tag(std::string scene_tag) {
  unified_detector::UnifiedDetector::TagCategory scene;
  if (scene_tag == "Food") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_FOOD;
  } else if (scene_tag == "Animal") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_ANIMAL;
  } else if (scene_tag == "Text") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_TEXT;
  } else if (scene_tag == "Person") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_PERSON;
  } else if (scene_tag == "Flower" || scene_tag == "Flowers") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_FLOWER;
  } else if (scene_tag == "Flower_BG") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_FLOWER;
  } else if (scene_tag == "Flower_FG") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_FLOWER;
  } else if (scene_tag == "Tree") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_TREE;
  } else if (scene_tag == "Snow") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SNOW;
  } else if (scene_tag == "Mountain") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_MOUNTAIN;
  } else if (scene_tag == "Waterside") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_WATERSIDE;
  } else if (scene_tag == "Beach") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_BEACH;
  } else if (scene_tag == "Waterfall") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_WATERFALL;
  } else if (scene_tag == "Scenery") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SCENERY;
  } else if (scene_tag == "Nightview") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_NIGHTVIEW;
  } else if (scene_tag == "HomeIndoor") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_HOMEINDOOR;
  } else if (scene_tag == "Sunrise/Sunset" || scene_tag == "Sunrise_Sunset") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SUNSET_SUNRISE;
  } else if (scene_tag == "Mountain_Fall") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_MOUNTAIN_FALL;
  } else if (scene_tag == "Mountain_Green") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_MOUNTAIN_GREEN;
  } else if (scene_tag == "Greenery") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_GREENERY;
  }

  else if (scene_tag == "Baby") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_BABY;
  } else if (scene_tag == "Cat") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_CAT;
  } else if (scene_tag == "City") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_CITY;
  } else if (scene_tag == "Clothing") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_CLOTHING;
  } else if (scene_tag == "Dog") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_DOG;
  } else if (scene_tag == "Drink") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_DRINK;
  } else if (scene_tag == "People") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_PEOPLE;
  } else if (scene_tag == "Restaurant_Indoor") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_RESTAURANT_INDOOR;
  } else if (scene_tag == "Shoe_disp") {
	scene = unified_detector::UnifiedDetector::TagCategory::TAG_SHOE_DISP;
  } else if (scene_tag == "Shoe_on") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SHOE_ON;
  } else if (scene_tag == "Sky_Blue") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SKY_BLUE;
  } else if (scene_tag == "Sky_Grey") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SKY_GREY;
  } else if (scene_tag == "Skyscraper") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_SKYSCRAPER;
  } else if (scene_tag == "Stage") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_STAGE;
  } else if (scene_tag == "Tree_Green") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_TREE_GREEN;
  } else if (scene_tag == "Vehicle") {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_VEHICLE;
  } 
   else {
    scene = unified_detector::UnifiedDetector::TagCategory::TAG_INVALID;
  }
  return scene;
}
