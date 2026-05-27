#include <assert.h>

#include "base_roi_filterer.hpp"
#include "roi_utils.hpp"
#include "logger/logger.hpp"

BaseROIFilterer::BaseROIFilterer(std::shared_ptr<ROIDataHandler> data_handler,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode)
    : IROIFilterer(data_handler,mode){

}
BaseROIFilterer::~BaseROIFilterer(){

}

std::vector<unified_detector::BoundingBox> BaseROIFilterer::filter(std::vector<unified_detector::BoundingBox> roi_boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode){
    //LOGV("BaseROIFilterer::filter called boxes size : [%d]", roi_boxes.size());
    int width = image.cols;
    int height = image.rows;
    roi_boxes = remove_invalid_boxes(roi_boxes, width, height,mode);
    //LOGV("BaseROIFilterer::filter called return boxes size : [%d]", roi_boxes.size());
    //for(int i =0;i<roi_boxes.size();i++){
        //LOGV("VZ Debug: Det Label execute:[%s], Det class id:[%d]",roi_boxes[i].tag.c_str(), roi_boxes[i].tag_id);
    //}
    return roi_boxes;
}
//removing boxes with priority = 0
std::vector<unified_detector::BoundingBox> BaseROIFilterer::remove_invalid_boxes(std::vector<unified_detector::BoundingBox> boxes,int width,int height,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode){
    std::vector<unified_detector::BoundingBox> filtered_boxes;
    std::vector<unified_detector::BoundingBox> object_boxes;
    for(int i=0; i<boxes.size(); i++){
        int priority_index =0;
         priority_index = roi_data_handler->get_priority(boxes[i].tag);
        bool is_valid = roi_data_handler->is_valid_id(boxes[i].tag_id);
        if((priority_index > 0)&&(is_valid)){
            //applying thresholds
             if(roi_data_handler->get_threshold(roi_data_handler->get_label(boxes[i].tag_id))<boxes[i].score){
                 if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::ODCLS && boxes[i].tag_id ==1){
                    //LOGV("Adding the object box for ODCLS mode");
                    object_boxes.push_back(boxes[i]);
                 }else{
                    if(boxes[i].tag_id !=1){
                        filtered_boxes.push_back(boxes[i]);
                    }
                 }
             }else{
                 // Adding the text and scene text boxes not considering the threshold to fix the S24 MR issues.
                 if(boxes[i].tag_id == 12 || boxes[i].tag_id == 13){
                     filtered_boxes.push_back(boxes[i]);
                 }
             }
         }
     
    }
     if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::ODCLS){
        object_boxes=get_one_object_box(object_boxes, width,height);
        filtered_boxes.insert(filtered_boxes.end(),object_boxes.begin(),object_boxes.end());        
     }
     return filtered_boxes;
 }

 std::vector<unified_detector::BoundingBox> BaseROIFilterer::remove_unsupported_categories(std::vector<unified_detector::BoundingBox> boxes)
{
    std::vector<unified_detector::BoundingBox> filtered_boxes;
    for(int i=0;i<boxes.size();i++){
        if(boxes[i].tag_id==10 || boxes[i].tag_id==4 || boxes[i].tag_id==6)
            continue;
        else
        {
            filtered_boxes.push_back(boxes[i]);
        }
    }
    return filtered_boxes;
}

std::vector<unified_detector::BoundingBox> BaseROIFilterer::get_one_object_box(std::vector<unified_detector::BoundingBox> object_boxes,int width,int height)
{
    //LOGV("Getting the top one object box near to center for ODCLS mode boxes are : %d",object_boxes.size());
    float small_criteria_multi = (max_object_count == 1) ? 0.1f : 0.05f;
    object_boxes = ROIUtils::remove_smaller_boxes(object_boxes, width, height, small_criteria_multi, max_object_count);
    //LOGV("After remove_smaller_boxes object boxes are : %d", object_boxes.size());
    object_boxes = ROIUtils::get_top_boxes(object_boxes, width, height, max_object_count);
    //LOGV("After get_one_object_box object boxes are : %d", object_boxes.size());
    return object_boxes;
}

