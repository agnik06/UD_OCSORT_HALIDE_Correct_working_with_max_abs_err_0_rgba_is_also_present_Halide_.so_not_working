# =============================================================================
# Android.halide_only.mk
#
# Builds ONLY the standalone Halide preprocessing shared library:
#     libud_halide_preprocess.so
#
# It does NOT build (and does NOT need) the engine, the JNI bridge, OpenCV, or
# libc++ — the only inputs are the three Halide AOT .a archives + this stub.
# Use this to produce the .so in isolation once you have the .a files.
#
# Build:
#   cd android/unified_detector_jni/src/jni
#   $PATH_TO_NDK/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.halide_only.mk \
#       APP_ABI=arm64-v8a APP_PLATFORM=android-24 NDK_LIBS_OUT=./out_libs NDK_OUT=./out_obj
#   # result: out_libs/arm64-v8a/libud_halide_preprocess.so
# =============================================================================
LOCAL_PATH := $(call my-dir)
ENGINE_BASE_PATH             := $(LOCAL_PATH)/../../../../unified_detector
ENGINE_THIRD_PARTY_LIBS_PATH := $(ENGINE_BASE_PATH)/third_party

#------------------------------------------------------
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

#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := ud_halide_preprocess
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := halide_so_stub.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := UD_HALIDE_AOT UD_RGBA2BGR_AOT UD_ROTATE_NORMALIZE_AOT
LOCAL_LDLIBS := -llog
LOCAL_LDFLAGS := -fuse-ld=lld
LOCAL_LDFLAGS += "-Wl,-z,max-page-size=16384"
include $(BUILD_SHARED_LIBRARY)
