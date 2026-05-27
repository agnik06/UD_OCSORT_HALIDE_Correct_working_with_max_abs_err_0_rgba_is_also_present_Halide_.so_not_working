#include <jni.h>

#include "unified_detector.h"
#include "json.hpp"
#include <memory>
#include <string>

using json = nlohmann::json;

std::string convert_box_info_to_json_string(unified_detector::UnifiedDetector::TagInfo box_infos[], int n_boxes);
unified_detector::UnifiedDetector* get_od_engine_reference(JNIEnv *env, jobject obj);
unified_detector::UnifiedDetector* get_cls_engine_reference(JNIEnv *env, jobject obj);
unified_detector::UnifiedDetector* get_od_cls_engine_reference(JNIEnv *env, jobject obj);
void set_od_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector);
void set_cls_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector);
void set_od_cls_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector);
