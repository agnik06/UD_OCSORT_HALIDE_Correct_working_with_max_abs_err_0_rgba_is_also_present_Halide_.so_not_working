/**
 * @file   image_utils.hpp
 * @brief  This file implements the image utils Interface
 *
 * This file implements image utils Interface to be used by end api's
 */
#ifndef __IMAGE_UTILS__
#define __IMAGE_UTILS__

#include <opencv2/opencv.hpp>

#include "unified_detector.h"
#include <string>
#include "logger/timer.hpp"
#include "image_utils_neon.hpp"
#include "framework/tensor.hpp"
#include "framework/rectangle.hpp"

using byte = unsigned char;

/**
 * @brief Checks if imag is dark
 *
 * @param[in] image on which check is needed
 *
 * @return true if image is dark
 */
bool is_image_dark(cv::Mat image);
/**
 * @brief gets the bgr mat from buffer
 *
 * @param[in] bitmap buffer
 * @param[in] idth and height
 * @param[in] Image format
 *
 * @return true if image is dark
 */
cv::Mat get_bgr_mat_from_buffer(char* bitmap_buffer, const int width, const int height, const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat format,int& target_width,int& target_height);
/**
 * @brief Rotates the image based on the clockwise rotation
 *
 * @param[in] image on which rotation needs to be applied
 * @param[in] rotation angle (based on clockwise reference)
 *
 * @return rotated image
 */
cv::Mat rotate_image(cv::Mat image, int clockwise_angle);
/**
 * @brief Applies exifcorrection on the input image
 *
 * @param[in] image on which exifcorrection needs to be applied
 * @param[in] orientation of the image
 *
 * @return rotated image
 */
cv::Mat exif_correction(cv::Mat image, const int orientation);
cv::Rect get_4_3_center_crop_rect(cv::Mat image);
/**
 * @brief updates buffer from the mask
 *
 * @param[in] buffer that is updated
 * @param[in] mask containing the segmentation data
 *
 * @return none
 */
void fill_mask_buffer(std::string& buffer, cv::Mat mask);
std::string get_type_string(int type);
cv::Vec3b get_bgr_pixel_yuv(char* yuv, const int image_width, int const image_height, int i, int j);
cv::Vec3b get_bgr_pixel_yvu(char* yuv, const int image_width, int const image_height, int i, int j);
cv::Mat nv21_convert_resize_rotate(char* yuv, const int image_width, int const image_height, int const target_width, int const target_height, int rotation_angle);
bool checkSimilarColor(const cv::Mat &img);
/**
 * @brief Resize and rotate the given image
 * 
 * @param[in] Image buffer with yuv format
 * @param[in] input image width
 * @param[in] input image height
 * @param[in] converted image width
 * @param[in] converted image height
 * @param[in] rotated angle
 * @param[in] image format
 *
 * @return Rotated Image
 */
 cv::Mat nv_convert_resize_rotate(char* yuv, const int image_width, 
    int const image_height, int const target_width, 
    int const target_height, int rotation_angle, 
    const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat format);
/**
 * @brief Get bgr mat from image buffer
 * 
 * @param[in] bitmap buffer of image
 * @param[in] width of image
 * @param[in] height of image
 * @param[in] Image format
 * @param[out] target image width
 * @param[out] target image height
 *
 * @return Converted image 
 */


 cv::Mat preprocess_image(char* image_buffer,int width,int height,int target_width,int target_height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format);
 cv::Mat get_cropped_image(cv::Mat image, unified_detector::Rectangle crop_rect);
 cv::Mat& cropResizeBGR(char* image, int width, int height, int left, int top, int right, int bottom, int target_width, int target_height, cv::Mat& preprocessed_image);
 cv::Mat normalize_image(cv::Mat uchar_image, std::vector<float> means, std::vector<float> scales);

// =============================================================================
//  Detector preprocessing helper — Halide-accelerated when USE_HALIDE_PREPROCESS
//  is defined and the input is NV21/NV12 at 384x384, OpenCV fallback otherwise.
//
//  Implementation: src/utils/halide_preprocess.cpp
// =============================================================================
struct DetectorPreprocessResult {
    cv::Mat bgr_u8;     // 384x384 CV_8UC3 — used by DUMPBOX dumps and ROI code.
    cv::Mat input_f32;  // 384x384 CV_32FC3 — fed directly to SNAP (NHWC interleaved,
                        //                    value = bgr - 128.0f).
};

bool preprocess_detector_input_halide(
    char* image_buffer, int width, int height,
    int target_width, int target_height,
    int image_rotation,
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,
    DetectorPreprocessResult& result);

 #endif //__IMAGE_UTILS__