MODELS_DIR=models/unified_detector
DEVICE_MODELS_DIR=/sdcard/saiv/image_understanding/db/unified_detector
#DEVICE_MODELS_DIR=/vendor/saiv/image_understanding/db/unified_detector

ENGINE_DIR=../unified_detector_jni

#APP_DIR=../image-segmentation/android/applications/MaskOn
#APP_NAME=MaskOn

APP_DIR=../applications/SceneOptimizerPro
APP_NAME=UnifiedDetectorTestApp
AAR_NAME=UnifiedDetector

RELEASE_DIR=./releases/unified_detector
ARCH=(
	arm64-v8a 
	armeabi-v7a
	#x86_64 
)

export PATH_TO_ADB=~/Android/Sdk/platform-tools
export ANDROID_HOME=~/Android/Sdk/

OUTPUT_LIBS_DIR=$ENGINE_DIR/libs
#DLI_LIBS_DIR=libs/dlinterface
DLI_LIBS_DIR=../../unified_detector/third_party/DLInterface/libs
DEPLOY_LIBS=(
	$OUTPUT_LIBS_DIR/$libs_type/libOpenCv.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libOpenCv_clang.so
	$OUTPUT_LIBS_DIR/$libs_type/libc++.so
	$DLI_LIBS_DIR/$libs_type/libDLInterface.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libSceneSegmenter_v1.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libgalaxyraw_v1.camera.samsung.so
	$OUTPUT_LIBS_DIR/$libs_type/libUnifiedDetector.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libAFSegmenter_v1.camera.samsung.so

	$OUTPUT_LIBS_DIR/$libs_type/libSceneSegmenterJNI.so	
)

RELEASE_LIBS=(
	$OUTPUT_LIBS_DIR/$libs_type/libOpenCv.camera.samsung.so
	$OUTPUT_LIBS_DIR/$libs_type/libc++.so
	$DLI_LIBS_DIR/$libs_type/libDLInterface.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libgalaxyraw_v1.camera.samsung.so
	$OUTPUT_LIBS_DIR/$libs_type/libUnifiedDetector.camera.samsung.so
	#$OUTPUT_LIBS_DIR/$libs_type/libAFSegmenter_v1.camera.samsung.so
	$OUTPUT_LIBS_DIR/$libs_type/libSceneSegmenterJNI.so	
)

time_stamp=$(date +%Y-%m-%d-%T)
RED='\033[0;31m'
NC='\033[0m' 
BLUE='\033[0;34m'

#Get device id
device_id=""
while getopts i:s:d option #not 'p:' as p doesn't need value
do
 case "${option}"
 in
 s) device_id=${OPTARG};;
 esac
done

if [ "$device_id" == "" ]; then
	ADB=adb
else
    echo "Using Device: $device_id"
	ADB="adb -s $device_id"
fi

mkdir -p apps
mkdir -p models
mkdir -p libs
mkdir -p releases
