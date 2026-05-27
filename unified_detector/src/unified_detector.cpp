/**
 * @file   unified_detector.cpp
 * @brief  This file implements the UnifiedDetector Interface
 *
 * This file implements UnifiedDetector Interface to be used by end api's
 */

 #include "unified_detector.h"
 #include "unified_detector_impl.hpp"
 #include "snap/snap_wrapper.h"
 
 namespace unified_detector{    
 
 bool __attribute__((visibility("default"))) UnifiedDetector::initialize(void (*cb)(bool status),UNIFIED_DETECTOR_MODE param, char* model_base_path){
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (param.mode);
    return ((UnifiedDetectorImpl*)this)->initialize(cb,mode, model_base_path);
 }
 
 bool __attribute__((visibility("default"))) UnifiedDetector::initialize(UNIFIED_DETECTOR_MODE param, char* model_base_path){
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (param.mode);
    return ((UnifiedDetectorImpl*)this)->initialize(mode, model_base_path);
 }
 
 bool __attribute__((visibility("default"))) UnifiedDetector::execute(UNIFIED_DETECTOR_MODE mode_param, UNIFIED_DETECTOR_EXECUTE_PARAM execute_param, char* file_name){
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat imageFormat= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat> (execute_param.imageFormat);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (mode_param.mode);
    int width = execute_param.width;
    int height = execute_param.height;
    int image_rotation = execute_param.image_rotation;
    char* image_buffer = execute_param.image_buffer;
    //file_name = execute_param.filename;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType> (execute_param.zoomType);
    return ((UnifiedDetectorImpl*)this)->execute(image_buffer, width, height, image_rotation, imageFormat,false, mode,zoom_type,file_name);
 }

 bool __attribute__((visibility("default"))) UnifiedDetector::execute(UNIFIED_DETECTOR_MODE mode_param, char* data, int width,int height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,char *file_name){
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (mode_param.mode);
    return ((UnifiedDetectorImpl*)this)->execute(mode, data, width, height, image_rotation, image_format, file_name);
 }
 
 bool __attribute__((visibility("default"))) UnifiedDetector::execute_with_tracking(UNIFIED_DETECTOR_MODE mode_param, UNIFIED_DETECTOR_EXECUTE_PARAM execute_param, char* file_name){
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat imageFormat= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat> (execute_param.imageFormat);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (mode_param.mode);
    int width = execute_param.width;
    int height = execute_param.height;
    int image_rotation = execute_param.image_rotation;
    char* image_buffer = execute_param.image_buffer;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType> (execute_param.zoomType);
    return ((UnifiedDetectorImpl*)this)->execute_with_tracking(image_buffer, width, height, image_rotation, imageFormat, mode,zoom_type,file_name);
 }

 int __attribute__((visibility("default")))  UnifiedDetector::get_saliency_boxes_to_center_count(){
    return ((UnifiedDetectorImpl*)this)->get_saliency_boxes_to_center_count();    
}
 
 int __attribute__((visibility("default")))  UnifiedDetector::get_n_boxes(){
     return ((UnifiedDetectorImpl*)this)->get_n_boxes();    
 }
 
 bool __attribute__((visibility("default")))  UnifiedDetector::contains_doc_text(){
     return ((UnifiedDetectorImpl*)this)->contains_doc_text();    
 }

 int __attribute__((visibility("default")))  UnifiedDetector::get_n_tags(int max_scene_infos){
    return ((UnifiedDetectorImpl*)this)->get_n_tags(max_scene_infos);    
}
 
 bool __attribute__((visibility("default"))) UnifiedDetector::get_tag_info(TagInfo tag_infos[], int n){
     return ((UnifiedDetectorImpl*)this)->get_tag_info(tag_infos, n);    
 }
 
 } //namespace unified_detector
 
 unified_detector::UnifiedDetector* __attribute__((visibility("default"))) unified_detector::UnifiedDetector::Create(){
     return (UnifiedDetector*) new UnifiedDetectorImpl();
 }
 
 void __attribute__((visibility("default"))) unified_detector::UnifiedDetector::Destroy(UnifiedDetector* unified_detector){
     delete (UnifiedDetectorImpl*) (unified_detector);
 }

 //Vex changes
 long  __attribute__((visibility("default"))) unified_detector::UnifiedDetector::get_supported_mode_count(){
    return ((UnifiedDetectorImpl*)this)->get_supported_mode_count();
 }
 int __attribute__((visibility("default"))) unified_detector::UnifiedDetector::get_supported_modes(int* modes){
    return ((UnifiedDetectorImpl*)this)->get_supported_modes(modes);
 }
 long __attribute__((visibility("default"))) unified_detector::UnifiedDetector::get_supported_tag_count(int mode){
    return ((UnifiedDetectorImpl*)this)->get_supported_tag_count(mode);
 }
 int __attribute__((visibility("default"))) unified_detector::UnifiedDetector::get_supported_tags(int mode, const char** tags){
    return ((UnifiedDetectorImpl*)this)->get_supported_tags(mode,tags);
 }
 