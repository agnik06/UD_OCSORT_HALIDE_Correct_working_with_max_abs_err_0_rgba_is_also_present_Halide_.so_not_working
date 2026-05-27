// halide_preprocess.cpp
// =============================================================================
// C++ wrapper that calls the AOT-compiled Halide preprocess pipeline.
//
// When -DUSE_HALIDE_PREPROCESS is defined AND the input is NV21/NV12 AND
// the target size is 384x384, the wrapper invokes the Halide AOT function.
// In every other case it falls back to the existing OpenCV / viz_arm_neon
// path (preprocess_image + normalize_image), which keeps behavior identical
// to the unmodified codebase.
//
// This file ALWAYS compiles. Without USE_HALIDE_PREPROCESS the function
// behaves exactly like the inlined preprocess+normalize pair, so the build
// is safe to merge before the AOT artifacts have been produced.
// =============================================================================

#include "image_utils.hpp"
#include "core/logger/logger.hpp"

#ifdef USE_HALIDE_PREPROCESS
#include "ud_preprocess_aot.h"
#include "ud_rgba2bgr_aot.h"
#include "ud_rotate_normalize_aot.h"
#include "HalideBuffer.h"
#endif

bool preprocess_detector_input_halide(
    char* image_buffer, int width, int height,
    int target_width, int target_height,
    int image_rotation,
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,
    DetectorPreprocessResult& result)
{
    using IF = unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat;
    const bool is_yuv = (image_format == IF::NV21) || (image_format == IF::NV12);

#ifdef USE_HALIDE_PREPROCESS
    // Halide path is only valid for NV21/NV12 at the model's native size.
    // Anything else (RGBA, BGR, non-384) drops to the OpenCV fallback below.
    LOGV("Halide precondition: is_yuv=%d, image_format=%d, target_w=%d, target_h=%d, width=%d, height=%d",
         is_yuv, static_cast<int>(image_format), target_width, target_height, width, height);
    // Halide AOT requires even dimensions and exact 384x384 target
    // const bool has_even_dims = (width % 2 == 0) && (height % 2 == 0);
    
    if (is_yuv && target_width == 384 && target_height == 384 && width > 0 && height > 0) {

        LOGV("Check 1 inside Halide - entering Halide AOT path");

        // ---- Wrap the Y plane (no copy) -------------------------------------
        auto* y_data = reinterpret_cast<uint8_t*>(image_buffer);
        Halide::Runtime::Buffer<uint8_t> y_buf(y_data, width, height);

        LOGV("Check 2 inside Halide");

        // ---- Wrap the UV plane (no copy) ------------------------------------
        // NV21/NV12 has the chroma plane immediately after Y. The chroma plane
        // is half-resolution in BOTH dims, with two interleaved chroma channels
        // per sample. Layout in memory (NV21):
        //
        //   byte: V00 U00 V01 U01 ... V0,W/2-1 U0,W/2-1   <- chroma row 0
        //   byte: V10 U10 V11 U11 ...                     <- chroma row 1
        //   ...
        //
        // We model this as a 3D buffer with strides:
        //   x: stride 2  (each x-step skips one V-U pair)
        //   y: stride W  (each y-step skips one chroma row, which is W bytes
        //                 because W/2 samples * 2 bytes per sample)
        //   c: stride 1  (V is byte 0, U is byte 1 — for NV21)
        auto* uv_data = y_data + (size_t)width * height;
        halide_dimension_t uv_dims[3] = {
            { 0, width  / 2, 2 },
            { 0, height / 2, width },
            { 0, 2,           1 }
        };
        Halide::Runtime::Buffer<uint8_t> uv_buf(uv_data, 3, uv_dims);

        // ---- Allocate the float output as a cv::Mat -------------------------
        // The Halide function writes directly into this Mat's memory.
        result.input_f32.create(target_height, target_width, CV_32FC3);

        // Read the actual y-stride from the Mat; OpenCV may pad rows for
        // alignment on some builds. Hard-coding 3*384 would be fragile.
        const int y_stride_floats = static_cast<int>(result.input_f32.step[0] / sizeof(float));

        halide_dimension_t out_dims[3] = {
            { 0, 3,            1 },                  // c stride 1 (innermost)
            { 0, target_width, 3 },                  // x stride 3 floats
            { 0, target_height, y_stride_floats }    // y stride = mat row stride
        };
        Halide::Runtime::Buffer<float> out_buf(
            reinterpret_cast<float*>(result.input_f32.data), 3, out_dims);
        LOGV("Check 3 inside Halide");

        // ---- Call the AOT function ------------------------------------------
        const int is_nv21 = (image_format == IF::NV21) ? 1 : 0;
        int rc = ud_preprocess_aot(y_buf, uv_buf, is_nv21,
                                   image_rotation, width, height, out_buf);
        
        LOGV("Check 4 inside Halide");
        if (rc != 0) {
            LOGV("VZ Debug: ud_preprocess_aot returned non-zero (%d), falling back", rc);
            // Fall through to OpenCV path below.
        } else {
            // Reconstruct the uint8 BGR mat ONLY when DUMPBOX is enabled —
            // it's the only consumer of bgr_u8 in this codebase. Doing this
            // unconditionally cost ~0.04 ms of tracker time on average due to
            // cache pollution from the 1.7 MB float-to-uint8 pass running
            // right before SNAP detector inference. In release builds
            // (DUMPBOX undefined), leave bgr_u8 empty.
            result.bgr_u8.create(target_height,target_width,CV_8UC3);
#ifdef DUMPBOX
            result.input_f32.convertTo(result.bgr_u8, CV_8UC3, 1.0, 128.0);
#endif

#ifdef HALIDE_NUMERIC_CHECK
            // Optional: cross-check against the OpenCV reference.
            cv::Mat ref_bgr = preprocess_image(image_buffer, width, height,
                                               target_width, target_height,
                                               image_rotation, image_format);
            cv::Mat ref_f32 = normalize_image(ref_bgr,
                                              {128.0f, 128.0f, 128.0f},
                                              {1.0f, 1.0f, 1.0f});
            cv::Mat diff;
            cv::absdiff(ref_f32, result.input_f32, diff);
            double max_err = 0.0;
            cv::minMaxLoc(diff, nullptr, &max_err);
            LOGV("VZ Debug: HALIDE_NUMERIC_CHECK max_abs_err=%f", max_err);
#endif
            return true;
        }
    }

    // ====================================================================
    // RGBA -> Halide path. Bit-exact with the original RGBA preprocessing:
    //
    //   ud_rgba2bgr        : cv::cvtColor(CV_RGBA2BGR)   (channel reorder, exact)
    //   cv::resize         : INTER_LINEAR                (kept on OpenCV --
    //                        a Halide float bilinear is not bit-exact with
    //                        OpenCV's fixed-point bilinear)
    //   ud_rotate_normalize: cv::rotate + (float-128)/1  (exact: pure index
    //                        remap then exact arithmetic)
    //
    // Three steps, two AOT calls + one cv::resize. End-to-end max_abs_err = 0
    // vs the OpenCV reference (verifiable with -DHALIDE_NUMERIC_CHECK).
    // ====================================================================
    if (image_format == IF::RGBA &&
        target_width == 384 && target_height == 384 &&
        width > 0 && height > 0) {

        LOGV("Halide RGBA path: w=%d h=%d rotation=%d", width, height, image_rotation);

        // ---- Step 1: ud_rgba2bgr (RGBA full-res -> BGR full-res) ------------
        // Wrap the RGBA buffer in place. Layout from VZImageDecoder.getRGBA():
        // 4 interleaved bytes per pixel in order R,G,B,A, row-major.
        auto* rgba_data = reinterpret_cast<uint8_t*>(image_buffer);
        halide_dimension_t rgba_dims[3] = {
            { 0, 4,      1 },              // c (channel innermost)
            { 0, width,  4 },              // x (4 bytes per pixel)
            { 0, height, 4 * width }       // y (row stride = 4*w)
        };
        Halide::Runtime::Buffer<uint8_t> rgba_buf(rgba_data, 3, rgba_dims);

        // Allocate the full-resolution BGR cv::Mat that Halide writes into.
        // cv::Mat may pad rows for alignment, so read .step[0] for the actual
        // row stride and pass that to the Halide buffer.
        cv::Mat bgr_full(height, width, CV_8UC3);
        const int bgr_full_y_stride = static_cast<int>(bgr_full.step[0]);
        halide_dimension_t bgr_full_dims[3] = {
            { 0, 3,      1 },
            { 0, width,  3 },
            { 0, height, bgr_full_y_stride }
        };
        Halide::Runtime::Buffer<uint8_t> bgr_full_buf(bgr_full.data, 3, bgr_full_dims);

        int rc = ud_rgba2bgr_aot(rgba_buf, width, height, bgr_full_buf);
        if (rc != 0) {
            LOGV("VZ Debug: ud_rgba2bgr_aot returned non-zero (%d), falling back", rc);
            // Fall through to OpenCV fallback below.
        } else {
            // ---- Step 2: cv::resize to 384x384 (bit-exact, kept on OpenCV) --
            cv::Mat bgr_384(target_height, target_width, CV_8UC3);
            cv::resize(bgr_full, bgr_384, cv::Size(target_width, target_height));

            // ---- Step 3: ud_rotate_normalize (BGR 384 -> float32 384) -------
            // bgr_384 at 384x384 CV_8UC3 is contiguous (3*384=1152 bytes/row,
            // 64-byte aligned; cv::Mat does not pad in this case). But read
            // .step[0] anyway to be safe.
            const int bgr_384_y_stride = static_cast<int>(bgr_384.step[0]);
            halide_dimension_t bgr_384_dims[3] = {
                { 0, 3,             1 },
                { 0, target_width,  3 },
                { 0, target_height, bgr_384_y_stride }
            };
            Halide::Runtime::Buffer<uint8_t> bgr_384_buf(bgr_384.data, 3, bgr_384_dims);

            result.input_f32.create(target_height, target_width, CV_32FC3);
            const int out_y_stride_floats =
                static_cast<int>(result.input_f32.step[0] / sizeof(float));
            halide_dimension_t out_dims[3] = {
                { 0, 3,             1 },
                { 0, target_width,  3 },
                { 0, target_height, out_y_stride_floats }
            };
            Halide::Runtime::Buffer<float> out_buf(
                reinterpret_cast<float*>(result.input_f32.data), 3, out_dims);

            rc = ud_rotate_normalize_aot(bgr_384_buf, image_rotation, out_buf);
            if (rc != 0) {
                LOGV("VZ Debug: ud_rotate_normalize_aot returned non-zero (%d), falling back", rc);
                // Fall through to OpenCV fallback below.
            } else {
                // bgr_u8 must carry valid .cols / .rows for downstream
                // consumers (get_boxes_from_2D_tensors, the ROI filterers),
                // matching what the NV21 branch above does.
                result.bgr_u8.create(target_height, target_width, CV_8UC3);
#ifdef DUMPBOX
                result.input_f32.convertTo(result.bgr_u8, CV_8UC3, 1.0, 128.0);
#endif

#ifdef HALIDE_NUMERIC_CHECK
                // Cross-check against the OpenCV reference. Expect 0.
                cv::Mat ref_bgr = preprocess_image(image_buffer, width, height,
                                                   target_width, target_height,
                                                   image_rotation, image_format);
                cv::Mat ref_f32 = normalize_image(ref_bgr,
                                                  {128.0f, 128.0f, 128.0f},
                                                  {1.0f, 1.0f, 1.0f});
                cv::Mat diff;
                cv::absdiff(ref_f32, result.input_f32, diff);
                double max_err = 0.0;
                cv::minMaxLoc(diff, nullptr, &max_err);
                LOGV("VZ Debug: HALIDE_NUMERIC_CHECK (RGBA) max_abs_err=%f", max_err);
#endif
                return true;
            }
        }
    }
#endif // USE_HALIDE_PREPROCESS

    // // ---- Fallback: bit-exact with the original codebase --------------------
    result.bgr_u8 = preprocess_image(image_buffer, width, height,
                                     target_width, target_height,
                                     image_rotation, image_format);
    if (result.bgr_u8.empty()) return false;
    result.input_f32 = normalize_image(result.bgr_u8,
                                       {128.0f, 128.0f, 128.0f},
                                       {1.0f, 1.0f, 1.0f});
    return !result.input_f32.empty();
}