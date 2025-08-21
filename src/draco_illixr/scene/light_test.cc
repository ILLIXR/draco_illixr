// Copyright 2022 The Draco Authors.
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
#include "draco_illixr/scene/light.h"

#include <limits>

#include "draco_illixr/core/constants.h"
#include "draco_illixr/core/draco_test_base.h"
#include "draco_illixr/core/draco_test_utils.h"

namespace {

#ifdef DRACO_TRANSCODER_SUPPORTED

TEST(LightTest, TestDefaults) {
  // Text constructing draco_illixr::Light object with default properties.
  const draco_illixr::Light light;
  ASSERT_EQ(light.GetName(), "");
  ASSERT_EQ(light.GetColor(), draco_illixr::Vector3f(1.0f, 1.0f, 1.0f));
  ASSERT_EQ(light.GetIntensity(), 1.0);
  ASSERT_EQ(light.GetType(), draco_illixr::Light::POINT);
  ASSERT_EQ(light.GetRange(), std::numeric_limits<float>::max());
  ASSERT_EQ(light.GetInnerConeAngle(), 0.0);
  ASSERT_EQ(light.GetOuterConeAngle(), DRACO_PI / 4.0);
}

TEST(LightTest, TestCopy) {
  // Test copying of draco_illixr::Light object.
  draco_illixr::Light light;
  light.SetName("The Star of Earendil");
  light.SetColor(draco_illixr::Vector3f(0.90, 0.97, 1.00));
  light.SetIntensity(5.0);
  light.SetType(draco_illixr::Light::SPOT);
  light.SetRange(1000.0);
  light.SetInnerConeAngle(DRACO_PI / 8.0);
  light.SetOuterConeAngle(DRACO_PI / 2.0);

  // Create a copy of the initialized light and check all properties.
  draco_illixr::Light copy;
  copy.Copy(light);
  ASSERT_EQ(copy.GetName(), "The Star of Earendil");
  ASSERT_EQ(copy.GetColor(), draco_illixr::Vector3f(0.90, 0.97, 1.00));
  ASSERT_EQ(copy.GetIntensity(), 5.0);
  ASSERT_EQ(copy.GetType(), draco_illixr::Light::SPOT);
  ASSERT_EQ(copy.GetRange(), 1000.0);
  ASSERT_EQ(copy.GetInnerConeAngle(), DRACO_PI / 8.0);
  ASSERT_EQ(copy.GetOuterConeAngle(), DRACO_PI / 2.0);
}

#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace
