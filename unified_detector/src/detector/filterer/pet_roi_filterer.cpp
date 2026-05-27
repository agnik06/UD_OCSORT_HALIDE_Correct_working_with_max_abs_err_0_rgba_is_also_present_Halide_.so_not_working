#include <assert.h>

#include "pet_roi_filterer.hpp"
#include "roi_utils.hpp"
#include "logger/logger.hpp"

bool PetROIFilterer::is_small_petface_boxes(unified_detector::BoundingBox b1 ,  unified_detector::BoundingBox b2){

    int minAreaBoxThreshold = 0.15*100;
    int maxAreaBoxThreshold = 0.85*100;

    int area_box_b1 = b1.location.area();
    int area_box_b2 = b2.location.area();

    float ratio = float(area_box_b2) / float(area_box_b1);
    int areaThreshold = ratio*100;
    //LOGV("VZ Debug: is_small_petface_boxes  ratio: [%d]",areaThreshold);

    if(areaThreshold > minAreaBoxThreshold && areaThreshold < maxAreaBoxThreshold){
        return true;
    }
    return false;
}

PetROIFilterer::PetROIFilterer(std::shared_ptr<ROIDataHandler> data_handler,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode)
    : BaseROIFilterer(data_handler,mode){

}
PetROIFilterer::~PetROIFilterer(){

}

std::vector<unified_detector::BoundingBox> PetROIFilterer::filter(std::vector<unified_detector::BoundingBox> roi_boxes, cv::Mat image,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode){
    //LOGV("VZ Debug: PetROIFilterer::filter() called");
    int width = image.cols;
    int height = image.rows;
    roi_boxes = remove_invalid_boxes(roi_boxes,width,height,mode);
    roi_boxes  = remove_petface_over_person_boxes(roi_boxes);
    roi_boxes  = ensure_petface_over_pet_boxes(roi_boxes);
    roi_boxes  = ensure_petface_with_pet_boxes(roi_boxes);
    roi_boxes = remove_unsupported_categories(roi_boxes);
    return roi_boxes;
}

std::vector<unified_detector::BoundingBox> PetROIFilterer::remove_petface_over_person_boxes(std::vector<unified_detector::BoundingBox> boxes){
    std::vector<unified_detector::BoundingBox> filtered_boxes;
      
    for(int i = 0; i < boxes.size(); i++){
        // If it's not a petface box, add it directly
        if(roi_data_handler->get_label(boxes[i].tag_id) != "petface"){
            filtered_boxes.push_back(boxes[i]);
            continue;
        }
        
        bool should_add_petface = true;  // Default: add the petface box
        bool overlaps_with_person = false;
        
        // Check if petface overlaps with any person box
        for(int j = 0; j < boxes.size(); j++){
            if(roi_data_handler->get_label(boxes[j].tag_id) == "person"){
                int overlap_person = ROIUtils::check_overlap_box(boxes[j].location, boxes[i].location, 0.85);
                if(overlap_person == 1){
                    overlaps_with_person = true;
                    break;  // Found overlap with person, no need to check more
                }
            }
        }
        
        // If petface overlaps with person, check if it also overlaps with any pets box
        if(overlaps_with_person){
            should_add_petface = false;  // Don't add by default
            
            for(int k = 0; k < boxes.size(); k++){
                if(roi_data_handler->get_label(boxes[k].tag_id) == "pets"){
                    int overlap_animal = ROIUtils::check_overlap_box(boxes[k].location, boxes[i].location, 0.8);
                    if(overlap_animal != -1){
                        should_add_petface = true;  // Add it once if it overlaps with any pets box
                        //LOGV("VZ Debug: remove_petface_over_person_boxes petface adding id=[%d], threshold: [%f], i: [%d], id=[%d], threshold: [%f], k:[%d], id=[%d], threshold:[%f, j:[%d]", 
                             //boxes[i].tag_id, boxes[i].score, i, boxes[k].tag_id, boxes[k].score, k, boxes[j].tag_id, boxes[j].score, j);
                        break;  // Only need to find one overlapping pets box to add the petface
                    }
                }
            }
        }
        
        // Add the petface box if it should be added
        if(should_add_petface){
            filtered_boxes.push_back(boxes[i]);
        }
    }
    return filtered_boxes;
}

std::vector<unified_detector::BoundingBox> PetROIFilterer::ensure_petface_over_pet_boxes(std::vector<unified_detector::BoundingBox> boxes){
    std::vector<unified_detector::BoundingBox> filtered_boxes;
    for(int j=0;j<boxes.size();j++){
        if(roi_data_handler->get_label(boxes[j].tag_id)!="pets"){
            filtered_boxes.push_back(boxes[j]);
            continue;
        }
        if(roi_data_handler->get_label(boxes[j].tag_id) == "pets"){
            int added = 0;
            for(int k = 0;k < boxes.size(); k++){
                if(roi_data_handler->get_label(boxes[k].tag_id) == "petface"){
                    int overlap_pet = ROIUtils::check_overlap_box(boxes[j].location,boxes[k].location,0.80);
                    if(overlap_pet != -1){
                        #ifdef DUMPBOX
                            LOGV("VZ Debug: ensure_petface_over_pet_boxes adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                        #endif
                        filtered_boxes.push_back(boxes[j]);
                        added = 1;
                        break;
                    }
                }
            }
            if(!added){
                if(boxes[j].score > 0.60){
                    #ifdef DUMPBOX
                        LOGV("VZ Debug: ensure_petface_over_pet_boxes with no petfacebox adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                    #endif
                    filtered_boxes.push_back(boxes[j]);
                } else {
                    #ifdef DUMPBOX
                        LOGV("VZ Debug: ensure_petface_over_pet_boxes Skipping adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                    #endif
                }
            }
        }
    }
    return filtered_boxes;
}

std::vector<unified_detector::BoundingBox> PetROIFilterer::ensure_petface_with_pet_boxes(std::vector<unified_detector::BoundingBox> boxes){
    std::vector<unified_detector::BoundingBox> filtered_boxes;
    for(int j=0;j<boxes.size();j++){
        if(roi_data_handler->get_label(boxes[j].tag_id)!="petface"){
            filtered_boxes.push_back(boxes[j]);
            continue;
        }
        if(roi_data_handler->get_label(boxes[j].tag_id) == "petface"){
            int added = 0;
            for(int k = 0;k < boxes.size(); k++){
                if(roi_data_handler->get_label(boxes[k].tag_id) == "pets"){
                    int overlap_pet = ROIUtils::check_overlap_box(boxes[k].location,boxes[j].location,0.80);
                    if(overlap_pet != -1){
                        if(is_small_petface_boxes(boxes[k], boxes[j])){
                            #ifdef DUMPBOX
                               // LOGV("VZ Debug: ensure_petface_with_pet_boxes adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                                LOGV("VZ Debug: ensure_petface_with_pet_boxes adding id=[%d], threshold: [%f] j=[%d]",boxes[j].tag_id, boxes[j].score,j);
                            #endif
                            filtered_boxes.push_back(boxes[j]);
                            added = 1;
                            break;
                        } else {
                            #ifdef DUMPBOX
                                LOGV("VZ Debug: ensure_petface_with_pet_boxes Skipping petface id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                            #endif
                        }
                    }
                }
            }
            if(!added){
                if(boxes[j].score > 0.65){
                    #ifdef DUMPBOX
                        LOGV("VZ Debug: ensure_petface_with_pet_boxes with no petbox adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                    #endif
                    filtered_boxes.push_back(boxes[j]);
                } else {
                    #ifdef DUMPBOX
                        LOGV("VZ Debug: ensure_petface_with_pet_boxes Skipping adding id=[%d], threshold: [%f]",boxes[j].tag_id, boxes[j].score);
                    #endif
                }
            }
        }
    }
    return filtered_boxes;
}