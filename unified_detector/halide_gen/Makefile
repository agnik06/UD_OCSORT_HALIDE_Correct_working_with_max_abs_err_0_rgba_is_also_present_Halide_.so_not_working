# Makefile for AOT-compiling the UD preprocess Halide pipeline.
# Run on a Linux/WSL desktop with Halide installed (NOT on the device).
#
# Usage for Halide 21.0.0:
#   export HALIDE_ROOT=/home/test/agnik/Halide-21.0.0
#   make all
#
# Three pipelines are built; all three are selected automatically by input
# format inside halide_preprocess.cpp:
#   NV21 / NV12               ->  ud_preprocess_aot       (libud_preprocess_aot.a, carries the runtime)
#   RGBA (cvtColor step)      ->  ud_rgba2bgr_aot         (no_runtime)
#   RGBA (rotate + normalize) ->  ud_rotate_normalize_aot (no_runtime)
#
# The RGBA path uses cv::resize between the two RGBA generators so the bilinear
# resize stays bit-exact with the original cv::resize (a Halide float bilinear
# cannot match OpenCV's fixed-point bilinear).
#
# Outputs:
#   ../third_party/halide/libs/arm64-v8a/libud_preprocess_aot.a       (+ runtime)
#   ../third_party/halide/libs/arm64-v8a/libud_rgba2bgr_aot.a         (no_runtime)
#   ../third_party/halide/libs/arm64-v8a/libud_rotate_normalize_aot.a (no_runtime)
#   ../third_party/halide/include/ud_preprocess_aot.h
#   ../third_party/halide/include/ud_rgba2bgr_aot.h
#   ../third_party/halide/include/ud_rotate_normalize_aot.h
#   ../third_party/halide/include/HalideBuffer.h
#   ../third_party/halide/include/HalideRuntime.h

HALIDE_ROOT ?= /home/test/agnik/Halide-21.0.0
HALIDE_INC  := $(HALIDE_ROOT)/include
HALIDE_LIB  := $(HALIDE_ROOT)/lib
HALIDE_BIN  := $(HALIDE_ROOT)/bin

# Where AOT artifacts get dropped — relative to this Makefile.
OUT_DIR := ../third_party/halide/libs/arm64-v8a
INC_DIR := ../third_party/halide/include

# Halide target string. arm-64-android = ARMv8.0a baseline, no extra features.
TARGET := arm-64-android

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -fPIC -I$(HALIDE_INC)
LDFLAGS  := -L$(HALIDE_LIB) -lHalide -lz -ldl -lpthread -Wl,-rpath,$(HALIDE_LIB)

# ---- Step 1: build the generator as a shared library ----------------------
# Halide 21.0.0 uses gengen to load generator plugins (.so files)
ud_preprocess_gen.so: ud_preprocess_generator.cpp
	$(CXX) $(CXXFLAGS) -shared $< -o $@ $(LDFLAGS)

# ---- Step 2: run gengen to produce the static library + header ------------
aot: ud_preprocess_gen.so
	mkdir -p $(OUT_DIR) $(INC_DIR)
	LD_LIBRARY_PATH=$(HALIDE_LIB):$$LD_LIBRARY_PATH \
	  $(HALIDE_BIN)/gengen -p ./ud_preprocess_gen.so \
	  -g ud_preprocess -o $(OUT_DIR) \
	  -e static_library,h target=$(TARGET) -f ud_preprocess_aot
	mv $(OUT_DIR)/ud_preprocess_aot.h $(INC_DIR)/

# ---- RGBA pipeline: 2 generators in ud_rgba_generator.cpp, both no_runtime --
# They share the Halide runtime bundled in libud_preprocess_aot.a above.
ud_rgba_gen.so: ud_rgba_generator.cpp
	$(CXX) $(CXXFLAGS) -shared $< -o $@ $(LDFLAGS)

aot_rgba: ud_rgba_gen.so
	mkdir -p $(OUT_DIR) $(INC_DIR)
	LD_LIBRARY_PATH=$(HALIDE_LIB):$$LD_LIBRARY_PATH \
	  $(HALIDE_BIN)/gengen -p ./ud_rgba_gen.so \
	  -g ud_rgba2bgr -o $(OUT_DIR) \
	  -e static_library,h target=$(TARGET)-no_runtime -f ud_rgba2bgr_aot
	LD_LIBRARY_PATH=$(HALIDE_LIB):$$LD_LIBRARY_PATH \
	  $(HALIDE_BIN)/gengen -p ./ud_rgba_gen.so \
	  -g ud_rotate_normalize -o $(OUT_DIR) \
	  -e static_library,h target=$(TARGET)-no_runtime -f ud_rotate_normalize_aot
	mv $(OUT_DIR)/ud_rgba2bgr_aot.h         $(INC_DIR)/
	mv $(OUT_DIR)/ud_rotate_normalize_aot.h $(INC_DIR)/

# ---- Step 3: copy the C++ runtime headers the wrapper needs ---------------
buffer_header:
	mkdir -p $(INC_DIR)
	cp $(HALIDE_INC)/HalideBuffer.h  $(INC_DIR)/
	cp $(HALIDE_INC)/HalideRuntime.h $(INC_DIR)/

all: aot aot_rgba buffer_header
	@echo ""
	@echo "================================================================"
	@echo "  Halide AOT artifacts ready:"
	@echo "    $(OUT_DIR)/libud_preprocess_aot.a        (NV21/NV12 + runtime)"
	@echo "    $(OUT_DIR)/libud_rgba2bgr_aot.a          (RGBA cvtColor, no_runtime)"
	@echo "    $(OUT_DIR)/libud_rotate_normalize_aot.a  (RGBA rotate+norm, no_runtime)"
	@echo "    $(INC_DIR)/ud_preprocess_aot.h"
	@echo "    $(INC_DIR)/ud_rgba2bgr_aot.h"
	@echo "    $(INC_DIR)/ud_rotate_normalize_aot.h"
	@echo "    $(INC_DIR)/HalideBuffer.h"
	@echo "    $(INC_DIR)/HalideRuntime.h"
	@echo ""
	@echo "  Verify the runtime lives ONLY in libud_preprocess_aot.a:"
	@echo "    ar t $(OUT_DIR)/libud_preprocess_aot.a       | grep -i runtime  # non-empty"
	@echo "    ar t $(OUT_DIR)/libud_rgba2bgr_aot.a         | grep -i runtime  # empty"
	@echo "    ar t $(OUT_DIR)/libud_rotate_normalize_aot.a | grep -i runtime  # empty"
	@echo ""
	@echo "  Three pipelines auto-select by input format in halide_preprocess.cpp:"
	@echo "    NV21/NV12 -> ud_preprocess_aot (fused)"
	@echo "    RGBA      -> ud_rgba2bgr_aot + cv::resize + ud_rotate_normalize_aot"
	@echo "================================================================"

clean:
	rm -f ud_preprocess_generator ud_preprocess_gen.so ud_rgba_gen.so
	rm -rf $(OUT_DIR) $(INC_DIR)

.PHONY: all aot aot_rgba buffer_header clean