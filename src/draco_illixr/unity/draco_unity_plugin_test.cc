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
#include "draco_illixr/unity/draco_unity_plugin.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "draco_illixr/core/draco_test_base.h"
#include "draco_illixr/core/draco_test_utils.h"

namespace {

draco_illixr::DracoMesh *DecodeToDracoMesh(const std::string &file_name) {
  std::ifstream input_file(draco_illixr::GetTestFileFullPath(file_name),
                           std::ios::binary);
  if (!input_file) {
    return nullptr;
  }
  // Read the file stream into a buffer.
  std::streampos file_size = 0;
  input_file.seekg(0, std::ios::end);
  file_size = input_file.tellg() - file_size;
  input_file.seekg(0, std::ios::beg);
  std::vector<char> data(file_size);
  input_file.read(data.data(), file_size);
  if (data.empty()) {
    return nullptr;
  }

  draco_illixr::DracoMesh *draco_mesh = nullptr;
  draco_illixr::DecodeDracoMesh(data.data(), data.size(), &draco_mesh);
  return draco_mesh;
}

TEST(DracoUnityPluginTest, TestDecode) {
  draco_illixr::DracoMesh *draco_mesh =
      DecodeToDracoMesh("test_nm.obj.edgebreaker.cl4.2.2.drc");
  ASSERT_NE(draco_mesh, nullptr);
  ASSERT_EQ(draco_mesh->num_faces, 170);
  ASSERT_EQ(draco_mesh->num_vertices, 99);
  ASSERT_NE(draco_mesh->private_mesh, nullptr);

  draco_illixr::DracoData *indices = nullptr;
  ASSERT_TRUE(GetMeshIndices(draco_mesh, &indices));
  ASSERT_EQ(indices->data_type, draco_illixr::DT_INT32);
  draco_illixr::ReleaseDracoData(&indices);

  for (int i = 0; i < draco_mesh->num_attributes; ++i) {
    draco_illixr::DracoAttribute *draco_attribute = nullptr;
    ASSERT_TRUE(draco_illixr::GetAttribute(draco_mesh, i, &draco_attribute));
    ASSERT_NE(draco_attribute->data_type, draco_illixr::DT_INVALID);
    ASSERT_GT(draco_attribute->num_components, 0);
    ASSERT_NE(draco_attribute->private_attribute, nullptr);

    draco_illixr::DracoData *attribute_data = nullptr;
    ASSERT_TRUE(
        draco_illixr::GetAttributeData(draco_mesh, draco_attribute, &attribute_data));
    draco_illixr::ReleaseDracoData(&attribute_data);
    draco_illixr::ReleaseDracoAttribute(&draco_attribute);
  }
  draco_illixr::ReleaseDracoMesh(&draco_mesh);
}

TEST(DracoUnityPluginTest, TestAttributeTypes) {
  draco_illixr::DracoMesh *draco_mesh = DecodeToDracoMesh("color_attr.drc");
  ASSERT_NE(draco_mesh, nullptr);

  draco_illixr::DracoAttribute *pos_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::POSITION, 0, &pos_attribute));
  ASSERT_EQ(pos_attribute->attribute_type, draco_illixr::GeometryAttribute::POSITION);
  ASSERT_EQ(pos_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(pos_attribute->num_components, 3);
  ASSERT_EQ(pos_attribute->unique_id, 0);
  ASSERT_NE(pos_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&pos_attribute);

  draco_illixr::DracoAttribute *color_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::COLOR, 0, &color_attribute));
  ASSERT_EQ(color_attribute->attribute_type, draco_illixr::GeometryAttribute::COLOR);
  ASSERT_EQ(color_attribute->data_type, draco_illixr::DT_UINT8);
  ASSERT_EQ(color_attribute->num_components, 4);
  ASSERT_EQ(color_attribute->unique_id, 1);
  ASSERT_NE(color_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&color_attribute);

  draco_illixr::DracoAttribute *bad_attribute = nullptr;
  ASSERT_FALSE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::NORMAL, 0, &bad_attribute));
  ASSERT_FALSE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::TEX_COORD, 0, &bad_attribute));
  ASSERT_FALSE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::GENERIC, 0, &bad_attribute));

  draco_illixr::ReleaseDracoMesh(&draco_mesh);

  draco_mesh = DecodeToDracoMesh("cube_att_sub_o_2.drc");
  ASSERT_NE(draco_mesh, nullptr);

  draco_illixr::DracoAttribute *norm_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::NORMAL, 0, &norm_attribute));
  ASSERT_EQ(norm_attribute->attribute_type, draco_illixr::GeometryAttribute::NORMAL);
  ASSERT_EQ(norm_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(norm_attribute->num_components, 3);
  ASSERT_EQ(norm_attribute->unique_id, 2);
  ASSERT_NE(norm_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&norm_attribute);

  draco_illixr::DracoAttribute *texcoord_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::TEX_COORD, 0, &texcoord_attribute));
  ASSERT_EQ(texcoord_attribute->attribute_type,
            draco_illixr::GeometryAttribute::TEX_COORD);
  ASSERT_EQ(texcoord_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(texcoord_attribute->num_components, 2);
  ASSERT_EQ(texcoord_attribute->unique_id, 1);
  ASSERT_NE(texcoord_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&texcoord_attribute);

  draco_illixr::DracoAttribute *generic_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::GENERIC, 0, &generic_attribute));
  ASSERT_EQ(generic_attribute->attribute_type,
            draco_illixr::GeometryAttribute::GENERIC);
  ASSERT_EQ(generic_attribute->data_type, draco_illixr::DT_UINT8);
  ASSERT_EQ(generic_attribute->num_components, 1);
  ASSERT_EQ(generic_attribute->unique_id, 3);
  ASSERT_NE(generic_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&generic_attribute);

  ASSERT_FALSE(draco_illixr::GetAttributeByType(
      draco_mesh, draco_illixr::GeometryAttribute::TEX_COORD, 1, &bad_attribute));

  draco_illixr::ReleaseDracoMesh(&draco_mesh);
}

TEST(DracoUnityPluginTest, TestAttributeUniqueId) {
  draco_illixr::DracoMesh *draco_mesh = DecodeToDracoMesh("cube_att_sub_o_2.drc");
  ASSERT_NE(draco_mesh, nullptr);

  draco_illixr::DracoAttribute *pos_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByUniqueId(draco_mesh, 0, &pos_attribute));
  ASSERT_EQ(pos_attribute->attribute_type, draco_illixr::GeometryAttribute::POSITION);
  ASSERT_EQ(pos_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(pos_attribute->num_components, 3);
  ASSERT_EQ(pos_attribute->unique_id, 0);
  ASSERT_NE(pos_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&pos_attribute);

  draco_illixr::DracoAttribute *norm_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByUniqueId(draco_mesh, 2, &norm_attribute));
  ASSERT_EQ(norm_attribute->attribute_type, draco_illixr::GeometryAttribute::NORMAL);
  ASSERT_EQ(norm_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(norm_attribute->num_components, 3);
  ASSERT_EQ(norm_attribute->unique_id, 2);
  ASSERT_NE(norm_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&norm_attribute);

  draco_illixr::DracoAttribute *texcoord_attribute = nullptr;
  ASSERT_TRUE(
      draco_illixr::GetAttributeByUniqueId(draco_mesh, 1, &texcoord_attribute));
  ASSERT_EQ(texcoord_attribute->attribute_type,
            draco_illixr::GeometryAttribute::TEX_COORD);
  ASSERT_EQ(texcoord_attribute->data_type, draco_illixr::DT_FLOAT32);
  ASSERT_EQ(texcoord_attribute->num_components, 2);
  ASSERT_EQ(texcoord_attribute->unique_id, 1);
  ASSERT_NE(texcoord_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&texcoord_attribute);

  draco_illixr::DracoAttribute *generic_attribute = nullptr;
  ASSERT_TRUE(draco_illixr::GetAttributeByUniqueId(draco_mesh, 3, &generic_attribute));
  ASSERT_EQ(generic_attribute->attribute_type,
            draco_illixr::GeometryAttribute::GENERIC);
  ASSERT_EQ(generic_attribute->data_type, draco_illixr::DT_UINT8);
  ASSERT_EQ(generic_attribute->num_components, 1);
  ASSERT_EQ(generic_attribute->unique_id, 3);
  ASSERT_NE(generic_attribute->private_attribute, nullptr);
  draco_illixr::ReleaseDracoAttribute(&generic_attribute);

  draco_illixr::DracoAttribute *bad_attribute = nullptr;
  ASSERT_FALSE(draco_illixr::GetAttributeByUniqueId(draco_mesh, 4, &bad_attribute));

  draco_illixr::ReleaseDracoMesh(&draco_mesh);
}

class DeprecatedDracoUnityPluginTest : public ::testing::Test {
 protected:
  DeprecatedDracoUnityPluginTest() : unity_mesh_(nullptr) {}

  void TestDecodingToDracoUnityMesh(const std::string &file_name,
                                    int expected_num_faces,
                                    int expected_num_vertices) {
    // Tests that decoders can successfully skip attribute transform.
    std::ifstream input_file(draco_illixr::GetTestFileFullPath(file_name),
                             std::ios::binary);
    ASSERT_TRUE(input_file);

    // Read the file stream into a buffer.
    std::streampos file_size = 0;
    input_file.seekg(0, std::ios::end);
    file_size = input_file.tellg() - file_size;
    input_file.seekg(0, std::ios::beg);
    std::vector<char> data(file_size);
    input_file.read(data.data(), file_size);

    ASSERT_FALSE(data.empty());

    const int num_faces =
        draco_illixr::DecodeMeshForUnity(data.data(), data.size(), &unity_mesh_);

    ASSERT_EQ(num_faces, expected_num_faces);
    ASSERT_EQ(unity_mesh_->num_faces, expected_num_faces);
    ASSERT_EQ(unity_mesh_->num_vertices, expected_num_vertices);
    ASSERT_TRUE(unity_mesh_->has_normal);
    ASSERT_NE(unity_mesh_->normal, nullptr);
    // TODO(fgalligan): Also test color and tex_coord attributes.

    draco_illixr::ReleaseUnityMesh(&unity_mesh_);
  }

  draco_illixr::DracoToUnityMesh *unity_mesh_;
};

TEST_F(DeprecatedDracoUnityPluginTest, DeprecatedDecodingToDracoUnityMesh) {
  TestDecodingToDracoUnityMesh("test_nm.obj.edgebreaker.1.0.0.drc", 170, 99);
}
}  // namespace
