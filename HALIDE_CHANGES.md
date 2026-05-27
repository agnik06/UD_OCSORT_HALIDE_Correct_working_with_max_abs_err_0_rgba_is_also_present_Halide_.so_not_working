# Halide preprocessing optimization — changeset reference

This document lists every file added or modified to introduce Halide-based
fused preprocessing (NV21/NV12 -> BGR resize + rotate + mean-subtract) for
the detector path. Use it as a recipe for replicating the same changes in
another codebase with the same shape.

The changes are **safe to merge as-is** — without the AOT artifacts in
`unified_detector/third_party/halide/`, the wrapper falls back to the
existing `preprocess_image` + `normalize_image` path and behavior is
bit-identical to before. To actually enable Halide, follow the build
steps in `unified_detector/halide_gen/README.md`.

---

## Status overview

| What | Where | Done? |
|------|-------|-------|
| Halide Generator (host-side DSL) | `unified_detector/halide_gen/ud_preprocess_generator.cpp` | ✓ added |
| Makefile for AOT compile | `unified_detector/halide_gen/Makefile` | ✓ added |
| Build instructions | `unified_detector/halide_gen/README.md` | ✓ added |
| C++ wrapper with OpenCV fallback | `unified_detector/src/utils/halide_preprocess.cpp` | ✓ added |
| Helper struct + decl in image_utils header | `unified_detector/src/utils/image_utils.hpp` | ✓ modified |
| Detector call-site replacement | `unified_detector/src/unified_detector_impl.cpp` | ✓ modified |
| Empty AOT output dirs (placeholders) | `unified_detector/third_party/halide/{include,libs/arm64-v8a}/` | ✓ created (empty) |
| AOT artifacts (the actual `.a` + `.h`) | `unified_detector/third_party/halide/` | **YOU run `make all` in halide_gen/ on Linux** |
| Android.mk wiring | `android/unified_detector_jni/src/jni/Android.mk` | **YOU edit, after AOT step succeeds — see section below** |

---

## File-by-file diff summary

### NEW: `unified_detector/halide_gen/ud_preprocess_generator.cpp`

The Halide DSL that fuses YUV->BGR conversion, bilinear resize to 384x384,
clockwise rotation by 0/90/180/270 degrees, and mean-subtract (`bgr - 128.0f`)
into one pipeline. YUV coefficients are intentionally identical to those in
`viz_arm_neon::ics_yuv2rgb_bt709_neon` (constants 1.403, 0.344, 0.714, 1.773;
no Y-16 offset = full-range BT.601), so the Halide path matches the existing
NEON path numerically within bilinear-resize precision.

Key design choices recorded inline as comments:
- Output layout: `out.dim(0).set_stride(1)` makes channel innermost, matching
  `cv::Mat(384, 384, CV_32FC3)` and SNAP's NHWC `{1, 384, 384, 3}` expectation.
- Schedule: `tile(32, 16) + vectorize(xi, 8) + parallel(yo) + unroll(c)`.
  This is a baseline; tune per-SoC.

### NEW: `unified_detector/halide_gen/Makefile`

Drives the host-side AOT compilation. Targets `arm-64-android` (no extra
features). Outputs go to `../third_party/halide/libs/arm64-v8a/` and
`../third_party/halide/include/`. Run on a Linux/WSL machine with
`HALIDE_ROOT` pointing at a Halide 17.0.1 binary release.

### NEW: `unified_detector/halide_gen/README.md`

Step-by-step instructions for installing Halide, running AOT, and enabling
Halide in the Android build.

### NEW: `unified_detector/src/utils/halide_preprocess.cpp`

The C++ wrapper. Header logic:

```cpp
#ifdef USE_HALIDE_PREPROCESS
    if (NV21 or NV12 && 384x384) {
        // call ud_preprocess_aot()
        // produces input_f32; convertTo() reconstructs bgr_u8
        return true on success
    }
#endif
    // Fallback: bit-exact with original
    bgr_u8   = preprocess_image(...);
    input_f32 = normalize_image(bgr_u8, {128,128,128}, {1,1,1});
```

Includes `HalideBuffer.h` and `ud_preprocess_aot.h` ONLY when the
`USE_HALIDE_PREPROCESS` macro is defined, so this file compiles without
Halide artifacts present.

### MODIFIED: `unified_detector/src/utils/image_utils.hpp`

Added at the bottom of the file (just before `#endif //__IMAGE_UTILS__`):

```cpp
struct DetectorPreprocessResult {
    cv::Mat bgr_u8;     // 384x384 CV_8UC3
    cv::Mat input_f32;  // 384x384 CV_32FC3 — already (bgr - 128.0)
};

bool preprocess_detector_input_halide(
    char* image_buffer, int width, int height,
    int target_width, int target_height,
    int image_rotation,
    unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,
    DetectorPreprocessResult& result);
```

No removals. Existing declarations (`preprocess_image`, `normalize_image`,
etc.) are retained because the Halide wrapper falls back to them.

### MODIFIED: `unified_detector/src/unified_detector_impl.cpp`

**Only one call site changed** — the detector preprocessing block in
`detector_execute()`. The `#ifdef DUMPBOX` block that uses the BGR Mat is
preserved verbatim; only the lines that produced and normalized that Mat
were swapped for the helper.

Before (around line 363-381):
```cpp
std::string detector = "detector";
auto preprocess_start_time = CURRENT_TIME;
cv::Mat image = preprocess_image(image_buffer, width, height,
                                 target_width, target_height,
                                 image_rotation, image_format);
#ifdef DUMPBOX
{ ... imwrite(buffer, image); ... }
#endif
cv::Mat target_image;
target_image = normalize_image(image, {128.0,128.0,128.0}, {1.0,1.0,1.0});
```

After:
```cpp
std::string detector = "detector";
auto preprocess_start_time = CURRENT_TIME;

DetectorPreprocessResult prep;
if (!preprocess_detector_input_halide(image_buffer, width, height,
                                      target_width, target_height,
                                      image_rotation, image_format, prep)) {
    LOGV("VZ Debug: detector preprocess_detector_input_halide failed");
    return false;
}
cv::Mat image = prep.bgr_u8;
#ifdef DUMPBOX
{ ... imwrite(buffer, image); ... }    // unchanged
#endif
cv::Mat target_image = prep.input_f32;
```

The classifier path (around line 481-501) was **NOT** modified. If you
want the same optimization there, the same edit pattern applies, but
notice the classifier path may have a `detector_compute_unit != 3` style
guard around mean-subtraction in some forks — verify before duplicating.

---

## Android.mk wiring (apply MANUALLY after AOT artifacts exist)

File to edit: `android/unified_detector_jni/src/jni/Android.mk`

**Step 1.** Add this block above the main `unified_detector` module
(typically just after the OpenCV/STL prebuilt blocks):

```make
#------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := UD_HALIDE_AOT
LOCAL_SRC_FILES := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/libs/$(TARGET_ARCH_ABI)/libud_preprocess_aot.a
include $(PREBUILT_STATIC_LIBRARY)
```

If `ENGINE_THIRD_PARTY_LIBS_PATH` is not yet defined, add near the top:
```make
ENGINE_THIRD_PARTY_LIBS_PATH := $(ENGINE_BASE_PATH)/third_party
```

(Adjust `$(ENGINE_BASE_PATH)` to whatever variable points at
`unified_detector/` in your Android.mk; in this codebase it's already used
for OpenCV/Eigen.)

**Step 2.** Inside the main `unified_detector` module section, add:

```make
HALIDE_INC_PATH := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/include

LOCAL_C_INCLUDES += $(HALIDE_INC_PATH)
LOCAL_CFLAGS     += -DUSE_HALIDE_PREPROCESS
LOCAL_STATIC_LIBRARIES += UD_HALIDE_AOT
```

DO NOT add `-DUSE_HALIDE_PREPROCESS` until the AOT artifacts physically
exist under `third_party/halide/`. Without those files the include
`"ud_preprocess_aot.h"` in `halide_preprocess.cpp` will fail.

---

## Validation steps before shipping

1. **Build with USE_HALIDE_PREPROCESS undefined first.** This exercises only
   the fallback path. The build should succeed and behavior should be
   identical to today.

2. **Generate the AOT artifacts** (`cd unified_detector/halide_gen && make all`)
   on Linux/WSL.

3. **Build with `-DUSE_HALIDE_PREPROCESS -DHALIDE_NUMERIC_CHECK`.**
   On every detector frame the wrapper will log:
   `VZ Debug: HALIDE_NUMERIC_CHECK max_abs_err=<N>`

   Acceptance bands:
   - `<= 3.0`  -> good. Bilinear resize sample-grid differs slightly from
                  OpenCV's `(x+0.5)*W/W_out - 0.5` convention; ~3 is the
                  worst case.
   - `<= 10`   -> probably fine, but check detection scores haven't moved.
   - `> 10`    -> YUV coefficients drifted from `viz_arm_neon`; recheck the
                  generator constants 1.403 / 0.344 / 0.714 / 1.773.
   - `> 50`    -> rotation map inverted; flip the `select(rotation==...)`
                  arms in `generate()`.

4. **Run end-to-end detection on a known clip.** Bbox/score/track-id should
   match the no-Halide build to within model precision (typically ±0.02 on
   score, ±2 px on bbox edges due to bilinear-vs-OpenCV resampling diff).

5. **Benchmark.** Time `preprocess_start_time` to `preprocess_end_time`
   over 100+ frames in both builds. If Halide is < 25% faster than the
   existing NEON path, **schedule-tune** before shipping; the maintenance
   cost of the extra build step only pays off with a real win.

---

## Replicating to another codebase

If the other codebase has the same shape (same `preprocess_image` /
`normalize_image` API, same NV21/NV12 input, same 384x384 detector),
the recipe is:

1. Copy `unified_detector/halide_gen/` wholesale (all 3 files).
2. Copy `unified_detector/src/utils/halide_preprocess.cpp` wholesale.
3. Apply the `image_utils.hpp` patch (struct + function decl at end of
   the public area).
4. Apply the call-site patch in the equivalent `*_impl.cpp` file. Look
   for the `preprocess_image` -> `normalize_image` block. Keep the
   `#ifdef DUMPBOX` block as-is.
5. Apply the Android.mk patch only after AOT artifacts exist.

Caveats when porting:

- **YUV coefficient match.** Open the target codebase's NEON impl and
  confirm the constants. If they differ from 1.403/0.344/0.714/1.773,
  edit the four `Expr R_/G_/B_` lines in
  `ud_preprocess_generator.cpp` to match. Also check whether the target
  uses a `Y - 16` offset (BT.601 video range) — if yes, replace
  `Y_bilinear` with `(Y_bilinear - 16.0f) * 1.164f` in the BGR formulas.

- **Mean-subtract sometimes conditional.** In some forks the mean is
  only subtracted when `detector_compute_unit != 3` (the NPU path). If
  the target codebase has that guard, the helper needs a parameter
  controlling it; the unconditional `bgr - 128.0f` in the current
  generator would break NPU runs.

- **Target size.** If the target detector is not 384x384, change the
  `set_extent`/`bound`/stride constants in the generator and the
  guard `target_width == 384 && target_height == 384` in the wrapper.

---

## What this changeset does NOT do

- Does not modify the classifier preprocessing path.
- Does not modify the legacy `execute()` overload (the one taking
  pre-decoded `cv::Mat`).
- Does not handle RGBA or BGR inputs through Halide — those drop to the
  OpenCV fallback. Camera fast-path is NV21/NV12 only.
- Does not auto-tune the schedule. The baseline schedule
  (tile 32x16, vectorize 8, parallel yo, unroll c) is a starting point;
  expect to spend 1-3 days tuning per-SoC for a real win over hand-NEON.
