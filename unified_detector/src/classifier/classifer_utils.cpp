/**
 * @file   classifer_utils.cpp
 * @brief  This file implements the classifer utils 
 *
 * This file implements classifer utils interface
 */
#include "classifier_utils.hpp"

bool classifier_utils::dump_output_raws(std::vector<float> output, char* file_name)
{
  LOGV("VZ Debug: Classifier dump started");
  std::string filename_str = file_name;
  #ifdef __android__
      std::string dump_path = "/sdcard/Download/classifier_dumps/" + filename_str + ".raw";
  #else
      std::string dump_path = "/sdcard/Download/classifier_dumps/" + filename_str + ".raw";
  #endif
  LOGV("VZ Debug: Classifier log dump_path: %s", dump_path.c_str());
  std::ofstream out (dump_path, std::ios::out | std::ios::binary);
  if(!out)
  {
    std::cout<<"Cannot open file";
    LOGV("VZ Debug: Not found file %s", dump_path.c_str());
  }
  LOGV("VZ Debug: Classifier log dumping output");
  out.write((char*)&output[0], output.size() * sizeof(float));
  out.close();
  return true;
}

std::vector<float> classifier_utils::custom_softmax(std::vector<float> feature)
{
	// to check DSP model
  if(feature.size()==0)
  {
    return feature;
  }
	float mx_val = 0.0;
	float sum = 0.0;
	mx_val = feature[0];
  int top1_raw_output_index = 0;
	for(int i =0;i<feature.size();i++)
	{
                if(i==833 )
                {
                  continue;
                }
		mx_val = std::max(feature[i],mx_val);
    if(feature[i]>feature[top1_raw_output_index])
    {
      top1_raw_output_index = i;
    }
	}
  #ifdef TESTING
    LOGV("Top1 class before sigmoid output are %d, %f",top1_raw_output_index,feature[top1_raw_output_index]);
	#endif
  for(int i=0;i<feature.size();i++)
	{
               if(i==833 )
                {
                  continue;
                }
		feature[i] = exp(feature[i]-mx_val);
		sum += feature[i];
	}
  int top1_class = 0;
  int top1_index = 0;
  if (std::abs(sum) > 1e-9) {
    for(int i=0;i<feature.size();i++)
    {
                  if(i==833 )
                  {
                    continue;
                  }
      feature[i] = (feature[i]/sum);
      if(feature[i]>feature[top1_index])
      {
        top1_index = i;
      }
    }
  }
  #ifdef TESTING
    LOGV("Top1 class after sigmoid output are %d, %f",top1_index,feature[top1_index]);
  #endif
  #ifdef TESTING
	  LOGV("Applied sigmoid for artefact classes");
  #endif

	return feature;
}
void validateBounds(unified_detector::Rectangle &obj_box, int img_w, int img_h){
  //In case the crop is not oriented correctly, adjust the roi
  // To avoid  assert in opencv library
  if(obj_box.right < obj_box.left){
    int tmp = obj_box.right;
    obj_box.right = obj_box.left;
    obj_box.left = tmp;
  }
  if(obj_box.bottom < obj_box.top){
    int tmp = obj_box.bottom;
    obj_box.bottom = obj_box.top;
    obj_box.top = tmp;
  }

  if (obj_box.left < 0){
      obj_box.left = 0;
  }
  if (obj_box.top < 0){
      obj_box.top = 0;
  }
  if (obj_box.right < 0){
      obj_box.right = 0;
  }
  if (obj_box.bottom < 0){
      obj_box.bottom = 0;
  }
      if (obj_box.left && obj_box.left >= img_w){
      obj_box.left = img_w - 1;
  }
      if (obj_box.top && obj_box.top >= img_h){
      obj_box.top = img_h - 1;
  }
      if (obj_box.right && obj_box.right >= img_w){
      obj_box.right = img_w - 1;
  }
      if (obj_box.bottom && obj_box.bottom >= img_h){
      obj_box.bottom = img_h - 1;
  }
}


cv::Mat classifier_utils::get_expanded_image_crop(const cv::Mat& image, unified_detector::Rectangle crop_rect, cv::Mat& crop_image) {
  LOGV("VZ Debug: get_expanded_image_crop original croprect l:%d, t:%d, r:%d, b:%d, iw:%d, ih:%d", \
  crop_rect.left, crop_rect.top, crop_rect.right, crop_rect.bottom, image.size().width, image.size().height);

  cv::Mat image_roi;
  if(image.cols == 0 || image.rows == 0)
    return image_roi;
  float width = crop_rect.right - crop_rect.left + 1;
  float height = crop_rect.bottom - crop_rect.top + 1;
  int width_inc = (int)(0.025 * width);
  int height_inc = (int)(0.025 * height);

  crop_rect.left = crop_rect.left - width_inc;
  crop_rect.right = crop_rect.right + width_inc;
  crop_rect.top = crop_rect.top - height_inc;
  crop_rect.bottom = crop_rect.bottom + height_inc;

  float aspect_ratio = float(crop_rect.right - crop_rect.left) /
                      float(crop_rect.bottom - crop_rect.top);
  if (aspect_ratio > 2.0) {
    int required_height = (crop_rect.right - crop_rect.left) / 2;
    int deficit = required_height - (crop_rect.bottom - crop_rect.top);
    crop_rect.bottom += (deficit * 0.5);
    crop_rect.top -= (deficit * 0.5);
    std::cout << "deficit:" << deficit << std::endl;
  }
  if (aspect_ratio < 0.5) {
    int required_width = (crop_rect.bottom - crop_rect.top) / 2;
    int deficit = required_width - (crop_rect.right - crop_rect.left);
    crop_rect.right += (deficit * 0.5);
    crop_rect.left -= (deficit * 0.5);
    std::cout << "deficit:" << deficit << std::endl;
  }
  if (crop_rect.right >= image.cols) {
    crop_rect.right = image.cols - 1;
  }
  if (crop_rect.bottom >= image.rows) {
    crop_rect.bottom = image.rows - 1;
  }
  if (crop_rect.left < 0) {
    crop_rect.left = 0;
  }
  if (crop_rect.top < 0) {
    crop_rect.top = 0;
  }


  validateBounds(crop_rect, image.size().width, image.size().height);
  LOGV("VZ Debug: get_expanded_image_crop corrected croprect l:%d, t:%d, r:%d, b:%d, iw:%d, ih:%d", \
  crop_rect.left, crop_rect.top, crop_rect.right, crop_rect.bottom, image.size().width, image.size().height);
  cv::Rect roi(crop_rect.left, crop_rect.top,
              (crop_rect.right - crop_rect.left + 1),
              (crop_rect.bottom - crop_rect.top + 1));
  image_roi = image(roi);
  image_roi.copyTo(crop_image);
  LOGV("VZ Debug: get_expanded_image_crop completed");

  return crop_image;
}

cv::Mat classifier_utils::get_top_cropped(cv::Mat image){
  int width = image.cols;
  int height = image.rows;

  // int new_left = margin_left * width -1;
  // int new_top = margin_top * height;
  // int new_right = width - margin_right * width -1;
  // int new_bottom = height - margin_bottom * height -1;
  // Rectangle crop_rect(new_left, new_top, new_right, new_bottom);

  float crop_height_from_top = 0.65;
  float crop_width_from_left = 0.9;
  float crop_width_from_right = 0.9;
  unified_detector::Rectangle loc(0,0,width-1, height-1);

  int new_left = loc.right - (loc.right - loc.left) * crop_width_from_left; 
  int new_top = loc.top;
  int new_bottom = loc.top + (loc.bottom - loc.top) * crop_height_from_top;
  int new_right = loc.left + (loc.right - loc.left) * crop_width_from_right;

  LOGV("VZ Debug: Image cropped from (%d, %d, %d, %d) to (%d, %d, %d, %d)", loc.left, loc.top, loc.right, loc.bottom, new_left, new_top, new_right, new_bottom);

  unified_detector::Rectangle new_loc(new_left, new_top, new_right, new_bottom);

  cv::Mat image_top_crop;
  get_expanded_image_crop(image, new_loc, image_top_crop);
  // cv::Mat image_top_crop = get_expanded_image_crop(image, crop_rect);
  // cv::Mat image_top_crop = image({new_left, new_top, new_right - new_left, new_bottom - new_top}).clone();
  return image_top_crop;
}

void classifier_utils::assign_scene_category(std::vector<Entity>& entities){
  for (auto& e : entities) {
    std::string display_tag = e.display_tag;
#ifdef TESTING
    std::stringstream ss(e.display_tag);
    ss >> display_tag;
#endif
    //Assigning SceneInfo Enum
    e.scene_category = get_scene_info_from_tag(display_tag);
  }
}

std::vector<float> classifier_utils::apply_custom_softmax(std::vector<float> feature)
{
	// to check DSP model
  if(feature.size()==0)
  {
    return feature;
  }
	float mx_val = 0.0;
	float sum = 0.0;
	mx_val = feature[0];
  int top1_raw_output_index = 0;
	for(int i =0;i<feature.size();i++)
	{
                if(i==833)
                {
                  continue;
                }
		mx_val = std::max(feature[i],mx_val);
    if(feature[i]>feature[top1_raw_output_index])
    {
      top1_raw_output_index = i;
    }
	}
  #ifdef TESTING
    LOGV("Top1 class before sigmoid output are %d, %f",top1_raw_output_index,feature[top1_raw_output_index]);
	#endif
  for(int i=0;i<feature.size();i++)
	{
               if(i==833)
                {
                  continue;
                }
		feature[i] = exp(feature[i]-mx_val);
		sum += feature[i];
	}
  int top1_class = 0;
  int top1_index = 0;
  if (std::abs(sum) > 1e-9) {
    for(int i=0;i<feature.size();i++)
    {
                  if(i==833)
                  {
                    continue;
                  }
      feature[i] = (feature[i]/sum);
      if(feature[i]>feature[top1_index])
      {
        top1_index = i;
      }
    }
  }
  #ifdef TESTING
    LOGV("Top1 class after sigmoid output are %d, %f",top1_index,feature[top1_index]);
  #endif
  #ifdef TESTING
	  LOGV("Applied sigmoid for artefact classes");
  #endif

	return feature;
}

std::vector<Entity> classifier_utils::remove_unsupported_classes(std::vector<Entity> entities){
  std::vector<Entity> filtered_entities;
  std::unordered_set<int> cls_supported_ids = {100,101,102,103,104,105};
    for(int i =0;i<entities.size();i++){
        if(cls_supported_ids.count(static_cast<int>(entities[i].scene_category))){
          filtered_entities.push_back(entities[i]);
        }
    }
  return filtered_entities;

}