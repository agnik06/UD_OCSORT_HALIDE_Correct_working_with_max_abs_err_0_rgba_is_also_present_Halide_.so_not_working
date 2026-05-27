// ud_rgba_generator.cpp
// =============================================================================
// Halide generators for the RGBA-input detector preprocessing path.
//
// The original RGBA preprocessing in image_utils.cpp does:
//     cv::cvtColor(rgba, bgr, CV_RGBA2BGR)        // pure channel reorder
//     cv::resize(bgr, bgr, Size(384,384))         // INTER_LINEAR (fixed-point)
//     rotate_image(bgr, image_rotation)            // cv::rotate (90/180/270)
//     normalize_image(bgr, {128,128,128}, {1,1,1}) // (float - 128) / 1
//
// To stay BIT-EXACT with this reference, we port only the operations that can
// be bit-exact:
//
//   ud_rgba2bgr        : cv::cvtColor(CV_RGBA2BGR)         -- pure channel reorder
//   ud_rotate_normalize: cv::rotate + (float - 128)/1      -- pure index remap + exact arithmetic
//
// cv::resize is NOT ported. OpenCV's INTER_LINEAR for uint8 uses fixed-point
// bilinear weights with specific rounding; a Halide float bilinear can not be
// matched to it bit-exactly without painstakingly emulating OpenCV's internal
// HResize/VResize integer pipeline (the previous folder's float-bilinear RGBA
// generator explicitly admitted "detection-equivalent, not bit-exact", which
// is why it was dropped). The host wrapper calls `cv::resize` between the two
// generators below; that is the only OpenCV op left on the RGBA path and it
// guarantees the resize is bit-identical to the original.
//
// Both generators are emitted with target=...-no_runtime; they share the
// Halide runtime already bundled in libud_preprocess_aot.a (the NV21/NV12
// fused pipeline). Keep that linked.
// =============================================================================

#include "Halide.h"
using namespace Halide;

// -----------------------------------------------------------------------------
// ud_rgba2bgr : RGBA (interleaved 4-channel) -> BGR (interleaved 3-channel).
// Replaces cv::cvtColor(CV_RGBA2BGR). Bit-exact (pure channel reorder, no math).
// -----------------------------------------------------------------------------
class UDRgba2Bgr : public Halide::Generator<UDRgba2Bgr> {
public:
    // RGBA plane: 4 interleaved bytes per pixel in order R,G,B,A.
    // Modelled as [c=4, x=in_w, y=in_h] with channel innermost.
    Input<Buffer<uint8_t>> rgba{"rgba", 3};
    Input<int>             in_w{"in_w"};
    Input<int>             in_h{"in_h"};
    // BGR plane: 3 interleaved bytes per pixel in order B,G,R.
    Output<Buffer<uint8_t>> bgr{"bgr", 3};

    void generate() {
        Var x("x"), y("y"), c("c");

        // CV_RGBA2BGR: out channel c reads input channel:
        //   c==0 (B) -> 2,  c==1 (G) -> 1,  c==2 (R) -> 0
        // The select folds to a constant once c is unrolled.
        Expr src_c = select(c == 0, 2, select(c == 1, 1, 0));
        bgr(c, x, y) = rgba(src_c, x, y);

        // ---- strides : interleaved channel-innermost both ways --------------
        rgba.dim(0).set_stride(1).set_extent(4);
        rgba.dim(1).set_stride(4);
        rgba.dim(2).set_stride(4 * in_w);

        bgr.dim(0).set_stride(1).set_extent(3);
        bgr.dim(1).set_stride(3);
        // bgr.dim(2).stride left flexible: the host wrapper passes the actual
        // row stride of the cv::Mat (which OpenCV may pad for alignment).

        bgr.dim(0).set_bounds(0, 3);
        bgr.dim(1).set_bounds(0, in_w);
        bgr.dim(2).set_bounds(0, in_h);

        // ---- schedule -------------------------------------------------------
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");
        bgr.reorder(c, x, y)
           .unroll(c)
           .tile(x, y, xo, yo, xi, yi, 32, 16)
           .vectorize(xi, 8)
           .parallel(yo);
    }
};

// -----------------------------------------------------------------------------
// ud_rotate_normalize : BGR 384x384 uint8 -> float32 384x384, with rotation
// (90/180/270 clockwise per rotate_image semantics) and mean-subtract fused.
//
// Pre-rotation source coords (px, py) are derived from output (x, y) so the
// fused pipeline reads exactly the pixel that the post-resize, post-rotate
// reference would have produced. Mapping matches rotate_image:
//     90  -> ROTATE_90_COUNTERCLOCKWISE   src(W-1-y, x)        => px=383-y, py=x
//     180 -> ROTATE_180                   src(W-1-x, H-1-y)    => px=383-x, py=383-y
//     270 -> ROTATE_90_CLOCKWISE          src(y,     H-1-x)    => px=y,     py=383-x
//     other (0/invalid) -> identity                            => px=x,     py=y
//
// Rotation is a pure index gather (no interpolation); subtract-128 on uint8
// values is exact in float32. Bit-exact with the reference end-to-end.
// -----------------------------------------------------------------------------
class UDRotateNormalize : public Halide::Generator<UDRotateNormalize> {
public:
    Input<Buffer<uint8_t>> bgr{"bgr", 3};      // [c=3, x=384, y=384]
    Input<int>             rotation{"rotation"};
    Output<Buffer<float>>  out{"out", 3};      // [c=3, x=384, y=384]

    void generate() {
        Var x("x"), y("y"), c("c");

        Expr px = select(rotation == 90,  383 - y,
                  select(rotation == 180, 383 - x,
                  select(rotation == 270, y,  x)));
        Expr py = select(rotation == 90,  x,
                  select(rotation == 180, 383 - y,
                  select(rotation == 270, 383 - x,  y)));

        out(c, x, y) = cast<float>(bgr(c, px, py)) - 128.0f;

        // ---- strides --------------------------------------------------------
        // Input bgr 384x384 (post cv::resize). The host wrapper allocates a
        // CV_8UC3 cv::Mat for it; cv::Mat at 384x384 is contiguous.
        bgr.dim(0).set_stride(1).set_extent(3);
        bgr.dim(1).set_stride(3);
        bgr.dim(2).set_stride(3 * 384);

        // Output: HWC interleaved float32 384x384x3, identical layout to
        // ud_preprocess output.
        out.dim(0).set_stride(1).set_extent(3);
        out.dim(1).set_stride(3);
        out.dim(2).set_stride(3 * 384);

        bgr.dim(0).set_bounds(0, 3);
        bgr.dim(1).set_bounds(0, 384);
        bgr.dim(2).set_bounds(0, 384);

        out.dim(0).set_bounds(0, 3);
        out.dim(1).set_bounds(0, 384);
        out.dim(2).set_bounds(0, 384);

        // ---- schedule (mirrors ud_preprocess) -------------------------------
        Var xo("xo"), yo("yo"), xi("xi"), yi("yi");
        out.reorder(c, x, y)
           .unroll(c)
           .tile(x, y, xo, yo, xi, yi, 32, 16)
           .vectorize(xi, 8)
           .parallel(yo);
    }
};

HALIDE_REGISTER_GENERATOR(UDRgba2Bgr,        ud_rgba2bgr)
HALIDE_REGISTER_GENERATOR(UDRotateNormalize, ud_rotate_normalize)