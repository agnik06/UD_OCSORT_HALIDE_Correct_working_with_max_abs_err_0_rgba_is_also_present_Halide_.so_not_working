// ud_preprocess_generator.cpp
// =============================================================================
// Halide Generator: fused NV21/NV12 -> BGR resize -> rotate -> mean-subtract.
//
// This file is HOST-SIDE only. It is compiled into a binary that you run on
// your Linux/WSL desktop; the binary then emits a static library + header
// that the Android NDK build links against.
//
// Output spec (must match SNAP detector input):
//   - 384 x 384, BGR, float32, HWC interleaved
//   - value = float(bgr_pixel) - 128.0f
//
// YUV->BGR coefficients are intentionally identical to
//   viz_arm_neon::ics_yuv2rgb_bt709_neon (despite the name, it uses BT.601
//   full-range constants: 1.403 / 0.344 / 0.714 / 1.773, no Y-16 offset).
// =============================================================================

#include "Halide.h"
using namespace Halide;

class UDPreprocess : public Halide::Generator<UDPreprocess> {
public:
    // ---- Inputs ---------------------------------------------------------
    // Y plane: width * height bytes, row-major.
    Input<Buffer<uint8_t>> y_plane{"y_plane", 2};

    // UV plane: half-resolution in both dims, two interleaved chroma channels.
    // For NV21 the in-memory order is V,U; for NV12 it is U,V.
    // We model it as a 3D buffer [x = W/2, y = H/2, c = 2] with strides
    // chosen by the host wrapper to match the actual bi-planar layout.
    Input<Buffer<uint8_t>> uv_plane{"uv_plane", 3};

    Input<int> is_nv21{"is_nv21"};   // 1 = NV21, 0 = NV12
    Input<int> rotation{"rotation"}; // 0, 90, 180, 270 (clockwise)
    Input<int> in_w{"in_w"};
    Input<int> in_h{"in_h"};

    // ---- Output ---------------------------------------------------------
    // 384 x 384 x 3, float32, channel innermost (HWC interleaved).
    Output<Buffer<float>> out{"out", 3};

    void generate() {
    Var x("x"), y("y"), c("c");

    // (A) Undo rotation. The NEON path rotates the finished 384x384 BGR
    //     image AFTER resize+convert (rotate_image -> cv::rotate). So we
    //     read pre-rotation grid coords (px,py). rotate_image mapping:
    //     90 -> ROTATE_90_CCW, 180 -> ROTATE_180, 270 -> ROTATE_90_CW.
    Expr px = select(rotation == 90,  383 - y,
              select(rotation == 180, 383 - x,
              select(rotation == 270, y,  x)));
    Expr py = select(rotation == 90,  x,
              select(rotation == 180, 383 - y,
              select(rotation == 270, 383 - x,  y)));

    // (B) NEON resize mapping (resize_nv21_or_nv12_buffer):
    //     step = src_dim / 384  (float32 division)
    //     src  = (int)(dst * step)   -- truncate toward zero
    Expr step_w = cast<float>(in_w) / 384.0f;
    Expr step_h = cast<float>(in_h) / 384.0f;

    // (C) Y plane: SINGLE nearest sample (NOT bilinear).
    Func y_in = BoundaryConditions::repeat_edge(y_plane);
    Expr jY = cast<int>(cast<float>(px) * step_w);
    Expr iY = cast<int>(cast<float>(py) * step_h);
    Expr Y  = cast<float>(y_in(jY, iY));

    // (D) UV plane: 4-tap average over the output 2x2 block, exactly as
    //     resize_nv21_or_nv12_buffer accumulates 4 samples then /4.
    Func uv_in = BoundaryConditions::repeat_edge(uv_plane);
    Expr xe = px - (px & 1);                 // x_even
    Expr ye = py - (py & 1);                 // y_even
    Expr jA = cast<int>(cast<float>(xe)     * step_w);
    Expr jB = cast<int>(cast<float>(xe + 1) * step_w);
    Expr iA = cast<int>(cast<float>(ye)     * step_h);
    Expr iB = cast<int>(cast<float>(ye + 1) * step_h);
    // channel 0 = even byte, channel 1 = odd byte (V/U order set by is_nv21)
    Expr c0 = (cast<int>(uv_in(jA>>1, iA>>1, 0)) + cast<int>(uv_in(jB>>1, iA>>1, 0))
             + cast<int>(uv_in(jA>>1, iB>>1, 0)) + cast<int>(uv_in(jB>>1, iB>>1, 0))) >> 2;
    Expr c1 = (cast<int>(uv_in(jA>>1, iA>>1, 1)) + cast<int>(uv_in(jB>>1, iA>>1, 1))
             + cast<int>(uv_in(jA>>1, iB>>1, 1)) + cast<int>(uv_in(jB>>1, iB>>1, 1))) >> 2;

    Expr V_ = cast<float>(select(is_nv21 == 1, c0, c1));   // NV21: V = even byte
    Expr U_ = cast<float>(select(is_nv21 == 1, c1, c0));   // NV21: U = odd byte
    Expr Cr = V_ - 128.0f;
    Expr Cb = U_ - 128.0f;

    // (E) BT.601 YUV->BGR. Operation tree mirrors
    //     ics_yuv2rgb_bt709_neon_8I node-for-node.
    Expr Rf = strict_float(Cr * 1.403f + Y);
    Expr Gf = strict_float((Cb * (-0.344f) + Y) + Cr * (-0.714f));
    Expr Bf = strict_float(Cb * 1.773f + Y);

    // (F) NEON truncates the float result to uint8 (FCVTZU + saturating
    //     narrow) BEFORE the later float conversion. Replicate exactly.
    Expr R8 = clamp(cast<int>(Rf), 0, 255);
    Expr G8 = clamp(cast<int>(Gf), 0, 255);
    Expr B8 = clamp(cast<int>(Bf), 0, 255);
    Expr bgr8 = select(c == 0, B8, select(c == 1, G8, R8));

    // (G) normalize_image: (float)bgr_u8 - 128.0
    Func pre("pre");
    pre(c, x, y) = cast<float>(bgr8) - 128.0f;
    out(c, x, y) = pre(c, x, y);

    // ---- strides : UNCHANGED (keep your UV-stride fix + HWC output) ----
    uv_plane.dim(0).set_stride(2);
    uv_plane.dim(1).set_stride(in_w);
    uv_plane.dim(2).set_stride(1);
    out.dim(0).set_stride(1).set_extent(3);
    out.dim(1).set_stride(3);
    out.dim(2).set_stride(3 * 384);
    out.bound(c, 0, 3).bound(x, 0, 384).bound(y, 0, 384);

    // ---- schedule : UNCHANGED (preserves the 0.5 ms profile) ----
    Var xo("xo"), yo("yo"), xi("xi"), yi("yi");
    out.reorder(c, x, y)
       .unroll(c)
       .tile(x, y, xo, yo, xi, yi, 32, 16)
       .vectorize(xi, 8)
       .parallel(yo);
    pre.compute_at(out, xo).vectorize(x, 8).unroll(c);
              }

};

HALIDE_REGISTER_GENERATOR(UDPreprocess, ud_preprocess)
