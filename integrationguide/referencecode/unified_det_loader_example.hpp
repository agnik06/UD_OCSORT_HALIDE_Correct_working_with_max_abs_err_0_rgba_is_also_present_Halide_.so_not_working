#ifndef __UNIFIED_DETECTOR_LOADER_HPP__
#define __UNIFIED_DETECTOR_LOADER_HPP__


#include "unified_detector_wrapper.h"

void* load_unifieddetector_lib_handle();
UnifiedDetectorWrapper* (*get_create_fptr(void* handle)) ();
bool (*get_initialize_fptr(void* handle)) (UnifiedDetectorWrapper*,UNIFIED_DETECTOR_MODE_WRAPPER::MODE , char*);
bool (*get_execute_fptr(void* handle)) (UnifiedDetectorWrapper*, UNIFIED_DETECTOR_MODE_WRAPPER::MODE, UNIFIED_DETECTOR_EXECUTE_WRAPPER);
int (*get_n_tags_fptr(void* handle)(UnifiedDetectorWrapper*,int));
bool (*get_tag_info_fptr(void* handle)(UnifiedDetectorWrapper*, TagInfo*));
void (*get_destroy_fptr(void* handle)) (UnifiedDetectorWrapper*);
void (*unload_unified_det_lib_handle(void* handle)) ();

#endif 
