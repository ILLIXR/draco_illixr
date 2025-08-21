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
#ifndef DRACO_JAVASCRIPT_EMSCRIPTEN_ENCODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRIPTEN_ENCODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco_illixr/attributes/point_attribute.h"
#include "draco_illixr/compression/config/compression_shared.h"
#include "draco_illixr/compression/config/encoder_options.h"
#include "draco_illixr/compression/encode.h"
#include "draco_illixr/compression/expert_encode.h"
#include "draco_illixr/mesh/mesh.h"

typedef draco_illixr::GeometryAttribute draco_GeometryAttribute;
typedef draco_illixr::GeometryAttribute::Type draco_GeometryAttribute_Type;
typedef draco_illixr::EncodedGeometryType draco_EncodedGeometryType;
typedef draco_illixr::MeshEncoderMethod draco_MeshEncoderMethod;

class DracoInt8Array {
 public:
  DracoInt8Array();
  int8_t GetValue(int index) const;
  bool SetValues(const char *values, int count);

  size_t size() { return values_.size(); }

 private:
  std::vector<int8_t> values_;
};

class MetadataBuilder {
 public:
  MetadataBuilder();
  bool AddStringEntry(draco_illixr::Metadata *metadata, const char *entry_name,
                      const char *entry_value);
  bool AddIntEntry(draco_illixr::Metadata *metadata, const char *entry_name,
                   long entry_value);
  bool AddIntEntryArray(draco_illixr::Metadata *metadata, const char *entry_name,
                        const int32_t *entry_values, int32_t num_values);
  bool AddDoubleEntry(draco_illixr::Metadata *metadata, const char *entry_name,
                      double entry_value);
};

class PointCloudBuilder {
 public:
  PointCloudBuilder() {}
  int AddFloatAttribute(draco_illixr::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const float *att_values);
  int AddInt8Attribute(draco_illixr::PointCloud *pc, draco_GeometryAttribute_Type type,
                       long num_vertices, long num_components,
                       const char *att_values);
  int AddUInt8Attribute(draco_illixr::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const uint8_t *att_values);
  int AddInt16Attribute(draco_illixr::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const int16_t *att_values);
  int AddUInt16Attribute(draco_illixr::PointCloud *pc,
                         draco_GeometryAttribute_Type type, long num_vertices,
                         long num_components, const uint16_t *att_values);
  int AddInt32Attribute(draco_illixr::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const int32_t *att_values);
  int AddUInt32Attribute(draco_illixr::PointCloud *pc,
                         draco_GeometryAttribute_Type type, long num_vertices,
                         long num_components, const uint32_t *att_values);
  bool SetMetadataForAttribute(draco_illixr::PointCloud *pc, long attribute_id,
                               const draco_illixr::Metadata *metadata);
  bool AddMetadata(draco_illixr::PointCloud *pc, const draco_illixr::Metadata *metadata);

 private:
  template <typename DataTypeT>
  int AddAttribute(draco_illixr::PointCloud *pc, draco_GeometryAttribute_Type type,
                   long num_vertices, long num_components,
                   const DataTypeT *att_values,
                   draco_illixr::DataType draco_data_type) {
    if (!pc) {
      return -1;
    }
    std::unique_ptr<draco_illixr::PointAttribute> att(new draco_illixr::PointAttribute());
    att->Init(type, num_components, draco_data_type,
              /* normalized */ false, num_vertices);
    const int att_id = pc->AddAttribute(std::move(att));
    draco_illixr::PointAttribute *const att_ptr = pc->attribute(att_id);

    for (draco_illixr::PointIndex i(0); i < num_vertices; ++i) {
      att_ptr->SetAttributeValue(att_ptr->mapped_index(i),
                                 &att_values[i.value() * num_components]);
    }
    if (pc->num_points() == 0) {
      pc->set_num_points(num_vertices);
    } else if (pc->num_points() != num_vertices) {
      return -1;
    }
    return att_id;
  }
};

// TODO(draco-eng): Regenerate wasm decoder.
// TODO(draco-eng): Add script to generate and test all Javascipt code.
class MeshBuilder : public PointCloudBuilder {
 public:
  MeshBuilder();

  bool AddFacesToMesh(draco_illixr::Mesh *mesh, long num_faces, const int *faces);

  // Deprecated: Use AddFloatAttribute() instead.
  int AddFloatAttributeToMesh(draco_illixr::Mesh *mesh,
                              draco_GeometryAttribute_Type type,
                              long num_vertices, long num_components,
                              const float *att_values);

  // Deprecated: Use AddInt32Attribute() instead.
  int AddInt32AttributeToMesh(draco_illixr::Mesh *mesh,
                              draco_GeometryAttribute_Type type,
                              long num_vertices, long num_components,
                              const int32_t *att_values);

  // Deprecated: Use AddMetadata() instead.
  bool AddMetadataToMesh(draco_illixr::Mesh *mesh, const draco_illixr::Metadata *metadata);
};

class Encoder {
 public:
  Encoder();

  void SetEncodingMethod(long method);
  void SetAttributeQuantization(draco_GeometryAttribute_Type type,
                                long quantization_bits);
  void SetAttributeExplicitQuantization(draco_GeometryAttribute_Type type,
                                        long quantization_bits,
                                        long num_components,
                                        const float *origin, float range);
  void SetSpeedOptions(long encoding_speed, long decoding_speed);
  void SetTrackEncodedProperties(bool flag);

  int EncodeMeshToDracoBuffer(draco_illixr::Mesh *mesh, DracoInt8Array *buffer);

  int EncodePointCloudToDracoBuffer(draco_illixr::PointCloud *pc,
                                    bool deduplicate_values,
                                    DracoInt8Array *buffer);
  int GetNumberOfEncodedPoints();
  int GetNumberOfEncodedFaces();

 private:
  draco_illixr::Encoder encoder_;
};

class ExpertEncoder {
 public:
  ExpertEncoder(draco_illixr::PointCloud *pc);

  void SetEncodingMethod(long method);
  void SetAttributeQuantization(long att_id, long quantization_bits);
  void SetAttributeExplicitQuantization(long att_id, long quantization_bits,
                                        long num_components,
                                        const float *origin, float range);
  void SetSpeedOptions(long encoding_speed, long decoding_speed);
  void SetTrackEncodedProperties(bool flag);

  int EncodeToDracoBuffer(bool deduplicate_values, DracoInt8Array *buffer);

  int GetNumberOfEncodedPoints();
  int GetNumberOfEncodedFaces();

 private:
  std::unique_ptr<draco_illixr::ExpertEncoder> encoder_;

  draco_illixr::PointCloud *pc_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRIPTEN_ENCODER_WEBIDL_WRAPPER_H_
