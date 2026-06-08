// =============================================================================
// halide_so_stub.cpp
//
// Translation unit for the standalone `libud_halide_preprocess.so` module.
//
// The actual Halide preprocessing code lives in the three AOT static archives
// (libud_preprocess_aot.a, libud_rgba2bgr_aot.a, libud_rotate_normalize_aot.a),
// which are pulled into this shared library WHOLE (see LOCAL_WHOLE_STATIC_LIBRARIES
// in Android.halide_shared.mk / Android.halide_only.mk). Whole-archiving keeps and
// exports every symbol —
//   ud_preprocess_aot, ud_rgba2bgr_aot, ud_rotate_normalize_aot, and the Halide
//   runtime (halide_*) carried by libud_preprocess_aot.a —
// so the engine can resolve them dynamically at load time instead of statically
// fusing the .a into libUnifiedDetector.camera.samsung.so.
//

// =============================================================================

extern "C" {

// Exported so you can confirm the .so loaded and is the expected build:
//   adb shell "nm -D libud_halide_preprocess.so | grep ud_"
const char* ud_halide_preprocess_version() {
    return "ud_halide_preprocess 1.0 (ud_preprocess_aot + ud_rgba2bgr_aot + ud_rotate_normalize_aot)";
}

}  // extern "C"
