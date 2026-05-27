#include <assert.h>

#include "text_roi_filterer.hpp"
#include "roi_utils.hpp"
#include "logger/logger.hpp"
TextROIFilterer::TextROIFilterer(std::shared_ptr<ROIDataHandler> data_handler, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode)
    : BaseROIFilterer(data_handler,mode){

}
TextROIFilterer::~TextROIFilterer(){

}

std::vector<unified_detector::BoundingBox> TextROIFilterer::filter(std::vector<unified_detector::BoundingBox> roi_boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode){
    int width = image.cols;
    int height = image.rows;
    roi_boxes = remove_invalid_boxes(roi_boxes,width,height,mode);
    #ifdef TESTING
        LOGV("VZ Debug: filter org : [%d]",roi_boxes.size());
    #endif
    roi_boxes  = merge_text_boxes_inside_wine(roi_boxes);
    roi_boxes  = get_one_text_box(roi_boxes,width,height);
    #ifdef TESTING
        LOGV("VZ Debug: get_one_text_box after No. of boxes : [%d]",roi_boxes.size());
    #endif
    roi_boxes = remove_unsupported_categories(roi_boxes);
    return roi_boxes;
}
/*bool TextROIFilterer::get_text_box_based_on_large_area(unified_detector::BoundingBox box,int img_w,int img_h){
    float area_image = (float)(img_w * img_h);
    int l = box.location.left;
    int r = box.location.right;
    int t = box.location.top;
    int b = box.location.bottom;
    float area_box = (float)((r - l + 1) * (b - t + 1));
    float ratio = float(area_box) / float(area_image);
    int areaThreshold = ratio*100;
    int minAreaTextBoxThreshold = 0.50*100;
    #ifdef TESTING
        LOGV("get_text_box_based_on_large_area areaThreshold :[%d], minAreaTextBoxThreshold : [%d]", areaThreshold,minAreaTextBoxThreshold);
    #endif
    if(areaThreshold > minAreaTextBoxThreshold){
        return true;
    }else{
        return false;
    }
    
}*/

bool textBoxconfidenceComparatorV2(unified_detector::BoundingBox box1, unified_detector::BoundingBox box2){
  return box2.score < box1.score;
}

bool is_box_empty(const unified_detector::BoundingBox& box) {
    return box.score < 0.0f;  // Negative score indicates empty box
}

unified_detector::BoundingBox TextROIFilterer::create_merged_low_conf_text_box(const std::vector<unified_detector::BoundingBox> boxes, bool text_box_added) {
    std::vector<unified_detector::BoundingBox> low_conf_text_boxes;

    if (!boxes.empty() && !text_box_added) {
        for (size_t i = 0; i < boxes.size(); i++) {
            if ((boxes[i].tag_id == 12 || boxes[i].tag_id == 13) && boxes[i].score > 0.25) {
                low_conf_text_boxes.push_back(boxes[i]);
            }
        }
    }

    if (low_conf_text_boxes.empty()) {
        unified_detector::BoundingBox empty_box;
        empty_box.score = -1.0f;  // Use negative score to indicate empty box
        return empty_box;
    }

    // Initialize with first box and start score accumulation
    int min_top = low_conf_text_boxes[0].location.top;
    int min_left = low_conf_text_boxes[0].location.left;
    int max_bottom = low_conf_text_boxes[0].location.bottom;
    int max_right = low_conf_text_boxes[0].location.right;
    float total_score = low_conf_text_boxes[0].score;

    // Find bounding coordinates and accumulate scores
    for (size_t i = 1; i < low_conf_text_boxes.size(); i++) {
        min_top = std::min(min_top, low_conf_text_boxes[i].location.top);
        min_left = std::min(min_left, low_conf_text_boxes[i].location.left);
        max_bottom = std::max(max_bottom, low_conf_text_boxes[i].location.bottom);
        max_right = std::max(max_right, low_conf_text_boxes[i].location.right);
        total_score = std::max(total_score, low_conf_text_boxes[i].score);
    }

    unified_detector::BoundingBox merged_box;
    merged_box.trackid = -1;
    merged_box.location.top = min_top;
    merged_box.location.left = min_left;
    merged_box.location.bottom = max_bottom;
    merged_box.location.right = max_right;
    merged_box.tag_id = 12; // or appropriate merged text box id
    merged_box.tag = "text";
    merged_box.score = total_score;// Calculate average score
    merged_box.isSceneCategory = false;
    merged_box.isObjectCategory = true;

    return merged_box;
}

std::vector<unified_detector::BoundingBox> TextROIFilterer::get_one_text_box(std::vector<unified_detector::BoundingBox> boxes,int image_width, int image_height){
    std::vector<unified_detector::BoundingBox> filtered_boxes;
    int text_box_added = 0;
    if(!boxes.empty())
		sort(boxes.begin(),boxes.end(), &textBoxconfidenceComparatorV2);
    for(int i=0; i<boxes.size(); i++){
         //LOGV("VZ Debug: get_one_text_box adding id=[%d], threshold: [%f]",boxes[i].tag_id, boxes[i].score, boxes[i].tag.c_str());
        if(boxes[i].tag_id == 12 || boxes[i].tag_id == 13){
            if(!text_box_added && roi_data_handler->get_threshold(roi_data_handler->get_label(boxes[i].tag_id))<boxes[i].score){
                if(boxes[i].tag_id == 12){
                    unified_detector::BoundingBox new_box = boxes[i];
                    new_box.tag = "doc_text";
                    new_box.tag_id = 17; // 17 is aligned with light_object_detector.h header file for text only
                    filtered_boxes.push_back(new_box);
                }
                if(boxes[i].tag_id == 13){
                    unified_detector::BoundingBox new_scene_box = boxes[i];
                    new_scene_box.tag = "sceneText";
                    new_scene_box.tag_id = 13;
                    filtered_boxes.push_back(new_scene_box);
                }
                unified_detector::BoundingBox new_box = boxes[i];
                new_box.tag = "text";
                new_box.tag_id = 12; // 17 is aligned with light_object_detector.h header file for text only
                filtered_boxes.push_back(new_box);
                text_box_added = 1;
            }
        }else{
            filtered_boxes.push_back(boxes[i]);
        }
    }
    if(boxes.size() !=0 && !text_box_added){
        unified_detector::BoundingBox merged_box;
        merged_box = create_merged_low_conf_text_box(boxes,text_box_added);
        if (!is_box_empty(merged_box)) {
            filtered_boxes.push_back(merged_box); // Add merged box to filtered boxes
        }
    }

    /*for(int i =0 ;i<filtered_boxes.size();i++ ){
        LOGV("VZ Debug: after get_one_text_box adding id=[%d], threshold: [%f]",boxes[i].tag_id, boxes[i].score, boxes[i].tag.c_str());
    }*/
    return filtered_boxes;
}
bool compare_areas(unified_detector::BoundingBox& box1, unified_detector::BoundingBox& box2) {
    return box1.location.area() > box2.location.area();
}

unified_detector::BoundingBox find_max_area_box(std::vector<unified_detector::BoundingBox>& boxes) {
    std::sort(boxes.begin(), boxes.end(), compare_areas);
    return boxes[0];
}
std::vector<unified_detector::BoundingBox> TextROIFilterer::merge_text_boxes_inside_wine(std::vector<unified_detector::BoundingBox> boxes){
    std::vector<unified_detector::BoundingBox> wine_boxes;
    std::vector<unified_detector::BoundingBox> wine_label_boxes_tmp;
    std::vector<unified_detector::BoundingBox> wine_label_boxes;
    std::vector<unified_detector::BoundingBox> other_boxes;
    float iou_threshold = 0.95;
    for(int i =0;i<boxes.size();i++){
        if(boxes[i].tag_id == 14){
            wine_boxes.push_back(boxes[i]);
            //Adding drink boxes for every wine box
            unified_detector::BoundingBox drink_box = boxes[i];
            drink_box.tag = "drink";
            drink_box.tag_id = 7;
            other_boxes.push_back(drink_box);
        }else if(boxes[i].tag_id == 15) {
            wine_label_boxes_tmp.push_back(boxes[i]);
            unified_detector::BoundingBox wine_label_text_box = boxes[i]; //mapping wine_label as sceneText
            wine_label_text_box.tag = "sceneText";
            wine_label_text_box.tag_id = 13;
            other_boxes.push_back(wine_label_text_box);
        }else{
            other_boxes.push_back(boxes[i]);
        }
    }
    for (unified_detector::BoundingBox& wine_box : wine_boxes) {
        std::vector<unified_detector::BoundingBox> overlapping_boxes;
        int min_left = INT_MIN;
        int min_top = INT_MIN;
        int max_right = INT_MAX;
        int max_bottom = INT_MAX;

        for(int i =0 ;i < wine_label_boxes_tmp.size();i++){
        //for (const unified_detector::BoundingBox& text_box : text_boxes) {
            int overlap = ROIUtils::check_overlap_box(wine_box.location, wine_label_boxes_tmp[i].location,0.85);
            if (overlap != -1) {
                overlapping_boxes.push_back(wine_label_boxes_tmp[i]);
            }
        }

        if(overlapping_boxes.size() ==1){
            wine_label_boxes.push_back(overlapping_boxes[0]);
        }
        else if (overlapping_boxes.size() > 1) {
            unified_detector::BoundingBox merged_box = find_max_area_box(overlapping_boxes);
            wine_label_boxes.push_back(merged_box);
        }
        /*if (overlapping_boxes.size() >= 1) {
            text_boxes.erase(std::remove_if(text_boxes.begin(), text_boxes.end(), [&wine_box](const unified_detector::BoundingBox& box) {
                return (ROIUtils::check_overlap_box(wine_box.location, box.location,0.85) != -1)? true:false ;
            }), text_boxes.end());
        }*/
    }
    int id_cnt = 1;
    for(int i = 0; i < wine_boxes.size(); i++){
        for(int j =0; j<wine_label_boxes.size(); j++){
            int overlap = ROIUtils::check_overlap_box(wine_boxes[i].location, wine_label_boxes[j].location,0.85);
            if(overlap != -1){
                wine_boxes[i].trackid = id_cnt;
                wine_label_boxes[j].trackid = id_cnt;
                id_cnt++;
                break;
            }
        }
    }
    for(int i=0; i<wine_boxes.size();i++){
        if(wine_boxes[i].trackid == -1){
            wine_boxes[i].trackid = id_cnt;
            id_cnt++;
        }
    }
    std::vector<unified_detector::BoundingBox> final_boxes;
    final_boxes.insert(final_boxes.end(), wine_boxes.begin(), wine_boxes.end());
    final_boxes.insert(final_boxes.end(), wine_label_boxes.begin(), wine_label_boxes.end());
    final_boxes.insert(final_boxes.end(), other_boxes.begin(), other_boxes.end());
    return final_boxes;

}