# Building `libud_halide_preprocess.so` — the Halide preprocessing as a standalone shared library

This guide creates a **separate** shared object that contains the complete Halide preprocessing
(the three AOT kernels + the Halide runtime), so the engine links it **dynamically** instead of
statically fusing the `.a` into `libUnifiedDetector.camera.samsung.so`.

```
            BEFORE (fused)                          AFTER (dynamic)
   libud_*_aot.a ─┐                        libud_*_aot.a ─┐
                  │ static (whole)                        │ static (whole)
                  ▼                                       ▼
   libUnifiedDetector.camera.samsung.so     libud_halide_preprocess.so   ← NEW standalone .so
   (Halide fused inside)                                  ▲
                                                          │ DT_NEEDED (dynamic)
                                            libUnifiedDetector.camera.samsung.so
                                            (calls ud_*_aot at runtime)
```

What stays the same: the wrapper `unified_detector/src/utils/halide_preprocess.cpp` is **unchanged**
and still lives in the engine. It just resolves `ud_preprocess_aot` / `ud_rgba2bgr_aot` /
`ud_rotate_normalize_aot` from the new `.so` at load time. No engine C++ logic is edited.

---

## 0. Prerequisites (the "once I have the `.a` files" state)

You must already have completed **Stage 1** (Halide AOT generation — see
`unified_detector/halide_gen/README.md`) so these exist:

```
unified_detector/third_party/halide/libs/arm64-v8a/
├── libud_preprocess_aot.a          # NV21/NV12 fused kernel  (CARRIES the Halide runtime)
├── libud_rgba2bgr_aot.a            # RGBA -> BGR             (no_runtime)
└── libud_rotate_normalize_aot.a    # rotate + normalize      (no_runtime)
unified_detector/third_party/halide/include/
├── ud_preprocess_aot.h
├── ud_rgba2bgr_aot.h
├── ud_rotate_normalize_aot.h
├── HalideBuffer.h
└── HalideRuntime.h
```

And the Android NDK is installed (same one used for the engine):

```bash
export PATH_TO_NDK=/path/to/android-ndk-r26d        # or your version
```

You have **two ways** to build the `.so`. Method A is the integrated, recommended path (the engine
actually consumes it). Method B is the quickest "just turn the `.a` into a `.so`" one-liner.

---

## Method A — Build it with `ndk-build` (recommended; engine links it dynamically)

This uses the two files already added to `src/jni/`:

- `halide_so_stub.cpp` — a one-line translation unit (a shared module needs ≥1 source file).
- `Android.halide_shared.mk` — a drop-in replacement for `Android.mk` that:
  1. adds a `BUILD_SHARED_LIBRARY` module `ud_halide_preprocess` using
     `LOCAL_WHOLE_STATIC_LIBRARIES := UD_HALIDE_AOT UD_RGBA2BGR_AOT UD_ROTATE_NORMALIZE_AOT`, and
  2. changes the engine module from the 3 `LOCAL_STATIC_LIBRARIES` lines to a single
     `LOCAL_SHARED_LIBRARIES += ud_halide_preprocess`.

### A.1 Swap in the makefile

`ndk-build` reads `src/jni/Android.mk` by name, so activate the new one:

```bash
cd android/unified_detector_jni/src/jni
cp Android.mk Android.mk.fused.bak          # back up the original (static-fused) makefile
cp Android.halide_shared.mk Android.mk      # use the dynamic-Halide makefile
```

### A.2 Build

```bash
cd android/unified_detector_jni
./build.sh          # runs $PATH_TO_NDK/ndk-build -j16, then copies src/libs -> libs
```

### A.3 Result

```
android/unified_detector_jni/libs/arm64-v8a/
├── libud_halide_preprocess.so              ← NEW: the standalone Halide preprocessing .so
├── libUnifiedDetector.camera.samsung.so    ← engine; now NEEDS libud_halide_preprocess.so
├── libSceneSegmenterJNI.so
├── libOpenCv.camera.samsung.so
└── libc++.so
```

`-DUSE_HALIDE_PREPROCESS` and the `halide/include` path stay on the engine module, so the wrapper
still compiles; the `ud_*_aot` symbols are now satisfied by the shared `.so` at link and run time.

---

## Method B — Build it directly from the `.a` (one command, no ndk-build)

If you only want the `.so` itself (e.g. to inspect it, or to wire it up by hand), link the three
archives into a shared object with `--whole-archive` so all kernel + runtime symbols are kept:

```bash
export PATH_TO_NDK=/path/to/android-ndk-r26d
HOST=linux-x86_64                                  # or darwin-x86_64
CLANG=$PATH_TO_NDK/toolchains/llvm/prebuilt/$HOST/bin/aarch64-linux-android24-clang++

AOT=unified_detector/third_party/halide/libs/arm64-v8a

$CLANG -shared -fPIC -fuse-ld=lld \
  -o libud_halide_preprocess.so \
  -Wl,--whole-archive \
      $AOT/libud_preprocess_aot.a \
      $AOT/libud_rgba2bgr_aot.a \
      $AOT/libud_rotate_normalize_aot.a \
  -Wl,--no-whole-archive \
  -Wl,-z,max-page-size=16384 \
  -llog -lm
```

Notes:
- `--whole-archive` is **mandatory**: without it the linker drops every kernel (nothing in the link
  references them yet) and you get an empty `.so`.
- API level `24` matches the project `minSdk`. Use a matching `aarch64-linux-android<API>-clang++`.
- Add `-ldl`/`-pthread` only if the linker reports undefined `dlopen`/`pthread_*` symbols.
- This produces the same `.so` as Method A's `ud_halide_preprocess` module, but does **not** rewire
  the engine — for that, use Method A (or apply the change-2/2 edit shown in `Android.halide_shared.mk`).

---

## 1. Verify the `.so`

```bash
# The three kernels + the version probe must be exported (T = defined text symbol):
nm -D --defined-only libud_halide_preprocess.so | grep -E 'ud_preprocess_aot|ud_rgba2bgr_aot|ud_rotate_normalize_aot|ud_halide_preprocess_version'

# The Halide runtime must be present exactly once (it lives in libud_preprocess_aot.a):
nm -D --defined-only libud_halide_preprocess.so | grep -i 'halide_' | head

# The engine must now declare a runtime dependency on it:
$PATH_TO_NDK/toolchains/llvm/prebuilt/$HOST/bin/llvm-readelf -d \
  libUnifiedDetector.camera.samsung.so | grep -i NEEDED | grep ud_halide_preprocess
```

Expected: the first command lists all four symbols; the second is non-empty; the third shows
`NEEDED  libud_halide_preprocess.so` (Method A only).

---

## 2. Ship the `.so` (bundle it next to the engine)

The new `.so` is a runtime dependency of the engine, so it must travel with it. Drop **all** of
these into the AAR's `jniLibs` (or the device partition):

```
UnifiedDetectorAAR/unifieddetector/src/main/jniLibs/arm64-v8a/
├── libud_halide_preprocess.so          ← NEW — must be present or the engine fails to load
├── libUnifiedDetector.camera.samsung.so
├── libSceneSegmenterJNI.so
├── libOpenCv.camera.samsung.so
└── libc++.so
```

No change is needed in the AAR Java: `VZImageSegmenter` loads the engine, and the dynamic linker
pulls in `libud_halide_preprocess.so` automatically via the `NEEDED` entry. (If you load by absolute
path on a device partition, make sure `libud_halide_preprocess.so` sits in the same lib directory.)

---

## 3. Revert to the fused build (if ever needed)

```bash
cd android/unified_detector_jni/src/jni
cp Android.mk.fused.bak Android.mk
```

---

## 4. Troubleshooting

| Symptom | Cause / Fix |
|---|---|
| `dlopen failed: cannot locate symbol "ud_preprocess_aot"` when loading the engine | `libud_halide_preprocess.so` is missing at runtime, or was built without `--whole-archive`. Confirm it's bundled next to the engine and that `nm -D` lists the `ud_*_aot` symbols. |
| `.so` builds but is tiny / `nm` shows no `ud_*` symbols | `--whole-archive` (Method B) or `LOCAL_WHOLE_STATIC_LIBRARIES` (Method A) was not used — the kernels got stripped as unreferenced. |
| `undefined reference to halide_*` while linking the engine | The Halide runtime didn't make it into the `.so`. It is carried by `libud_preprocess_aot.a`; make sure that archive is included in the whole-archive set (it is in both methods). |
| `multiple definition of halide_*` | You linked the runtime twice — e.g. left the old `LOCAL_STATIC_LIBRARIES += UD_*_AOT` on the engine **and** added the shared lib. Use only `LOCAL_SHARED_LIBRARIES += ud_halide_preprocess` on the engine (the makefile here already does this). |
| `cannot locate symbol` only on the device, not in build | `.so` not packaged. Check `abiFilters 'arm64-v8a'` and that the file is under `jniLibs/arm64-v8a/`. |
| Page-size / alignment crash on 16 KB-page devices | Keep `-Wl,-z,max-page-size=16384` on the link line (present in both methods). |

---

## 5. Why this is safe (no behavior change)

- The Halide kernels are byte-for-byte the same machine code, just hosted in a `.so` instead of
  fused `.a`. The wrapper, the NV21/NV12 + RGBA branch logic, and the OpenCV fallback are untouched.
- There is **no circular dependency**: `libud_halide_preprocess.so` depends only on libc/libm/log
  (and the runtime it carries). The engine depends on it one-way.
- `model.info`, OpenCV, the JNI bridge, and the AAR Java API are all unaffected.
