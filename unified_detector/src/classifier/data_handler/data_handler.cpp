#include "data_handler.hpp"
#include <fstream>
#include "core/logger/logger.hpp"
#include "data/data_factory.hpp"

DataHandler::DataHandler() { 
    // json json_data = json::parse(json_str);

  json json_data = DataFactory::get_data(model::Version::V1);
// #ifdef TESTING
//   std::ifstream infile("/sdcard/data.json");
//   if (infile.good()) {
//     LOGV("AJ Debug: Using parameters from /sdcard/data.json");
//     infile >> json_data;
//   }
// #endif
  LOGV("get_data_called for version=%d",static_cast<int>(model::Version::V1));
  initialize(json_data);
}

DataHandler::DataHandler(json json_data) { 
  initialize(json_data);
}

DataHandler::~DataHandler() {}

void DataHandler::initialize(json json_data) {
  json disp_tags = json_data["display"];
  int display_tag_id = 0;
  for (json &ent : disp_tags) {
    std::string display_tag = ent["tag_name"];
    // int display_tag_id = ent["tag_id"];

    this->display_tags.push_back(display_tag);
    this->display_tag_id_mapping[display_tag] = display_tag_id;
    this->display_thresholds[display_tag] = ent["threshold"];
    this->display_base_mapping[display_tag_id] = std::vector<int>();
    this->display_type[display_tag] = ent["type"];
    this->display_cluster[display_tag] = ent.value("cluster", display_tag);
    this->disp_priority_index[display_tag] = ent.value("priority_index", 1);

    display_tag_id++;
  }

  json baseline_tags = json_data["baseline"];
  for (json &ent : baseline_tags) {
    int display_id = display_tag_id_mapping[ent["display_tag"]];
    int base_id = ent["tag_id"];
    std::string base_tag = ent["tag_name"];

    this->base_tags.push_back(base_tag);
    this->base_tag_id_mapping[base_tag] = base_id;
    this->base_display_mapping[base_id] = display_id;
    this->base_suppress_threshold[base_id] = ent["suppress_th"];
    this->display_base_mapping[display_id].push_back(base_id);

    for (json add_disp : ent["add_display_tags"]) {
      int add_display_id = display_tag_id_mapping[add_disp];
      this->display_base_mapping[add_display_id].push_back(base_id);
    }
  }
}

/**
 * @brief Returns Baseline tag of given baseline id
 * @param[in]	base_id Baseline id
 * @return  Baseline tag
 */
std::string DataHandler::getBaseTag(int base_id) {
  if (base_id >= base_tags.size()) {
    LOGV(
        "VZ Debug DH: Problem in  accessing %d location from array with length "
        "%d",
        base_id, (int)base_tags.size());
  }
  else if(base_id<0){
    LOGV("CD debug: base_id<0 returning null,base_id=%d",base_id);
  }
  return base_tags[base_id];
}

/**
 * @brief Returns Display tag of given display id
 * @param[in]	display_id Display id
 * @return  Display tag
 */
std::string DataHandler::getDisplayTag(int display_id) {
  if (display_id >= display_tags.size()) {
    LOGV(
        "VZ Debug DH: Problem in  accessing %d location from array with length "
        "%d",
        display_id, (int)display_tags.size());
  }
  else if(display_id<0){
    LOGV("CD Debug: Display_id_negetive=%d",display_id);
  }
  return display_tags[display_id];
}

/**
 * @brief Returns Baseline Id of given baseline tag
 * @param[in]	base_tag Baseline tag
 * @return  Baseline Id
 */
int DataHandler::getBaseId(std::string base_tag) {
  if (base_tag_id_mapping.count(base_tag) > 0) {
    return base_tag_id_mapping[base_tag];
  } else {
    LOGV("VZ Debug DH: Key base_tag=%s not found in base_tag_id_mapping",
         base_tag.c_str());
    return -1;
  }
}

/**
 * @brief Returns Display Id of given display tag
 * @param[in]	display_id Display id
 * @return  Display Id
 */
int DataHandler::getDisplayId(std::string display_tag) {
  if (display_tag_id_mapping.count(display_tag) > 0) {
    // LOGV("CD Debug: Display_tag_requested%s",display_tag.c_str());
    return display_tag_id_mapping[display_tag];
  } else {
    LOGV("VZ Debug DH: Key display_tag=%s not found", display_tag.c_str());
    return -1;
  }
}

/**
 * @brief Returns Baseline ids that belong to given Display id
 * @param[in]	display_id Display Id
 * @return  Baseline Ids <Vector>
 */
std::vector<int> DataHandler::getBaseIdsFromDisplay(int display_id) {
  std::vector<int> base_ids;
  if (display_base_mapping.count(display_id) > 0)
    base_ids = display_base_mapping[display_id];
  else {
    LOGV("VZ Debug DH: Key display_id=%d not found", display_id);
  }
  return base_ids;
}

/**
 * @brief Returns Baseline ids that belong to given Display Tag
 * @param[in]	display_tag Display Tag
 * @return  Baseline Ids <Vector>
 */
std::vector<int> DataHandler::getBaseIdsFromDisplay(std::string display_tag) {
  std::vector<int> base_ids;
  int display_id = getDisplayId(display_tag);
  return getBaseIdsFromDisplay(display_id);
}

/**
 * @brief Returns Display id of given Baseline Id
 * @param[in]	base_id Baseline Id
 * @return  Display Id
 */
int DataHandler::getDisplayIdFromBase(int base_id) {
  if (base_display_mapping.count(base_id) > 0)
    return base_display_mapping[base_id];
  else {
    LOGV("VZ Debug DH: Key display_id=%d not found in base_display_mapping",
         base_id);
  }
  return 0;
}

/**
 * @brief Returns Display id of given Baseline Tag
 * @param[in]	base_tag Baseline Tag
 * @return  Display Id
 */
int DataHandler::getDisplayIdFromBase(std::string base_tag) {
  int base_id = getBaseId(base_tag);
  return getDisplayIdFromBase(base_id);
}

/**
 * @brief Returns Display Tag of given Baseline Id
 * @param[in]	base_id Baseline Id
 * @return  Display Tag
 */
std::string DataHandler::getDisplayTagFromBase(int base_id) {
  int display_id = getDisplayIdFromBase(base_id);
    if (display_id >= 0) {
      return getDisplayTag(display_id);
    } else {
      return getDisplayTag(0); // Return first tag in case the tag id is negative
    }
}

/**
 * @brief Returns Display Tag of given Baseline Tag
 * @param[in]	base_tag Baseline Tag
 * @return  Display Tag
 */
std::string DataHandler::getDisplayTagFromBase(std::string base_tag) {
  return getDisplayTag(getDisplayIdFromBase(base_tag));
}

/**
 * @brief Returns Threshold corresponding to given display tag
 * @param[in]	display_tag Display Tag
 * @return  Threshold
 */
float DataHandler::getDisplayThreshold(std::string display_tag) {
  if (display_thresholds.count(display_tag) > 0)
    return display_thresholds[display_tag];
  else {
    LOGV(
        "VZ Debug DH: Key display_tag=%s not found in display_thresholds, so "
        "threshold set to 1",
        display_tag.c_str());
    return 1.0f;
  }
}

/**
 * @brief Returns Threshold corresponding to given display id
 * @param[in]	display_id Display Id
 * @return  Threshold
 */
float DataHandler::getDisplayThreshold(int display_id) {
  std::string display_tag = getDisplayTag(display_id);
  return getDisplayThreshold(display_tag);
}

/**
 * @brief Returns total number of baseline classes
 * @param[in]	void
 * @return  No. of baseline classes
 */
int DataHandler::getBaseCount() { return base_tags.size(); }

/**
 * @brief Returns total number of display classes
 * @param[in]	void
 * @return  No. of display classes
 */
int DataHandler::getDisplayCount() { return display_tags.size(); }

/**
 * @brief Returns if the given tag is Foreground category
 * @param[in]	roi_tag Display tag of the roi
 * @return  true if roi_tag is foreground
 */
bool DataHandler::isForegroundCategory(std::string roi_tag) {
  return (this->display_type[roi_tag] == "FG");
}

/**
 * @brief Returns if the given tag is Backup category
 * @param[in]	display_tag Display Tag
 * @return  true if display_tag belongs to Backup categories
 */
bool DataHandler::isBackupCategory(std::string display_tag) {
  return (display_tag == "Scenery" || display_tag == "Greenery" ||
          display_tag == "Group_Mountain" || display_tag == "Sky_new" || this->disp_priority_index[display_tag] == 2);
}

/**
 * @brief Returns the area threshold of given roi_tag
 * @param[in]	roi_tag Display tag of the roi
 * @return  Area threshold of roi
 */
float DataHandler::getROIAreaThreshold(std::string roi_tag) {
  if (roi_tag == "person") {
    return 0.1;
  } else {
    return 0.15;
  }
}

/**
 * @brief Inplace function to suppress probabilities of pre-defined baseline
 * classes
 * @param[in]	&probs reference to softmax output of the classifier
 * @return  void
 */
void DataHandler::suppressBaseProbs(std::vector<float> &probs) {
  LOGV("Before suppressBaseProbs");
  float probs_sum = 0.0;
  /*std::vector<int> snowindex = {997,15,13,492,114,687,993,488,650};
  for(int i=0;i<snowindex.size();i++)
  {
    LOGV("Before Suppress: %d, %f",i,probs[snowindex[i]]);
  }*/
  for (int i = 0; i < probs.size(); i++) {
    probs[i] = (probs[i] > base_suppress_threshold[i] ? probs[i] : 0.0);
    probs_sum += probs[i];
  }
  for (int i = 0; i < probs.size(); i++) {
    if (std::abs(probs_sum) > 1e-9) {
      probs[i] = probs[i] * 1.0 / probs_sum;
    }
  }
  /*for(int i=0;i<snowindex.size();i++)
  {
    LOGV("After Suppress and divide by %f : %d, %f",probs_sum,i,probs[snowindex[i]]);
  }*/
}

/**
 * @brief Get the ratio of area of as primary region to area of image. If the
 * object is not present in primary region, its threshold will be scaled (>1).
 * Change the value to define Primary region.
 * @param[in]	void
 * @return  Ratio (value) of central region's area to image's area to be
 * considered Primary region
 */
float DataHandler::getPrimaryRegionAreaRatio() { return 0.3f; }

/**
 * @brief If the object is not present in primary region, then the threshold of
 * display class will be scaled.
 * @param[in]	void
 * @return  Scaling factor of the display class threshold
 */
float DataHandler::getThresholdScalingFactor() { return 2.0f; }


std::string DataHandler::getDisplayClusterTag(std::string display_tag){
  return this->display_cluster[display_tag];
}
std::string DataHandler::getDisplayClusterTag(int display_id){
  std::string display_tag = getDisplayTag(display_id);
  return this->display_cluster[display_tag];
}

int DataHandler::getDisplayTagPriority(std::string display_tag){
  return this->disp_priority_index[display_tag];
}
int DataHandler::getDisplayTagPriority(int display_id){
  std::string display_tag = getDisplayTag(display_id);
  return this->disp_priority_index[display_tag];
}