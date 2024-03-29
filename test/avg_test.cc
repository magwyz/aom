/*
 *  Copyright (c) 2019, Alliance for Open Media. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "third_party/googletest/src/googletest/include/gtest/gtest.h"

#include "config/aom_dsp_rtcd.h"

#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/util.h"

namespace {

using libaom_test::ACMRandom;

template <typename Pixel>
class AverageTestBase : public ::testing::Test {
 public:
  AverageTestBase(int width, int height)
      : width_(width), height_(height), source_data_(NULL), source_stride_(0),
        bit_depth_(8) {}

  virtual void TearDown() {
    aom_free(source_data_);
    source_data_ = NULL;
    libaom_test::ClearSystemState();
  }

 protected:
  // Handle blocks up to 4 blocks 64x64 with stride up to 128
  static const int kDataAlignment = 16;
  static const int kDataBlockSize = 64 * 128;

  virtual void SetUp() {
    source_data_ = reinterpret_cast<Pixel *>(
        aom_memalign(kDataAlignment, kDataBlockSize * sizeof(source_data_[0])));
    ASSERT_TRUE(source_data_ != NULL);
    source_stride_ = (width_ + 31) & ~31;
    bit_depth_ = 8;
    rnd_.Reset(ACMRandom::DeterministicSeed());
  }

  // Sum Pixels
  static unsigned int ReferenceAverage8x8(const Pixel *source, int pitch) {
    unsigned int average = 0;
    for (int h = 0; h < 8; ++h) {
      for (int w = 0; w < 8; ++w) average += source[h * pitch + w];
    }
    return (average + 32) >> 6;
  }

  static unsigned int ReferenceAverage4x4(const Pixel *source, int pitch) {
    unsigned int average = 0;
    for (int h = 0; h < 4; ++h) {
      for (int w = 0; w < 4; ++w) average += source[h * pitch + w];
    }
    return (average + 8) >> 4;
  }

  void FillConstant(Pixel fill_constant) {
    for (int i = 0; i < width_ * height_; ++i) {
      source_data_[i] = fill_constant;
    }
  }

  void FillRandom() {
    for (int i = 0; i < width_ * height_; ++i) {
      source_data_[i] = rnd_.Rand16() & ((1 << bit_depth_) - 1);
    }
  }

  int width_, height_;
  Pixel *source_data_;
  int source_stride_;
  int bit_depth_;

  ACMRandom rnd_;
};
typedef unsigned int (*AverageFunction)(const uint8_t *s, int pitch);

// Arguments: width, height, pitch, block size, avg function.
typedef std::tuple<int, int, int, int, AverageFunction> AvgFunc;

class AverageTest : public AverageTestBase<uint8_t>,
                    public ::testing::WithParamInterface<AvgFunc> {
 public:
  AverageTest() : AverageTestBase(GET_PARAM(0), GET_PARAM(1)) {}

 protected:
  void CheckAverages() {
    const int block_size = GET_PARAM(3);
    unsigned int expected = 0;
    if (block_size == 8) {
      expected =
          ReferenceAverage8x8(source_data_ + GET_PARAM(2), source_stride_);
    } else if (block_size == 4) {
      expected =
          ReferenceAverage4x4(source_data_ + GET_PARAM(2), source_stride_);
    }

    unsigned int actual;
    ASM_REGISTER_STATE_CHECK(
        actual = GET_PARAM(4)(source_data_ + GET_PARAM(2), source_stride_));

    EXPECT_EQ(expected, actual);
  }
};

TEST_P(AverageTest, MinValue) {
  FillConstant(0);
  CheckAverages();
}

TEST_P(AverageTest, MaxValue) {
  FillConstant(255);
  CheckAverages();
}

TEST_P(AverageTest, Random) {
  // The reference frame, but not the source frame, may be unaligned for
  // certain types of searches.
  for (int i = 0; i < 1000; i++) {
    FillRandom();
    CheckAverages();
  }
}

using std::make_tuple;

INSTANTIATE_TEST_CASE_P(
    C, AverageTest,
    ::testing::Values(make_tuple(16, 16, 1, 8, &aom_avg_8x8_c),
                      make_tuple(16, 16, 1, 4, &aom_avg_4x4_c)));

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, AverageTest,
    ::testing::Values(make_tuple(16, 16, 0, 8, &aom_avg_8x8_sse2),
                      make_tuple(16, 16, 5, 8, &aom_avg_8x8_sse2),
                      make_tuple(32, 32, 15, 8, &aom_avg_8x8_sse2),
                      make_tuple(16, 16, 0, 4, &aom_avg_4x4_sse2),
                      make_tuple(16, 16, 5, 4, &aom_avg_4x4_sse2),
                      make_tuple(32, 32, 15, 4, &aom_avg_4x4_sse2)));
#endif

#if HAVE_NEON
INSTANTIATE_TEST_CASE_P(
    NEON, AverageTest,
    ::testing::Values(make_tuple(16, 16, 0, 8, &aom_avg_8x8_neon),
                      make_tuple(16, 16, 5, 8, &aom_avg_8x8_neon),
                      make_tuple(32, 32, 15, 8, &aom_avg_8x8_neon),
                      make_tuple(16, 16, 0, 4, &aom_avg_4x4_neon),
                      make_tuple(16, 16, 5, 4, &aom_avg_4x4_neon),
                      make_tuple(32, 32, 15, 4, &aom_avg_4x4_neon)));
#endif

}  // namespace
