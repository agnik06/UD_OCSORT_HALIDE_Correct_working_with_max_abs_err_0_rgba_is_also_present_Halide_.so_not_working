#ifdef ANDROID_ARM_NEON
#ifndef __IMAGE_UTILS_NEON__
#define __IMAGE_UTILS_NEON__

#include <opencv2/opencv.hpp>
#include <string>

namespace viz_arm_neon{
    bool resize_nv21_or_nv12_buffer(char* yuv, char* yuv_resized, const int image_width, const int image_height, const int target_width, const int target_height, const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format);
    static inline void ics_yuv2rgb_bt709_neon_8I(uint8x8_t y,  uint8x8_t cb,   uint8x8_t cr,   uint8x8_t *r, uint8x8_t *g, uint8x8_t *b);
    static inline void ics_yuv2rgb_bt709_neon(unsigned char y, unsigned char cb, unsigned char cr,
                            unsigned char *r, unsigned char *g, unsigned char *b);
    static inline unsigned char ics_usat8_neon(int a);
    int ics_fconvert_nv21_or_nv12_torgb_neon(unsigned char* in, unsigned char* out, int width, int height, 
        const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format);
    cv::Mat nv21_or_nv12_resize_convert_rotate(char* yuv, const int image_width, int const image_height, int const target_width, int const target_height, int rotation_angle, 
        const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format);
    cv::Mat nv21_or_nv12_convert_resize_rotate(char* yuv, const int image_width, int const image_height, int const target_width, int const target_height, int rotation_angle, 
        const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format);
};

#endif //__IMAGE_UTILS_NEON__
#endif // ANDROID_ARM_NEON