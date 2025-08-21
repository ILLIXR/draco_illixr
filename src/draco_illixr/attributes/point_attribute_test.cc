// Copyright 2017 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco_illixr/attributes/point_attribute.h"

#include "draco_illixr/core/draco_test_base.h"

namespace {

class PointAttributeTest : public ::testing::Test {
 protected:
  PointAttributeTest() {}
};

TEST_F(PointAttributeTest, TestCopy) {
  // This test verifies that PointAttribute can copy data from another point
  // attribute.
  draco_illixr::PointAttribute pa;
  pa.Init(draco_illixr::GeometryAttribute::POSITION, 1, draco_illixr::DT_INT32, false, 10);

  for (int32_t i = 0; i < 10; ++i) {
    pa.SetAttributeValue(draco_illixr::AttributeValueIndex(i), &i);
  }

  pa.set_unique_id(12);

  draco_illixr::PointAttribute other_pa;
  other_pa.CopyFrom(pa);

  draco_illixr::PointAttributeHasher hasher;
  ASSERT_EQ(hasher(pa), hasher(other_pa));
  ASSERT_EQ(pa.unique_id(), other_pa.unique_id());

  // The hash function does not actually compute the hash from attribute values,
  // so ensure the data got copied correctly as well.
  for (int32_t i = 0; i < 10; ++i) {
    int32_t data;
    other_pa.GetValue(draco_illixr::AttributeValueIndex(i), &data);
    ASSERT_EQ(data, i);
  }
}

TEST_F(PointAttributeTest, TestGetValueFloat) {
  draco_illixr::PointAttribute pa;
  pa.Init(draco_illixr::GeometryAttribute::POSITION, 3, draco_illixr::DT_FLOAT32, false, 5);
  float points[3];
  for (int32_t i = 0; i < 5; ++i) {
    points[0] = i * 3.0;
    points[1] = (i * 3.0) + 1.0;
    points[2] = (i * 3.0) + 2.0;
    pa.SetAttributeValue(draco_illixr::AttributeValueIndex(i), &points);
  }

  for (int32_t i = 0; i < 5; ++i) {
    pa.GetValue(draco_illixr::AttributeValueIndex(i), &points);
    ASSERT_FLOAT_EQ(points[0], i * 3.0);
    ASSERT_FLOAT_EQ(points[1], (i * 3.0) + 1.0);
    ASSERT_FLOAT_EQ(points[2], (i * 3.0) + 2.0);
  }
}

TEST_F(PointAttributeTest, TestGetArray) {
  draco_illixr::PointAttribute pa;
  pa.Init(draco_illixr::GeometryAttribute::POSITION, 3, draco_illixr::DT_FLOAT32, false, 5);
  float points[3];
  for (int32_t i = 0; i < 5; ++i) {
    points[0] = i * 3.0;
    points[1] = (i * 3.0) + 1.0;
    points[2] = (i * 3.0) + 2.0;
    pa.SetAttributeValue(draco_illixr::AttributeValueIndex(i), &points);
  }

  for (int32_t i = 0; i < 5; ++i) {
    std::array<float, 3> att_value;
    att_value = pa.GetValue<float, 3>(draco_illixr::AttributeValueIndex(i));
    ASSERT_FLOAT_EQ(att_value[0], i * 3.0);
    ASSERT_FLOAT_EQ(att_value[1], (i * 3.0) + 1.0);
    ASSERT_FLOAT_EQ(att_value[2], (i * 3.0) + 2.0);
  }
  for (int32_t i = 0; i < 5; ++i) {
    std::array<float, 3> att_value;
    EXPECT_TRUE(
        (pa.GetValue<float, 3>(draco_illixr::AttributeValueIndex(i), &att_value)));
    ASSERT_FLOAT_EQ(att_value[0], i * 3.0);
    ASSERT_FLOAT_EQ(att_value[1], (i * 3.0) + 1.0);
    ASSERT_FLOAT_EQ(att_value[2], (i * 3.0) + 2.0);
  }
}

TEST_F(PointAttributeTest, TestArrayReadError) {
  draco_illixr::PointAttribute pa;
  pa.Init(draco_illixr::GeometryAttribute::POSITION, 3, draco_illixr::DT_FLOAT32, false, 5);
  float points[3];
  for (int32_t i = 0; i < 5; ++i) {
    points[0] = i * 3.0;
    points[1] = (i * 3.0) + 1.0;
    points[2] = (i * 3.0) + 2.0;
    pa.SetAttributeValue(draco_illixr::AttributeValueIndex(i), &points);
  }

  std::array<float, 3> att_value;
  EXPECT_FALSE(
      (pa.GetValue<float, 3>(draco_illixr::AttributeValueIndex(5), &att_value)));
}

TEST_F(PointAttributeTest, TestResize) {
  draco_illixr::PointAttribute pa;
  pa.Init(draco_illixr::GeometryAttribute::POSITION, 3, draco_illixr::DT_FLOAT32, false, 5);
  ASSERT_EQ(pa.size(), 5);
  ASSERT_EQ(pa.buffer()->data_size(), 4 * 3 * 5);

  pa.Resize(10);
  ASSERT_EQ(pa.size(), 10);
  ASSERT_EQ(pa.buffer()->data_size(), 4 * 3 * 10);
}

}  // namespace
