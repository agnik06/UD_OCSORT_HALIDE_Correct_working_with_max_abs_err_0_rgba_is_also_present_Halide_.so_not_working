#include "jni_wrapper_utils.hpp"
#include "logger/logger.hpp"
#include "opencv2/opencv.hpp"

std::string convert_box_info_to_json_string(unified_detector::UnifiedDetector::TagInfo box_infos[], int n_boxes){
    json j_arr = json::array();
    for(int i=0; i<n_boxes; i++){
        json j_obj;
        j_obj["track_id"] = box_infos[i].trackid;
        j_obj["tag"]    = std::string(box_infos[i].tag); 
        j_obj["tag_id"] = (int)box_infos[i].tag_class;
        j_obj["left"]   = box_infos[i].left;
        j_obj["top"]    = box_infos[i].top;
        j_obj["right"]  = box_infos[i].right;
        j_obj["bottom"] = box_infos[i].bottom;
        j_obj["image_width"]  = box_infos[i].image_width;
        j_obj["image_height"] = box_infos[i].image_height;
        j_obj["score"] = box_infos[i].score;
        j_arr.push_back(j_obj);
    }
    return j_arr.dump();
}

unified_detector::UnifiedDetector* get_od_engine_reference(JNIEnv *env, jobject obj){
    jclass   cls      = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "od_engine_reference", "J");
    jlong    lValue   = env->GetLongField(obj, field_id);
    return (unified_detector::UnifiedDetector*)lValue;
}

unified_detector::UnifiedDetector* get_cls_engine_reference(JNIEnv *env, jobject obj){
    jclass   cls      = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "cls_engine_reference", "J");
    jlong    lValue   = env->GetLongField(obj, field_id);
    return (unified_detector::UnifiedDetector*)lValue;
}

unified_detector::UnifiedDetector* get_od_cls_engine_reference(JNIEnv *env, jobject obj){
    jclass   cls      = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "od_cls_engine_reference", "J");
    jlong    lValue   = env->GetLongField(obj, field_id);
    return (unified_detector::UnifiedDetector*)lValue;
}

void set_od_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector){
    jclass cls        = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "od_engine_reference", "J");
    env->SetLongField(obj, field_id, (jlong)object_detector);
}

void set_cls_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector){
    jclass cls        = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "cls_engine_reference", "J");
    env->SetLongField(obj, field_id, (jlong)object_detector);
}

void set_od_cls_engine_reference(JNIEnv *env, jobject obj, unified_detector::UnifiedDetector* object_detector){
    jclass cls        = env->GetObjectClass(obj);
    const char* field_signature = "[Lvizinsight/atl/image_segmenter/VZImageSegmenter;";
    jfieldID field_id = env->GetFieldID(cls, "od_cls_engine_reference", "J");
    env->SetLongField(obj, field_id, (jlong)object_detector);
}
