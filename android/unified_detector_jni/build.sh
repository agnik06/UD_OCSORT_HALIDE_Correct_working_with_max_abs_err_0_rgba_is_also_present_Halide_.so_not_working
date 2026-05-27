ENGINE_SRC="../../unified_detector/src"
touch $ENGINE_SRC/unified_detector_impl.cpp
touch src/jni/UnifiedDetectorJNI.cpp
cd src/jni
# export PATH_TO_NDK=~/Android/android-ndk-r20
$PATH_TO_NDK/ndk-build -j16
cd -
mkdir -p libs
cp -r src/libs/* libs/

#Changes for HWASAN build from here
#mkdir -p ./libs_HWASAN

#cp -r ./libs/arm64-v8a ./libs_HWASAN/

#rm -rf src/libs/arm64-v8a/libSceneSegmenterJNI_HWASAN.so src/libs/arm64-v8a/libSemanticMap_v1_HWASAN.camera.samsung.so
#rm -rf ./libs/arm64-v8a/libSceneSegmenterJNI_HWASAN.so ./libs/arm64-v8a/libSemanticMap_v1_HWASAN.camera.samsung.so
#mv ./libs_HWASAN/arm64-v8a/libSemanticMap_v1_HWASAN.camera.samsung.so ./libs_HWASAN/arm64-v8a/libSemanticMap_v1.camera.samsung.so
#mv ./libs_HWASAN/arm64-v8a/libSceneSegmenterJNI_HWASAN.so ./libs_HWASAN/arm64-v8a/libSceneSegmenterJNI.so
#rm -rf ./libs_HWASAN/arm64-v8a/libc++.so ./libs_HWASAN/arm64-v8a/libOpenCv*

#mv libs Libs
#mv libs_HWASAN Libs_HWASAN
exit $?
