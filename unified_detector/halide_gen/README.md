# Halide AOT preprocessing — build instructions

This directory contains the host-side Halide DSL that produces the AOT
static library used by the Android build for fused detector preprocessing
(NV21/NV12 -> BGR + resize + rotate + mean-subtract).

## When you'd use this

Only run these steps when you want to enable Halide preprocessing.
Until then, the Android build falls back to the existing OpenCV /
`viz_arm_neon` path and behaves identically to today.

## Prerequisites

- **Linux or WSL2 Ubuntu**. macOS works too. Windows native does not.
- **Halide 17.0.1** binary release (no `pip install`, no admin rights).
- The Android NDK you already have for the engine build — but you don't
  invoke it from this directory; the AOT generator targets arm64-android
  through Halide's own backend.
- ~1 GB free disk space (Halide tarball + extracted).

## One-time setup

```bash
mkdir -p ~/tools && cd ~/tools
wget https://github.com/halide/Halide/releases/download/v17.0.1/Halide-17.0.1-x86-64-linux-<sha>.tar.gz
tar xf Halide-17.0.1-x86-64-linux-*.tar.gz
mv Halide-17.0.1-x86-64-linux Halide-17.0.1
export HALIDE_ROOT=$HOME/tools/Halide-17.0.1
export LD_LIBRARY_PATH=$HALIDE_ROOT/lib:$LD_LIBRARY_PATH
```

Verify the layout matches what the Makefile expects:

```bash
ls $HALIDE_ROOT/tools/GenGen.cpp        # must exist
ls $HALIDE_ROOT/include/Halide.h        # must exist
ls $HALIDE_ROOT/include/HalideBuffer.h  # must exist
ls $HALIDE_ROOT/lib/libHalide.so        # must exist
```

## Generate the AOT artifacts

```bash
cd unified_detector/halide_gen
make all
```

After the run, you should see:

```
../third_party/halide/libs/arm64-v8a/libud_preprocess_aot.a
../third_party/halide/include/ud_preprocess_aot.h
../third_party/halide/include/HalideBuffer.h
../third_party/halide/include/HalideRuntime.h
```

Sanity check that the Halide runtime is bundled inside the static lib
(this is the default for `-e static_library`):

```bash
ar t ../third_party/halide/libs/arm64-v8a/libud_preprocess_aot.a | grep -i runtime
```

You should see entries like `halide_runtime_*.o`. If you do not, change
the `aot` rule's emit list to `static_library,h,runtime`.

## Enable Halide on the device build

Once the artifacts above exist, edit
`android/unified_detector_jni/src/jni/Android.mk` to:

1. Add this prebuilt static lib stanza near the top of the file
   (above the main `unified_detector` `include $(CLEAR_VARS)` block):

   ```make
   include $(CLEAR_VARS)
   LOCAL_MODULE := UD_HALIDE_AOT
   LOCAL_SRC_FILES := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/libs/$(TARGET_ARCH_ABI)/libud_preprocess_aot.a
   include $(PREBUILT_STATIC_LIBRARY)
   ```

2. Add the include path and link the static lib in the main module:

   ```make
   HALIDE_INC_PATH := $(ENGINE_THIRD_PARTY_LIBS_PATH)/halide/include
   LOCAL_C_INCLUDES += $(HALIDE_INC_PATH)
   LOCAL_CFLAGS     += -DUSE_HALIDE_PREPROCESS
   LOCAL_STATIC_LIBRARIES += UD_HALIDE_AOT
   ```

If `$(ENGINE_THIRD_PARTY_LIBS_PATH)` is not yet defined in this Android.mk,
add: `ENGINE_THIRD_PARTY_LIBS_PATH := $(ENGINE_BASE_PATH)/third_party`.

Do **not** add `-DUSE_HALIDE_PREPROCESS` until the AOT artifacts exist —
otherwise the include `"ud_preprocess_aot.h"` will fail.

## Schedule tuning checklist

The first build will *probably not* beat the existing hand-NEON path on
Snapdragon. Tune in this order:

1. **Tile size**. Try 16x16, 32x16, 32x32, 64x16. Profile with
   `Halide::Tools::benchmark` (host x86) and `systrace` on device.
2. **Vector width**. The default `vectorize(xi, 8)` is for 256-bit-wide
   targets. ARM NEON is 128-bit, so 4 may be the right answer. Try
   `vectorize(xi, 4)`.
3. **Parallelism**. `parallel(yo)` over 24 tiles (16-pixel rows) is
   typically too many threads. Try splitting `yo` further and parallelizing
   only on the outer split.
4. **Compute granularity**. Currently `pre.compute_at(out, xo)`. Try
   `compute_root` or `compute_at(out, yo)` for cache locality experiments.
5. **NEON-specific**. Add `target=arm-64-android-armv8_2a` for newer SoCs
   (S8 Gen 2+); enables more vectorizable opcodes.

## How to validate numerically

Build the project with `-DHALIDE_NUMERIC_CHECK` defined to enable a runtime
diff between the Halide path and the OpenCV reference inside the wrapper.
Acceptable max absolute error: ~3.0 over the 384x384x3 float buffer.

If you see >10, the YUV coefficients have drifted from
`viz_arm_neon::ics_yuv2rgb_bt709_neon`. The current generator uses the
exact same constants (1.403, 0.344, 0.714, 1.773, no Y-16 offset) so a
mismatch usually indicates a code edit went sideways.

If you see >50, the rotation map is inverted; flip `x`/`y` and `383-x`/
`383-y` in the `select(rotation==...)` chain in `generate()`.
