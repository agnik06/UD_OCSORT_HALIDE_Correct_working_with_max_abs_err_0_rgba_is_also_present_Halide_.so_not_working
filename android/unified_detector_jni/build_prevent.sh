export PATH_TO_NDK=/opt/android-ndk-r21e

ENGINE_SRC="./android/unified_detector_jni/../../unified_detector/src"
touch $ENGINE_SRC/utils/*.hpp
touch $ENGINE_SRC/unified_detector_impl.cpp
touch ./android/unified_detector_jni/src/jni/UnifiedDetectorJNI.cpp
touch ./android/unified_detector_jni/src/jni/jni_wrapper_utils.cpp

cd ./android/unified_detector_jni/src/jni
# export PATH_TO_NDK=~/Android/android-ndk-r20
$PATH_TO_NDK/ndk-build -j16
cd -
mkdir -p ./android/unified_detector_jni/libs
cp -r ./android/unified_detector_jni/src/libs/* ./android/unified_detector_jni/libs/

exit $?
