/*
 * File:   data_handler.hpp
 * Author: Abhishek
 */
#ifndef __DATA_HANDLER__
#define __DATA_HANDLER__

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

class DataHandler {
 private:
  std::vector<std::string> base_tags;
  std::vector<std::string> display_tags;
  
  std::map<std::string, int> base_tag_id_mapping;
  std::map<std::string, int> display_tag_id_mapping;
  std::map<std::string, std::string> display_type;
  std::map<std::string, std::string> display_cluster;
  std::map<std::string, int> disp_priority_index;
  
  std::map<int, int> base_display_mapping;
  std::map<int, std::vector<int>> display_base_mapping;

  std::map<std::string, float> display_thresholds;
  std::map<int, float> base_suppress_threshold;
  void initialize(json json_data);

 public:
  DataHandler();
  DataHandler(json json_data);
  ~DataHandler();

  std::string getBaseTag(int base_id);
  std::string getDisplayTag(int display_id);
  int getBaseId(std::string base_tag);
  int getDisplayId(std::string display_tag);

  std::vector<int> getBaseIdsFromDisplay(int display_id);
  std::vector<int> getBaseIdsFromDisplay(std::string display_tag);
  int getDisplayIdFromBase(int base_id);
  int getDisplayIdFromBase(std::string base_tag);
  std::string getDisplayTagFromBase(int base_id);
  std::string getDisplayTagFromBase(std::string base_tag);

  float getDisplayThreshold(std::string display_tag);
  float getDisplayThreshold(int display_id);

  std::string getDisplayClusterTag(std::string display_tag);
  std::string getDisplayClusterTag(int display_id);
  int getDisplayTagPriority(std::string display_tag);
  int getDisplayTagPriority(int display_id);

  int getBaseCount();
  int getDisplayCount();

  bool isForegroundCategory(std::string roi_tag);
  bool isBackupCategory(std::string display_tag);
  float getROIAreaThreshold(std::string roi_tag);
  void suppressBaseProbs(std::vector<float> &probs);

  float getPrimaryRegionAreaRatio();
  float getThresholdScalingFactor();
};

#endif