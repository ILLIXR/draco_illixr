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
#ifndef DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_DECODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_DECODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco_illixr/animation/keyframe_animation_decoder.h"
#include "draco_illixr/attributes/attribute_transform_type.h"
#include "draco_illixr/attributes/point_attribute.h"
#include "draco_illixr/compression/config/compression_shared.h"
#include "draco_illixr/compression/decode.h"
#include "draco_illixr/core/decoder_buffer.h"

typedef draco_illixr::AttributeTransformType draco_AttributeTransformType;
typedef draco_illixr::GeometryAttribute draco_GeometryAttribute;
typedef draco_GeometryAttribute::Type draco_GeometryAttribute_Type;
typedef draco_illixr::EncodedGeometryType draco_EncodedGeometryType;
typedef draco_illixr::Status draco_Status;
typedef draco_illixr::Status::Code draco_StatusCode;

class DracoFloat32Array {
 public:
  DracoFloat32Array();
  float GetValue(int index) const;

  // In case |values| is nullptr, the data is allocated but not initialized.
  bool SetValues(const float *values, int count);

  // Directly sets a value for a specific index. The array has to be already
  // allocated at this point (using SetValues() method).
  void SetValue(int index, float val) { values_[index] = val; }

  int size() const { return values_.size(); }

 private:
  std::vector<float> values_;
};

// Class used by emscripten WebIDL Binder [1] to wrap calls to decode animation
// data.
class AnimationDecoder {
 public:
  AnimationDecoder();

  // Decodes animation data from the provided buffer.
  const draco_illixr::Status *DecodeBufferToKeyframeAnimation(
      draco_illixr::DecoderBuffer *in_buffer, draco_illixr::KeyframeAnimation *animation);

  static bool GetTimestamps(const draco_illixr::KeyframeAnimation &animation,
                            DracoFloat32Array *timestamp);

  static bool GetKeyframes(const draco_illixr::KeyframeAnimation &animation,
                           int keyframes_id, DracoFloat32Array *animation_data);

 private:
  draco_illixr::KeyframeAnimationDecoder decoder_;
  draco_illixr::Status last_status_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRIPTEN_ANIMATION_DECODER_WEBIDL_WRAPPER_H_
