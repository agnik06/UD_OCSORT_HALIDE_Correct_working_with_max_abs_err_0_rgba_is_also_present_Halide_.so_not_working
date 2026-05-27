#include <jni.h>

#include "unified_detector.h"
#include <memory>
#include "json.hpp"
#include "unified_detector_jni_utils.hpp"
#include "opencv2/opencv.hpp"
#include "logger/logger.hpp"
#include "logger/timer.hpp"
#include <string.h>

#define JNI_FUNC(name) Java_vizinsight_atl_unifieddetector_UnifiedDetector_##name
using json = nlohmann::json;

extern "C"{

JavaVM* jvm;
jobject main_obj; 

void init_cb(bool init_status) { 
  LOGV("VZ Debug: Initialization completed JNI mode");
  try{
    JNIEnv* main_env = NULL;
    jint get_env_status;
    jint attach_status = JNI_OK;
    jobject obj = main_obj;
    if(jvm != NULL){
      get_env_status = jvm->GetEnv((void**)&main_env, JNI_VERSION_1_6);//VZ Debug: Exception
      if(get_env_status == JNI_EDETACHED){
        attach_status = jvm->AttachCurrentThread(&main_env, NULL);
        if(attach_status != JNI_OK){
          LOGV("VZ Debug: Failed to attach current thread to jvm");
        }
      }
      else if(get_env_status == JNI_EVERSION){
          LOGV("VZ Debug: Failed to attach current thread to jvm as he JNI version is not supported");
      }
    }
    if(main_env != NULL && obj != NULL && attach_status == JNI_OK){
      jclass cls = main_env->GetObjectClass(obj);
      jmethodID init_cb_id = main_env->GetMethodID(cls, "callback", "(I)V");
      main_env->CallVoidMethod(obj, init_cb_id, init_status);
    }
    if(jvm != NULL && get_env_status == JNI_EDETACHED){
      jvm->DetachCurrentThread();
    }
  } catch (std::exception& ex) {
    LOGV("VZ Debug: Exception %s", ex.what());
  }
}



// unified_detector::UnifiedDetector* object_detector;
JNIEXPORT jboolean JNICALL JNI_FUNC(initializeJNI)(JNIEnv *env, jobject obj, jint mode_num, jstring model_base_path_str) {
    unified_detector::UnifiedDetector* object_detector = unified_detector::UnifiedDetector::Create();
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      set_od_engine_reference(env, obj, object_detector);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      set_cls_engine_reference(env, obj, object_detector);
    }else{
      set_od_cls_engine_reference(env,obj,object_detector);
    }
    env->GetJavaVM(&jvm);
    auto start_time = CURRENT_TIME;
    char* model_base_path = (char*) env->GetStringUTFChars(model_base_path_str, NULL);
    
    main_obj=env->NewGlobalRef(obj);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE param;
    param.mode = mode;
    bool init_status = object_detector->initialize(init_cb,param, model_base_path);
    LG<<"VZ Debug: Time taken for initialization: "<< CURRENT_TIME - start_time<< " ms";
    env->ReleaseStringUTFChars(model_base_path_str, model_base_path);
    return init_status;
}

JNIEXPORT jboolean JNICALL JNI_FUNC(executeJNI)(JNIEnv *env, jobject obj, jobject image_bitmap_buffer,
        jint width, jint height, jint image_rotation, jint image_format_num, jint mode_num,jint zoom_type) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }
            
    auto start_time = CURRENT_TIME;
    char* image_buffer = (char*) env->GetDirectBufferAddress(image_bitmap_buffer);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat>(image_format_num);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM param;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE m_mode;
    m_mode.mode = mode;
    param.image_rotation = image_rotation;  
    param.imageFormat = image_format; 
    param.width = width;
    param.height = height;
    param.image_buffer = image_buffer;
    param.zoomType = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType>(zoom_type);
    bool execute_status = object_detector->execute(m_mode,param);
    LG<<"VZ Debug: Time taken for execution: "<< CURRENT_TIME - start_time<<" ms";

    return execute_status;
}

JNIEXPORT jboolean JNICALL JNI_FUNC(executeWithByteArrayJNI)(JNIEnv *env, jobject obj, jbyteArray imageBytes,
  jint width, jint height, jint image_rotation, jint image_format_num, jint mode_num) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }

    jbyte* byteArray = env->GetByteArrayElements(imageBytes, nullptr); 
    jint length = env->GetArrayLength(imageBytes);
    std::vector<char> data;
    data.reserve(length);
    for (jint i = 0; i < length; ++i) {
      data.push_back(static_cast<char>(byteArray[i]));
    }
    env->ReleaseByteArrayElements(imageBytes, byteArray, JNI_ABORT); // Release the memory          
    auto start_time = CURRENT_TIME;
    //uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(image_bitmap_buffer));
    cv::Mat bgrImage(height, width, CV_8UC3, byteArray);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE m_mode;
    m_mode.mode = mode;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat>(image_format_num);
    bool execute_status = object_detector->execute(m_mode,data.data(), width,height,image_rotation,image_format);
    LG<<"VZ Debug: Time taken for exexecuteWithBtyeArrayJNI execution: "<< CURRENT_TIME - start_time<<" ms";
    

    return execute_status;
}

JNIEXPORT jboolean JNICALL JNI_FUNC(executeWithTrackJNI)(JNIEnv *env, jobject obj, jobject image_bitmap_buffer,
        jint width, jint height, jint image_rotation, jint image_format_num, jint mode_num,jint zoom_type) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }
            
    auto start_time = CURRENT_TIME;
    char* image_buffer = (char*) env->GetDirectBufferAddress(image_bitmap_buffer);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat>(image_format_num);
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM param;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE m_mode;
    m_mode.mode = mode;
    param.image_rotation = image_rotation;  
    param.imageFormat = image_format; 
    param.width = width;
    param.height = height;
    param.image_buffer = image_buffer;
    param.zoomType = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType>(zoom_type);
    bool execute_status = object_detector->execute_with_tracking(m_mode,param);
    LG<<"VZ Debug: executeWithTrackJNI Time taken for execution: "<< CURRENT_TIME - start_time<<" ms";

    return execute_status;
}

JNIEXPORT jstring JNICALL JNI_FUNC(getagInfosJNI)(JNIEnv *env, jobject obj, jint mode_num) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }

    auto start_time = CURRENT_TIME;
    
    int n_boxes = object_detector->get_n_tags(1);
    unified_detector::UnifiedDetector::TagInfo box_infos[n_boxes]; 

    bool extraction_staus = object_detector->get_tag_info(box_infos, n_boxes);
    
    if(!extraction_staus){
        return env->NewStringUTF("");
    }
    std::string result_info_json_str = convert_box_info_to_json_string(box_infos, n_boxes);
    LG<<"VZ Debug: Time taken for result-extraction1: "<< CURRENT_TIME - start_time<<" ms";
    LG<<"VZ Debug: BoxInfo :"<< result_info_json_str;
    jstring result_info_json_utf_str = env->NewStringUTF(result_info_json_str.c_str());
    return result_info_json_utf_str;
}

JNIEXPORT jstring JNICALL JNI_FUNC(getSalienntBoxInfosToCenterJNI)(JNIEnv *env, jobject obj, jint mode_num) {
  unified_detector::UnifiedDetector* object_detector;
  unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
  if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
    object_detector = get_od_engine_reference(env, obj);
  }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
    object_detector = get_cls_engine_reference(env, obj);
  }else{
    object_detector = get_od_cls_engine_reference(env, obj);
  }

  auto start_time = CURRENT_TIME;
  
  int n_boxes = object_detector->get_saliency_boxes_to_center_count();
  unified_detector::UnifiedDetector::TagInfo box_infos[n_boxes]; 

  bool extraction_staus = object_detector->get_tag_info(box_infos, n_boxes);
  
  if(!extraction_staus){
      return env->NewStringUTF("");
  }
  std::string result_info_json_str = convert_box_info_to_json_string(box_infos, n_boxes);
  LG<<"VZ Debug: Time taken for result-extraction1: "<< CURRENT_TIME - start_time<<" ms";
  LG<<"VZ Debug: BoxInfo :"<< result_info_json_str;
  jstring result_info_json_utf_str = env->NewStringUTF(result_info_json_str.c_str());
  return result_info_json_utf_str;
}

JNIEXPORT void JNICALL JNI_FUNC(releaseJNI)(JNIEnv *env, jobject obj, jint mode_num) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }

    auto start_time = CURRENT_TIME;
    if(object_detector != NULL){
        unified_detector::UnifiedDetector::Destroy(object_detector);
        object_detector = NULL;
    }
    LG<<"VZ Debug: Time taken for release: "<< CURRENT_TIME - start_time<<" ms";
}

JNIEXPORT jboolean JNICALL JNI_FUNC(isTextPResentJNI)(JNIEnv *env, jobject obj, jint mode_num) {
    unified_detector::UnifiedDetector* object_detector;
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode mode = static_cast<unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode>(mode_num);
    if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::OD){
      object_detector = get_od_engine_reference(env, obj);
    }else if(mode == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode::CLS){
      object_detector = get_cls_engine_reference(env, obj);
    }else{
      object_detector = get_od_cls_engine_reference(env, obj);
    }
            
    auto start_time = CURRENT_TIME;
    bool execute_status = object_detector->contains_doc_text();
    LG<<"VZ Debug: isTextPResentJNI : text present : "<<execute_status;
    LG<<"VZ Debug: Time taken for execution: "<< CURRENT_TIME - start_time<<" ms";

    return execute_status;
}
        
}; //extern "C"