LOCAL_PATH := $(call my-dir)
ENGINE_BASE_PATH 				:= $(LOCAL_PATH)/../../../../unified_detector
ENGINE_SRC_PATH 				:= $(ENGINE_BASE_PATH)/src
ENGINE_THIRD_PARTY_LIBS_PATH 	:= $(ENGINE_BASE_PATH)/third_party
THIRD_PARTY_LIBS_PATH 			:= $(LOCAL_PATH)/../../third_party

#INCLUDE PATHS
OPENCV_INC_PATH 		:= $(THIRD_PARTY_LIBS_PATH)/pcv340/include
STL_INCLUDE 			:= $(THIRD_PARTY_LIBS_PATH)/stl_libc++/include/libcxx_inc
JSON_CPP_INCLUDE 		:= $(ENGINE_THIRD_PARTY_LIBS_PATH)/json
DL_INTERFACE_INCLUDE 	:= $(ENGINE_THIRD_PARTY_LIBS_PATH)/DLInterface/include/
EIGEN_CPP_INCLUDE       := $(ENGINE_THIRD_PARTY_LIBS_PATH)/Eigen/
IMAGE_SEGMENTER_INCLUDE := $(ENGINE_BASE_PATH)/include/unified_detector
ENGINE_CORE_INCLUDE 	:= $(ENGINE_SRC_PATH)/core
ENGINE_SRC_INCLUDE	 	:= $(ENGINE_SRC_PATH)/
ENGINE_TRACKER_INCLUDE  := $(ENGINE_SRC_PATH)/detector/tracker
#HOTA_SRC_INCLUDE		:= $(ENGINE_SRC_PATH)/utils/hota
#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := OPENCV_HQ
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := $(THIRD_PARTY_LIBS_PATH)/pcv340/libs/$(TARGET_ARCH_ABI)/libOpenCv.camera.samsung.so
include $(PREBUILT_SHARED_LIBRARY)

#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := STL_LIBC++
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := $(THIRD_PARTY_LIBS_PATH)/stl_libc++/libs/$(TARGET_ARCH_ABI)/libc++.so
include $(PREBUILT_SHARED_LIBRARY)

#------------------------------------------------------
# Halide AOT static archives (produced by unified_detector/halide_gen, Stage 1).
# These remain PREBUILT_STATIC_LIBRARY here, but instead of being fused into the
# engine they are whole-archived into the standalone ud_halide_preprocess .so below.
include $(CLEAR_VARS)
LOCAL_MODULE := UD_HALIDE_AOT
LOCAL_SRC_FILES := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/libs/$(TARGET_ARCH_ABI)/libud_preprocess_aot.a
include $(PREBUILT_STATIC_LIBRARY)

#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := UD_RGBA2BGR_AOT
LOCAL_SRC_FILES := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/libs/$(TARGET_ARCH_ABI)/libud_rgba2bgr_aot.a
include $(PREBUILT_STATIC_LIBRARY)

#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := UD_ROTATE_NORMALIZE_AOT
LOCAL_SRC_FILES := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/libs/$(TARGET_ARCH_ABI)/libud_rotate_normalize_aot.a
include $(PREBUILT_STATIC_LIBRARY)

#======================================================================================
# >>> HALIDE-SHARED CHANGE (1/2) — standalone Halide preprocessing shared library.
#
# Builds: libud_halide_preprocess.so
#
# LOCAL_WHOLE_STATIC_LIBRARIES pulls the three AOT archives in WHOLE, so every
# symbol is retained and exported even though halide_so_stub.cpp references none
# of them:
#     ud_preprocess_aot        (NV21/NV12 fused kernel)
#     ud_rgba2bgr_aot          (RGBA -> BGR)
#     ud_rotate_normalize_aot  (rotate + normalize)
#     halide_*                 (Halide runtime, carried by libud_preprocess_aot.a)
#
# The engine then links THIS .so dynamically (see change 2/2) instead of statically
# fusing the .a — so the Halide preprocessing is a separate, swappable .so.
#======================================================================================
include $(CLEAR_VARS)
LOCAL_MODULE := ud_halide_preprocess
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := halide_so_stub.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := UD_HALIDE_AOT UD_RGBA2BGR_AOT UD_ROTATE_NORMALIZE_AOT
LOCAL_LDLIBS := -llog
LOCAL_LDFLAGS := -fuse-ld=lld
LOCAL_LDFLAGS += "-Wl,-z,max-page-size=16384"
include $(BUILD_SHARED_LIBRARY)

#------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := UnifiedDetector.camera.samsung
LOCAL_ARM_MODE := arm

ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
    LOCAL_CFLAGS += -march=armv8-a
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_SYSTEM_HIDL='"/system/lib64/libDLInterface_hidl.camera.samsung.so"'
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_SYSTEM_AIDL='"/system/lib64/libDLInterface_aidl.camera.samsung.so"'
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_VENDOR='"/vendor/lib64/libDLInterface.camera.samsung.so"'
else
    LOCAL_CFLAGS += -mfpu=neon -march=armv7-a -DSS_NEON_ASSEMBLY -Da32_build
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_SYSTEM_HIDL='"/system/lib/libDLInterface_hidl.camera.samsung.so"'
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_SYSTEM_AIDL='"/system/lib/libDLInterface_aidl.camera.samsung.so"'
    LOCAL_CFLAGS += -DDLINTERFACELIBLOCATION_VENDOR='"/vendor/lib/libDLInterface.camera.samsung.so"'
endif

LOCAL_CFLAGS += -D__android__
LOCAL_CFLAGS += -D__ISEG_VERSION__='"v1"'
LOCAL_CFLAGS += -D__UNIFIED_DETECTOR_VERSION__=v1.6.7
LOCAL_CFLAGS += -DLIBBUILDMODE='"default"'

LOCAL_CFLAGS += -fno-var-tracking
LOCAL_CFLAGS += -DANDROID_ARM_NEON=TRUE
LOCAL_CFLAGS += -Wno-non-c-typedef-for-linkage
LOCAL_CFLAGS += -Wno-return-type-c-linkage
LOCAL_CFLAGS += -Wvla-extension
LOCAL_CFLAGS += -fstack-protector-strong
LOCAL_CFLAGS += -O3
LOCAL_CFLAGS     += -DUSE_HALIDE_PREPROCESS
LOCAL_LDFLAGS := -fuse-ld=lld
LOCAL_LDLIBS := -lz -llog -lc -pthread
LOCAL_LDFLAGS += "-Wl,-z,max-page-size=16384,-O3"

LOCAL_SHARED_LIBRARIES += OPENCV_HQ
LOCAL_SHARED_LIBRARIES += STL_LIBC++
# >>> HALIDE-SHARED CHANGE (2/2) — link the Halide preprocessing DYNAMICALLY.
# Original (static fuse) was:
#     LOCAL_STATIC_LIBRARIES += UD_HALIDE_AOT
#     LOCAL_STATIC_LIBRARIES += UD_RGBA2BGR_AOT
#     LOCAL_STATIC_LIBRARIES += UD_ROTATE_NORMALIZE_AOT
LOCAL_SHARED_LIBRARIES += ud_halide_preprocess

HALIDE_INC_PATH := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/include
LOCAL_C_INCLUDES := $(OPENCV_INC_PATH) \
					$(JSON_CPP_INCLUDE) \
					$(DL_INTERFACE_INCLUDE) \
					$(EIGEN_CPP_INCLUDE)\
					$(IMAGE_SEGMENTER_INCLUDE) \
					$(ENGINE_CORE_INCLUDE) \
					$(ENGINE_SRC_INCLUDE) \
					$(ENGINE_TRACKER_INCLUDE) \
					$(STL_INCLUDE) \
					$(HALIDE_INC_PATH)

FILE_LIST := $(wildcard $(ENGINE_SRC_PATH)/*.cpp)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/*.cpp)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/**/*.cpp)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/**/**/*.cpp)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/**/**/**/*.cpp)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/*.cc)
FILE_LIST += $(wildcard $(ENGINE_SRC_PATH)/**/**/*.cc)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
include $(BUILD_SHARED_LIBRARY)

#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := SceneSegmenterJNI
LOCAL_ARM_MODE := arm

LOCAL_CFLAGS += -D__android__
LOCAL_CFLAGS += -D__ISEGJNI_VERSION__='"v1"'
LOCAL_CFLAGS += -DLIBBUILDMODE='"default"'
LOCAL_CFLAGS += -Wno-non-c-typedef-for-linkage
LOCAL_CFLAGS += -Wno-return-type-c-linkage
LOCAL_CFLAGS += -Wvla-extension
LOCAL_CFLAGS += -fstack-protector-strong

LOCAL_LDFLAGS := -fuse-ld=lld
LOCAL_LDLIBS := -lz -llog -lc -pthread
LOCAL_LDFLAGS += "-Wl,-z,max-page-size=16384"

LOCAL_SHARED_LIBRARIES += OPENCV_HQ
LOCAL_SHARED_LIBRARIES += UnifiedDetector.camera.samsung
LOCAL_SHARED_LIBRARIES += STL_LIBC++

LOCAL_C_INCLUDES := $(OPENCV_INC_PATH) \
					$(JSON_CPP_INCLUDE) \
					$(STL_INCLUDE) \
					$(IMAGE_SEGMENTER_INCLUDE) \
					$(ENGINE_CORE_INCLUDE)

LOCAL_SRC_FILES := $(LOCAL_PATH)/jni_wrapper_utils.cpp $(LOCAL_PATH)/UnifiedDetectorJNI.cpp
include $(BUILD_SHARED_LIBRARY)
