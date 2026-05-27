/**
 * @file   detector_utils.cpp
 * @brief  This file implements the detector utils 
 *
 * This file implements detector utils interface
 */
#include "detector_utils.hpp"
#define MAX_OBJECT_COUNT 1

bool detector_utils::dump_output_raws(float* output, char* file_name, int n, int h, int w, int c)
{
  LOGV("VZ Debug: Detector dump started");
  std::string filename_str = file_name;
  #ifdef __android__
      std::string dump_path = "/sdcard/Download/detector_dumps/" + filename_str + ".raw";
  #else
      std::string dump_path = "/sdcard/Download/detector_dumps/" + filename_str + ".raw";
  #endif
  LOGV("VZ Debug: Detector log dump_path: %s", dump_path.c_str());
  std::ofstream out (dump_path, std::ios::out | std::ios::binary);
  if(!out)
  {
    std::cout<<"Cannot open file";
    LOGV("VZ Debug: Not found file %s", dump_path.c_str());
  }
  LOGV("VZ Debug: Detector log dumping output");
  out.write((char*)output, sizeof(float) * n * h* w *c);
  out.close();
  return true;
}

void correct_box_bounds(unified_detector::Rectangle &obj_box, int &img_w, int &img_h){
    if (obj_box.left < 0)       obj_box.left = 0;
    if (obj_box.top < 0)        obj_box.top = 0;
    if (obj_box.right < 0)      obj_box.right = 0;
    if (obj_box.bottom < 0)     obj_box.bottom = 0;
    if (obj_box.left > img_w)   obj_box.left = img_w;
    if (obj_box.top > img_h)    obj_box.top = img_h;
    if (obj_box.right > img_w)  obj_box.right = img_w;
    if (obj_box.bottom > img_h) obj_box.bottom = img_h;
}
std::vector<unified_detector::BoundingBox> detector_utils::get_boxes_from_2D_tensors(std::vector<std::vector<float> > roi, cv::Mat input_image){
   int n_boxes = roi.size();
   int img_w = input_image.cols;
   int img_h = input_image.rows;
   std::vector<unified_detector::BoundingBox> boxes;
   for (int i = 0; i < n_boxes; i++) {
       int id = (int)roi[i][5];	/* if id is zero ignore the box */	
       if(!id)
            continue;
       unified_detector::Rectangle obj_box;
       obj_box.left   = roi[i][0] * img_w;
       obj_box.top    = roi[i][1] * img_h;
       obj_box.right  = roi[i][2] * img_w;
       obj_box.bottom = roi[i][3] * img_h;
       correct_box_bounds(obj_box, img_w, img_h);
       float box_score  = roi[i][4];
       std::string label = std::to_string(roi[i][5]);
       unified_detector::BoundingBox new_box(obj_box, {img_w, img_h}, label, box_score, id, -1, false, true);
       boxes.push_back(new_box);
   }
   //LOGV("get_boxes_from_2D_tensors boxes size : %d", boxes.size());
   //for (int i = 0; i < boxes.size(); i++) {
    //LOGV("get_boxes_from_2D_tensors boxes[%d] left : %d, top : %d, right : %d, bottom : %d, label : %s, score : %f", i, boxes[i].location.left, boxes[i].location.top, boxes[i].location.right, boxes[i].location.bottom, boxes[i].tag.c_str(), boxes[i].score);
   //}
   return boxes;
}

cv::Mat detector_utils::preprocess_image(char* image_buffer,int width,int height,int target_width,int target_height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format){
    cv::Mat image;
    if(image_format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV21 || image_format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV12){
        //This is done so that image orientation is resized and corrected before its passed to tagger
        #ifdef ANDROID_ARM_NEON
          image = viz_arm_neon::nv21_or_nv12_resize_convert_rotate(image_buffer, width, height, target_width, target_height, image_rotation, image_format);
          #else
              image = nv_convert_resize_rotate(image_buffer,width,height,target_width,target_height,image_rotation,image_format);
          #endif
    }
    else{
        //rest of the image types
        image = get_bgr_mat_from_buffer(image_buffer, width, height, image_format,target_width,target_height);
        image = rotate_image(image, image_rotation);
    }
    return image;
}

/**
 * @brief rotation_corrected_roi corrects the ROI based on rotation
 * @details Corrects the roi based on roi and image dimensions
 *
 * @param[in] defines the roi for the image
 * @param[in] target width of image
 * @param[in] target height of image   
 * @param[in] image_rotation defines rotation angle of image
 *
 * @return roi with correction
 */
 unified_detector::Rectangle rotation_corrected_roi(unified_detector::Rectangle roi,int width,int height,const int angular_clockwise_rotation){
    unified_detector::Rectangle bounds({roi.left, roi.top,
                  roi.right, roi.bottom});
    int image_width  = width;
    int image_height = height;
    switch(angular_clockwise_rotation){
      case 270:
        bounds = {bounds.top, image_height - bounds.right, bounds.bottom, image_height - bounds.left};
        break;
      case 180:
        bounds = {image_width - bounds.right, image_height - bounds.bottom, image_width - bounds.left, image_height - bounds.top};
        break;
      case 90:
        bounds = {image_width - bounds.bottom, bounds.left, image_width - bounds.top, bounds.right};
        break;
    }
    return bounds;
  }

/**
 * @brief correct_roi_boxes corrects the ROI based on rotation
 * @details Corrects the roi based on new image dimensions
 *
 * @param[in] target width of image
 * @param[in] target height of image   
 * @param[in] image_rotation defines rotation angle of image
 * @param[in] width of image
 * @param[in] height of image
 *
 * @return void
 */
 std::vector<unified_detector::BoundingBox> detector_utils::correct_roi_boxes(std::vector<unified_detector::BoundingBox> result_boxes,int target_width,int target_height,const int angular_rotation_clockwise,int width,int height){
    for(int i=0;i<result_boxes.size();i++){
        unified_detector::Rectangle region=rotation_corrected_roi(unified_detector::Rectangle(result_boxes[i].location.left,
                                              result_boxes[i].location.top,
                                              result_boxes[i].location.right,
                                              result_boxes[i].location.bottom),
                                              target_width,target_height,angular_rotation_clockwise);
  
  
      result_boxes[i].location.left=(int)(((float)region.left)*(((float)width)/((float)target_width)));
      result_boxes[i].location.top=(int)(((float)region.top)*(((float)height)/((float)target_height)));
      result_boxes[i].location.right=(int)(((float)region.right)*(((float)width)/((float)target_width)));
      result_boxes[i].location.bottom=(int)(((float)region.bottom)*(((float)height)/((float)target_height)));
    }    
    return result_boxes;
  }

  bool detector_utils::od_classes_detected(std::vector<unified_detector::BoundingBox> result_boxes){
    std::unordered_set<int> od_class_ids = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    for(int i =0;i<result_boxes.size();i++){
        if(od_class_ids.count(result_boxes[i].tag_id)){
            return true;
        }
    }
    return false;
  }

  std::vector<unified_detector::BoundingBox> detector_utils::remove_smaller_boxes(std::vector<unified_detector::BoundingBox> entities, int img_w, int img_h, float ratio_threshold, int num_boxes){
    std::vector<unified_detector::BoundingBox> selected_boxes;
    float area_image = (float)(img_w * img_h);
    float changed_threshold = 0.0;
    for (int i = 0; i < entities.size(); i++) {
        changed_threshold = ratio_threshold;
        int l = entities[i].location.left;
        int r = entities[i].location.right;
        int t = entities[i].location.top;
        int b = entities[i].location.bottom;
        float area_box = (float)((r - l + 1) * (b - t + 1));
        float ratio = area_box / area_image;
        float aspect_ratio = float(r - l + 1) / float(b - t + 1);
        if (aspect_ratio < 0.5 || aspect_ratio > 2.0) {
            if (num_boxes == 1) {
                changed_threshold = 0.05;
            } else {
                changed_threshold = 0.02;
            }
        }
        if (ratio > changed_threshold) {
            selected_boxes.push_back(entities[i]);
        }
    }
    return selected_boxes;
  }

  std::vector<unified_detector::BoundingBox> detector_utils::sort_entities(std::vector<unified_detector::BoundingBox> entities, std::vector<float> scores){
    if(entities.size() != scores.size()){
        //Assert score for each entity is available
        return {};
    }
    std::vector<int> indices(entities.size());
    int idx = 0;
    std::iota(indices.begin(), indices.end(), idx++);
    std::sort(indices.begin(), indices.end(), [scores](int i, int j){
        return scores[i] < scores[j];
    });

    std::vector<unified_detector::BoundingBox> sorted_entities;
    for(int index: indices){
        sorted_entities.push_back(entities[index]);
    }
    return sorted_entities;
}

  std::vector<unified_detector::BoundingBox> detector_utils::get_top_boxes(std::vector<unified_detector::BoundingBox>& boxes, int img_w, int img_h, int num_boxes){
    std::vector<unified_detector::BoundingBox> candidate_boxes;
    std::vector<unified_detector::BoundingBox> top_boxes;
    int taken = 0;
    if (boxes.size() <= num_boxes) {
        return boxes;
    }
    else {
        std::vector<float> box_scores;
        for (unified_detector::BoundingBox box : boxes) {
            float center_x = img_w * 0.5;
            float center_y = img_h * 0.5;
            unified_detector::Rectangle loc;
            loc = box.location;

            float loc_y = (loc.bottom + loc.top) * 0.5;
            float loc_x = (loc.right + loc.left) * 0.5;
            float center_dist = sqrt((center_y - loc_y) * (center_y - loc_y) +
                                    (center_x - loc_x) * (center_x - loc_x));

            float area_box = (float)(loc.bottom - loc.top + 1) * (loc.right - loc.left + 1);
            box_scores.push_back(center_dist + (1.0 / area_box));
            candidate_boxes.push_back(box);
        }
        candidate_boxes = sort_entities(candidate_boxes, box_scores);
        for(int i=0; i<MIN(candidate_boxes.size(), num_boxes); i++){
            top_boxes.push_back(candidate_boxes[i]);
        } 
    }

    return top_boxes;
}


  std::vector<unified_detector::BoundingBox> detector_utils::get_top_object_boxes(std::vector<unified_detector::BoundingBox> result_boxes, int width, int height){
    std::vector<unified_detector::BoundingBox> object_boxes;
    float area_img = (float)(width * height);
    float object_box_area_threshold = 0.030;
    float box_threshold = 0.30;

    for (auto &entity : result_boxes){
      if (entity.tag == "object" && entity.score > box_threshold && (entity.location.area()> object_box_area_threshold*area_img)){
        object_boxes.push_back(entity);
        float small_criteria_multi = (MAX_OBJECT_COUNT == 1) ? 0.1f : 0.05f;
        result_boxes = remove_smaller_boxes(result_boxes, width, height, small_criteria_multi, MAX_OBJECT_COUNT);
        result_boxes = get_top_boxes(result_boxes, width, height, MAX_OBJECT_COUNT);
      }
    }
    return object_boxes;
  }

  std::vector<unified_detector::BoundingBox> detector_utils::remove_object_boxes(std::vector<unified_detector::BoundingBox> result_boxes){
    std::vector<unified_detector::BoundingBox> filtered_result_boxes;
    for(int i =0;i<result_boxes.size();i++){
      if(result_boxes[i].tag_id ==1 ){
        continue;
      }else{
        filtered_result_boxes.push_back(result_boxes[i]);
      }
    }
    return filtered_result_boxes;
  }

  int detector_utils::resize_uniform(const cv::Mat &src, cv::Mat &dst, cv::Size dst_size) {
    int dst_w = dst_size.width;
    int dst_h = dst_size.height;
    cv::resize(src, dst, dst_size);
    return 0;
  }

  float detector_utils::calculateDistance(int x1, int y1, int x2, int y2) {
    int32x4_t dx = vdupq_n_s32(x2 - x1);
    int32x4_t dy = vdupq_n_s32(y2 - y1);
    int32x4_t dxdx = vmulq_s32(dx, dx);
    int32x4_t dydy = vmulq_s32(dy, dy);
    int32x4_t dist = vaddq_s32(dxdx, dydy);
    float32x4_t distf = vcvtq_f32_s32(dist);
    float32x2_t distf2 = vpadd_f32(vget_low_f32(distf), vget_high_f32(distf));
    distf2 = vpadd_f32(distf2, distf2);
    return vget_lane_f32(distf2, 0);
  }
  bool compare_distances(const std::pair<float, unified_detector::BoundingBox>& a, const std::pair<float, unified_detector::BoundingBox>& b) {
    return a.first < b.first;
  }
  // Function to find the bounding box nearest to the image center
  std::vector<std::pair<float, unified_detector::BoundingBox>> detector_utils::findNearestBoundingBox(const std::vector<unified_detector::BoundingBox>& detector_boxes,int imageWidth, int imageHeight) {
    std::vector<std::pair<float, unified_detector::BoundingBox>> distanceAndBoxes;
    int centerX = imageWidth / 2;
    int centerY = imageHeight / 2;

    for (const auto& box : detector_boxes) {
        int boxCenterX = (box.location.left + box.location.right) / 2;
        int boxCenterY = (box.location.top + box.location.bottom) / 2;
        float distance = calculateDistance(boxCenterX, boxCenterY, centerX, centerY);
        distanceAndBoxes.emplace_back(distance, box);
    }
    std::sort(distanceAndBoxes.begin(), distanceAndBoxes.end(),compare_distances);
    return distanceAndBoxes;
  }
  
  std::vector<unified_detector::BoundingBox> detector_utils::get_saliency_boxes_to_center(std::vector<unified_detector::BoundingBox> result_boxes,int width,int height){
    int max_boxes =1;
    float min_distance = 3.4028235e+38;
    const float area_thresh = 5.0f; //5% of the total frame
    std::vector<std::pair<float, unified_detector::BoundingBox>> distanceAndBoxes;
    std::vector<unified_detector::BoundingBox> final_boxes = {};
    for(int i =0; i<result_boxes.size();i++){
        if(result_boxes[i].isObjectCategory){
          float areaRatio = (result_boxes[i].location.right - result_boxes[i].location.left)*(result_boxes[i].location.bottom - result_boxes[i].location.top)/(float)(width*height)*100.0f;
          if ((result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_PERSON) && areaRatio > area_thresh) ||
          (result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_FOOD) && areaRatio > area_thresh) ||
          (result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_DRINK) && areaRatio > area_thresh) ||
          (result_boxes[i].tag_id ==  11 && areaRatio > 5.0f) ||
          (result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_FLOWER) && areaRatio > area_thresh) ||
          (result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_TEXT) && areaRatio > area_thresh)) {
            LOGV("Calculating center and deistance");
            float ROIcenterX = (result_boxes[i].location.left + result_boxes[i].location.right)/2.0f;
            float ROIcenterY = (result_boxes[i].location.top + result_boxes[i].location.bottom)/2.0f;
            float dist_x = (ROIcenterX/(float)width - 0.5f);
            float dist_y = (ROIcenterY/(float)height - 0.5f);
            float distance = sqrt(dist_x*dist_x+dist_y*dist_y);
            LOGV("Near box to centwr i, tag class, distance : %d %s %f", i, result_boxes[i].tag.c_str(), distance);
            distanceAndBoxes.emplace_back(distance, result_boxes[i]);
          }
      }
    }
    if(distanceAndBoxes.empty()){
      //LOGV("Returning final boxes of size: %d", final_boxes.size());
      return final_boxes;
    }

    std::sort(distanceAndBoxes.begin(), distanceAndBoxes.end(),compare_distances);
    for(int i =0;i<max_boxes;i++){
      final_boxes.push_back(distanceAndBoxes[i].second);
    }
    //LOGV("Returning final boxes of size: %d", final_boxes.size());
    return final_boxes;

  }