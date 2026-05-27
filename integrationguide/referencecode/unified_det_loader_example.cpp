#include <dlfcn.h>
#include "logger.hpp"
#include <stdexcept>

#include "unified_det_loader_example.hpp"


void* load_unifieddetector_lib_handle() {
	void* unifieddetector_lib_handle = NULL;

  unifieddetector_lib_handle = dlopen("./libUnifiedDetector_v1.camera.samsung.so", RTLD_NOW | RTLD_GLOBAL);
  if (unifieddetector_lib_handle != NULL) {
    LOGV("VZ Debug: Dynamic loading of UnifiedDetector successful from current executable path");
  }
  else{ 
    LOGV("VZ Debug: Dynamic loading of UnifiedDetector Failed unifieddetector_lib_handle is null");
  }


  if (unifieddetector_lib_handle == NULL) {
    LOGV("VZ Debug: Dynamic loading of LibUnifiedDetectorInterface failed.\nERROR: %s", dlerror());

    throw std::runtime_error("Dynamic loading of LibUnifiedDetectorInterface failed.");
  }
  return unifieddetector_lib_handle;
}


UnifiedDetectorWrapper* (*get_create_fptr(void* handle)) () {
	auto UnifiedDetectorWrapper_create_ptr = (UnifiedDetectorWrapper* (*)()) (dlsym(handle, "UnifiedDetectorWrapper_create"));
	if(UnifiedDetectorWrapper_create_ptr == NULL) {
		LOGV("VZ Debug: UnifiedDetectorWrapper_create symbol not found");
	}
	return UnifiedDetectorWrapper_create_ptr;
}

bool (*get_initialize_fptr(void* handle)) (UnifiedDetectorWrapper*,UNIFIED_DETECTOR_MODE_WRAPPER::MODE , char*){
    auto UnifiedDetectorWrapper_initialie_ptr = (bool (*)(UnifiedDetectorWrapper*, UNIFIED_DETECTOR_MODE_WRAPPER::MODE, char*)) (dlsym(handle, "UnifiedDetectorWrapper_Initialize"));
	if(UnifiedDetectorWrapper_initialie_ptr == NULL) {
		LOGV("VZ Debug: UnifiedDetectorWrapper_Initialize symbol not found");
	}
	return UnifiedDetectorWrapper_initialie_ptr;
}

bool (*get_execute_fptr(void* handle)) (UnifiedDetectorWrapper*, UNIFIED_DETECTOR_MODE_WRAPPER::MODE, UNIFIED_DETECTOR_EXECUTE_WRAPPER){
    auto UnifiedDetectorWrapper_execute_ptr = (bool (*)(UnifiedDetectorWrapper*, UNIFIED_DETECTOR_MODE_WRAPPER::MODE, UNIFIED_DETECTOR_EXECUTE_WRAPPER)) (dlsym(handle, "UnifiedDetectorWrapper_execute"));
	if(UnifiedDetectorWrapper_execute_ptr == NULL) {
		LOGV("VZ Debug: UnifiedDetectorWrapper_execute symbol not found");
	}
	return UnifiedDetectorWrapper_execute_ptr;
}

int (*get_n_tags_fptr(void* handle)(UnifiedDetectorWrapper*,int)){
	auto UnifiedDetectorWrapper_get_n_tags_ptr = (int (*)(UnifiedDetectorWrapper*, int)) (dlsym(handle, "UnifiedDetectorWrapper_get_n_tags"));
	return UnifiedDetectorWrapper_get_n_tags_ptr;
}

bool (*get_tag_info_fptr(void* handle)) (UnifiedDetectorWrapper*,TagInfo*){

    auto UnifiedDetectorWrapper_tag_info_ptr = (bool (*)(UnifiedDetectorWrapper*,TagInfo*)) (dlsym(handle, "UnifiedDetectorWrapper_get_tag_info"));
	if(UnifiedDetectorWrapper_tag_info_ptr == NULL) {
		LOGV("VZ Debug: UnifiedDetectorWrapper_tag_info_ptr symbol not found");
	}
	return 	 UnifiedDetectorWrapper_tag_info_ptr;

}

	

void (*get_destroy_fptr(void* handle)) (UnifiedDetectorWrapper*) {
	auto UnifiedDetectorWrapper_destroy_ptr = (void (*)(UnifiedDetectorWrapper*)) (dlsym(handle, "UnifiedDetectorWrapper_destroy"));
	if(UnifiedDetectorWrapper_destroy_ptr == NULL) {
		LOGV("VZ Debug: UnifiedDetectorWrapper_destroy symbol not found");
	}
}

void (*unload_unifieddetector_lib_handle(void* handle)) () {
	
	dlclose(handle);
    LOGV("VZ Debug: Dynamic unloading of LibUnifiedDetector successful from current executable path");
  
}

