/**
 * @file   light_Unified_detector_wrapper.cpp
 * @brief  This file implements the UnifiedDetector Interface in form of a wrapper
 *
 * This file implements UnifiedDetector Interface to be used by end api's in form of a wrapper
 */

 #include "unified_detector_wrapper.h"
 #include <string>
 #include <vector>
 #include "logger/logger.hpp"
 #include "unified_detector.h"
 #include "unified_detector_impl.hpp"
 
 /**
  * @brief Constructor
  * @details Unified Detector wrapper instance is created
  *
  * @param[in] None
  *
  * @return void
  */
 UnifiedDetectorWrapper* __attribute__((visibility("default"))) UnifiedDetectorWrapper_create() {
     UnifiedDetectorWrapper* dli_handle = nullptr;
     try {
         dli_handle = reinterpret_cast<UnifiedDetectorWrapper*>(unified_detector::UnifiedDetector::Create());
     }
     catch(std::exception& ex) {
         LOGV("VZ Debug: UnifiedDetectorWrapper_create: can't create UnifiedDetectorWrapper: "
              " exception: %s", ex.what());
         return nullptr;
     }
       return dli_handle;
 }
 
 void __attribute__((visibility("default"))) UnifiedDetectorWrapper_destroy(UnifiedDetectorWrapper* handle) {
   //delete reinterpret_cast<viz::UnifiedDetector*>(handle);
   unified_detector::UnifiedDetector::Destroy(reinterpret_cast<unified_detector::UnifiedDetector*>(handle));
 }

 /**
  * @brief Initializes the Unified Detector
  * @details Initializes the Unified detector with mode, basepath
  *
  * @param[in] takes the mode
  *
  * @return bool status
  */
  bool __attribute__((visibility("default"))) UnifiedDetectorWrapper_initialize(UnifiedDetectorWrapper* handle,UNIFIED_DETECTOR_MODE_WRAPPER params, char* model_base_path){
    LOGV("VZ Debug: UnifiedDetectorWrapper_initialize model path c wrapper=[%s]", std::string(model_base_path).c_str());
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode MODE_Od= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (params.mode);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE param;
    param.mode = MODE_Od;
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->initialize(param, model_base_path);
}
 
 /**
  * @brief Initializes the Unified Detector
  * @details Initializes the Unified detector with mode, basepath and callback
  *
  * @param[in] takes the callback, mode and basepath
  * @param[in] takes the mode
  *
  * @return bool status
  */
 bool __attribute__((visibility("default"))) UnifiedDetectorWrapper_initialize_Cb(UnifiedDetectorWrapper* handle,void (*cb)(bool status),UNIFIED_DETECTOR_MODE_WRAPPER params, char* model_base_path){
     LOGV("VZ Debug: UnifiedDetectorWrapper_initialize_Cb model path c wrapper=[%s]", std::string(model_base_path).c_str());
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode MODE_Od= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (params.mode);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE param;
    param.mode = MODE_Od;
     return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->initialize(cb,param, model_base_path);
 }
 
 
 /**
  * @brief Performs inference on an image buffer
  * @details Handles image format NV21 and NV12
  *
  * @param[in] image_buffer defines the image buffer
  * @param[in] width of image
  * @param[in] height of image   
  * @param[in] image_rotation defines rotation angle of image
  * @param[in] image_format of given image(one of types in ImageFormat)
  *
  * @return bool status of inference
  */
 
 bool __attribute__((visibility("default"))) UnifiedDetectorWrapper_execute(UnifiedDetectorWrapper* handle,UNIFIED_DETECTOR_MODE_WRAPPER mode_param, UNIFIED_DETECTOR_EXECUTE_WRAPPER execute_params, char* file_name){
    LOGV("VZ Debug: UnifiedDetectorWrapper_execute c wrapper" );
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (mode_param.mode);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat>(execute_params.imageFormat);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM param;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE m_mode;
    m_mode.mode = mode;
    param.image_rotation = execute_params.image_rotation;  
    param.imageFormat = image_format; 
    param.width = execute_params.width;
    param.height = execute_params.height;
    param.image_buffer = execute_params.image_buffer;
    param.zoomType = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType>(execute_params.zoomType);
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->execute(m_mode, param, file_name);
 }

  /**
  * @brief Performs inference on an image buffer
  * @details Handles image format NV21 and NV12
  *
  * @param[in] image_buffer defines the image buffer
  * @param[in] width of image
  * @param[in] height of image   
  * @param[in] image_rotation defines rotation angle of image
  * @param[in] image_format of given image(one of types in ImageFormat)
  *
  * @return bool status of inference
  */
 
  bool __attribute__((visibility("default"))) UnifiedObjectDetectorWrapper_execute_with_tracking(UnifiedDetectorWrapper* handle,UNIFIED_DETECTOR_MODE_WRAPPER mode_param, UNIFIED_DETECTOR_EXECUTE_WRAPPER execute_params, char*file_name){
    LOGV("VZ Debug: UnifiedObjectDetectorWrapper_execute_with_tracking c wrapper" );
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode= static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode> (mode_param.mode);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat>(execute_params.imageFormat);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM param;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE m_mode;
    m_mode.mode = mode;
    param.image_rotation = execute_params.image_rotation;  
    param.imageFormat = image_format; 
    param.width = execute_params.width;
    param.height = execute_params.height;
    param.image_buffer = execute_params.image_buffer;
    param.zoomType = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType>(execute_params.zoomType);
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->execute_with_tracking(m_mode, param,file_name);
}
 
 /**
  * @brief Interface to check text is present or not on scene
  *
  * @param[in] none
  *
  * @return true if text is present or else false
  */ 
 int __attribute__((visibility("default"))) UnifiedDetectorWrapper_contains_doc_text(UnifiedDetectorWrapper* handle){
     LOGV("VZ Debug: UnifiedDetectorWrapper_contains_doc_text c wrapper");
     return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->contains_doc_text();
 }
 
 /**
  * @brief Interface to bet the ROI bixes
  * @details extracts all the boxes post detection
  *
  * @param[in] none
  *
  * @return number of boxes
  */ 
 int __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_n_boxes(UnifiedDetectorWrapper* handle){
     LOGV("VZ Debug: UnifiedDetectorWrapper_get_n_boxes c wrapper");
     return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_n_boxes();
 }

 /**
  * @brief Interface to bet the ROI bixes
  * @details extracts all the boxes post detection
  *
  * @param[in] none
  *
  * @return number of boxes
  */ 
  int __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_n_tags(UnifiedDetectorWrapper* handle,int max_scene_infos){
    LOGV("VZ Debug: UnifiedDetectorWrapper_get_n_boxes c wrapper");
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_n_tags(max_scene_infos);
}
 
 /**
  * @brief ROI box details
  * @details extracts all the boxes post detection
  *
  * @param[in] box_infos contains the details of ROI
  * @param[in] size of box_infos
  *
  * @return bool status
  */
 bool __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_tag_info(UnifiedDetectorWrapper* handle,TagInfo tag_infos[], int n){
     LOGV("VZ Debug: UnifiedDetectorWrapper_get_box_info c wrapper" );
     return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_tag_info(reinterpret_cast<unified_detector::UnifiedDetector::TagInfo*> (tag_infos), n); 
 }

  //Vex Changes
 long __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_supported_mode_count(UnifiedDetectorWrapper* handle){
     LOGV("VZ Debug: UnifiedDetectorWrapper_get_supported_mode_count c wrapper" );
     return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_supported_mode_count();
 }

  int __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_supported_modes(UnifiedDetectorWrapper* handle, int* modes){
    LOGV("VZ Debug: UnifiedDetectorWrapper_get_supported_modes c wrapper" );
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_supported_modes(modes);
 }

 long __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_supported_tag_count(UnifiedDetectorWrapper* handle, int mode){
    LOGV("VZ Debug: UnifiedDetectorWrapper_get_supported_tag_count c wrapper");
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_supported_tag_count(mode);
 }

 int __attribute__((visibility("default"))) UnifiedDetectorWrapper_get_supported_tags(UnifiedDetectorWrapper* handle, int mode, const char** tags){
    LOGV("VZ Debug: UnifiedDetectorWrapper_get_supported_tags c wrapper");
    return reinterpret_cast<unified_detector::UnifiedDetector*>(handle)->get_supported_tags(mode, tags);
 }