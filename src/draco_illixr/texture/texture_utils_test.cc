// Copyright 2019 The Draco Authors.
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
#include "draco_illixr/texture/texture_utils.h"

#include <utility>

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco_illixr/core/draco_test_utils.h"
#include "draco_illixr/io/texture_io.h"
#include "draco_illixr/texture/color_utils.h"

namespace {

TEST(TextureUtilsTest, TestGetTargetNameForTextureLoadedFromFile) {
  // Tests that correct target stem and format are returned by texture utils for
  // texture loaded from image file (stem and format from source file).
  std::unique_ptr<draco_illixr::Texture> texture =
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("fast.jpg"))
          .value();
  ASSERT_NE(texture, nullptr);
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetStem(*texture), "fast");
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetExtension(*texture), "jpg");
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetFormat(*texture),
            draco_illixr::ImageFormat::JPEG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetOrGenerateTargetStem(*texture, 5, "_Color"),
            "fast");
}

TEST(TextureUtilsTest, TestGetTargetNameForNewTexture) {
  // Tests that correct target stem and format are returned by texture utils for
  // a newly created texture (empty stem and PNG image type by default).
  std::unique_ptr<draco_illixr::Texture> texture(new draco_illixr::Texture());
  ASSERT_NE(texture, nullptr);
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetStem(*texture), "");
  ASSERT_EQ(draco_illixr::TextureUtils::GetOrGenerateTargetStem(*texture, 5, "_Color"),
            "Texture5_Color");
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetExtension(*texture), "png");
  ASSERT_EQ(draco_illixr::TextureUtils::GetTargetFormat(*texture),
            draco_illixr::ImageFormat::PNG);
}

TEST(TextureUtilsTest, TestGetSourceFormat) {
  // Tests that the source format is determined correctly for new textures and
  // for textures loaded from file.
  std::unique_ptr<draco_illixr::Texture> new_texture(new draco_illixr::Texture());
  DRACO_ASSIGN_OR_ASSERT(
      std::unique_ptr<draco_illixr::Texture> png_texture,
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("test.png")));
  DRACO_ASSIGN_OR_ASSERT(
      std::unique_ptr<draco_illixr::Texture> jpg_texture,
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("fast.jpg")));

  // Check source formats.
  ASSERT_EQ(draco_illixr::TextureUtils::GetSourceFormat(*new_texture),
            draco_illixr::ImageFormat::PNG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetSourceFormat(*png_texture),
            draco_illixr::ImageFormat::PNG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetSourceFormat(*jpg_texture),
            draco_illixr::ImageFormat::JPEG);

  // Remove the mime-type from the jpeg texture and ensure the source format is
  // still detected properly based on the filename.
  jpg_texture->source_image().set_mime_type("");
  ASSERT_EQ(draco_illixr::TextureUtils::GetSourceFormat(*jpg_texture),
            draco_illixr::ImageFormat::JPEG);
}

TEST(TextureUtilsTest, TestGetFormat) {
  typedef draco_illixr::ImageFormat ImageFormat;
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("png"), ImageFormat::PNG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("jpg"), ImageFormat::JPEG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("jpeg"), ImageFormat::JPEG);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("basis"), ImageFormat::BASIS);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("ktx2"), ImageFormat::BASIS);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("webp"), ImageFormat::WEBP);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat(""), ImageFormat::NONE);
  ASSERT_EQ(draco_illixr::TextureUtils::GetFormat("bmp"), ImageFormat::NONE);
}

TEST(TextureUtilsTest, TestGetExtension) {
  typedef draco_illixr::ImageFormat ImageFormat;
  ASSERT_EQ(draco_illixr::TextureUtils::GetExtension(ImageFormat::PNG), "png");
  ASSERT_EQ(draco_illixr::TextureUtils::GetExtension(ImageFormat::JPEG), "jpg");
  ASSERT_EQ(draco_illixr::TextureUtils::GetExtension(ImageFormat::BASIS), "ktx2");
  ASSERT_EQ(draco_illixr::TextureUtils::GetExtension(ImageFormat::WEBP), "webp");
  ASSERT_EQ(draco_illixr::TextureUtils::GetExtension(ImageFormat::NONE), "");
}

#ifdef DRACO_UNRLEASED_FEATURES
TEST(TextureUtilsTest, TestHasTargetImageFormat) {
  // Tests that the presence of image format in texture library can be detected.

  // Create test texture library with textures in PNG and JPEG image formats.
  draco_illixr::TextureLibrary library;
  library.PushTexture(
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("test.png"))
          .value());
  library.PushTexture(
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("fast.jpg"))
          .value());

  // Check target texture image formats.
  ASSERT_TRUE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::PNG));
  ASSERT_TRUE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::JPEG));
  ASSERT_FALSE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::BASIS));
  ASSERT_FALSE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::WEBP));

  // Change PNG texture image format to BASIS and check again.
  library.GetTexture(0)->GetMutableCompressionOptions().target_image_format =
      draco_illixr::ImageFormat::BASIS;
  ASSERT_FALSE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::PNG));
  ASSERT_TRUE(draco_illixr::TextureUtils::HasTargetImageFormat(
      library, draco_illixr::ImageFormat::BASIS));
}

TEST(TextureUtilsTest, TestFindMutableTextures) {
  // Tests that all textures from material library can be found.

  // Read a mesh that has multiple textures of various types.
  std::unique_ptr<draco_illixr::Mesh> mesh =
      draco_illixr::ReadMeshFromTestFile("SphereAllSame/sphere_texture_all.gltf");
  ASSERT_NE(mesh, nullptr);
  draco_illixr::MaterialLibrary &library = mesh->GetMaterialLibrary();
  ASSERT_EQ(library.NumMaterials(), 1);
  const draco_illixr::Material &material = *library.GetMaterial(0);
  ASSERT_EQ(material.NumTextureMaps(), 5);
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 4);

  // Check that all textures from material library can be found.
  const std::vector<draco_illixr::Texture *> textures =
      draco_illixr::TextureUtils::FindMutableTextures(&library);
  ASSERT_EQ(textures.size(), 4);
  ASSERT_EQ(textures[0], library.GetTextureLibrary().GetTexture(0));
  ASSERT_EQ(textures[1], library.GetTextureLibrary().GetTexture(1));
  ASSERT_EQ(textures[2], library.GetTextureLibrary().GetTexture(2));
  ASSERT_EQ(textures[3], library.GetTextureLibrary().GetTexture(3));
}

TEST(TextureUtilsTest, TestFindTexturesWithType) {
  // Tests that textures of a given type can be found.

  // Read a mesh that has multiple textures of various types.
  std::unique_ptr<draco_illixr::Mesh> mesh =
      draco_illixr::ReadMeshFromTestFile("SphereAllSame/sphere_texture_all.gltf");
  ASSERT_NE(mesh, nullptr);
  const draco_illixr::MaterialLibrary &library = mesh->GetMaterialLibrary();
  ASSERT_EQ(library.NumMaterials(), 1);
  const draco_illixr::Material &material = *library.GetMaterial(0);
  ASSERT_EQ(material.NumTextureMaps(), 5);
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 4);

  // Check that occlusion texture can be found.
  {
    const std::vector<const draco_illixr::Texture *> textures =
        draco_illixr::TextureUtils::FindTextures(draco_illixr::TextureMap::AMBIENT_OCCLUSION,
                                          &library);
    ASSERT_EQ(textures.size(), 1);
    ASSERT_EQ(textures[0],
              library.GetMaterial(0)
                  ->GetTextureMapByType(draco_illixr::TextureMap::AMBIENT_OCCLUSION)
                  ->texture());
  }

  // Check that metallic texture can be found.
  {
    const std::vector<const draco_illixr::Texture *> textures =
        draco_illixr::TextureUtils::FindTextures(draco_illixr::TextureMap::METALLIC_ROUGHNESS,
                                          &library);
    ASSERT_EQ(textures.size(), 1);
    ASSERT_EQ(textures[0],
              library.GetMaterial(0)
                  ->GetTextureMapByType(draco_illixr::TextureMap::METALLIC_ROUGHNESS)
                  ->texture());
  }
}

TEST(TextureUtilsTest, TestFindMutableTexturesWithType) {
  // Tests that textures of a given type can be found.

  // Read a mesh that has multiple textures of various types.
  std::unique_ptr<draco_illixr::Mesh> mesh =
      draco_illixr::ReadMeshFromTestFile("SphereAllSame/sphere_texture_all.gltf");
  ASSERT_NE(mesh, nullptr);
  draco_illixr::MaterialLibrary &library = mesh->GetMaterialLibrary();
  ASSERT_EQ(library.NumMaterials(), 1);
  const draco_illixr::Material &material = *library.GetMaterial(0);
  ASSERT_EQ(material.NumTextureMaps(), 5);
  ASSERT_EQ(library.GetTextureLibrary().NumTextures(), 4);

  // Check that occlusion texture can be found.
  {
    const std::vector<draco_illixr::Texture *> textures =
        draco_illixr::TextureUtils::FindMutableTextures(
            draco_illixr::TextureMap::AMBIENT_OCCLUSION, &library);
    ASSERT_EQ(textures.size(), 1);
    ASSERT_EQ(textures[0],
              library.GetMaterial(0)
                  ->GetTextureMapByType(draco_illixr::TextureMap::AMBIENT_OCCLUSION)
                  ->texture());
  }

  // Check that metallic texture can be found.
  {
    const std::vector<draco_illixr::Texture *> textures =
        draco_illixr::TextureUtils::FindMutableTextures(
            draco_illixr::TextureMap::METALLIC_ROUGHNESS, &library);
    ASSERT_EQ(textures.size(), 1);
    ASSERT_EQ(textures[0],
              library.GetMaterial(0)
                  ->GetTextureMapByType(draco_illixr::TextureMap::METALLIC_ROUGHNESS)
                  ->texture());
  }
}
#endif  // DRACO_UNRLEASED_FEATURES

TEST(TextureUtilsTest, TestComputeRequiredNumChannels) {
  // Tests that the number of texture channels can be computed. Material library
  // under test is created programmatically.

  // Load textures.
  DRACO_ASSIGN_OR_ASSERT(
      auto texture0, draco_illixr::ReadTextureFromFile(
                         draco_illixr::GetTestFileFullPath("fully_transparent.png")));
  ASSERT_NE(texture0, nullptr);
  draco_illixr::Texture *texture0_ptr = texture0.get();
  DRACO_ASSIGN_OR_ASSERT(
      auto texture1,
      draco_illixr::ReadTextureFromFile(draco_illixr::GetTestFileFullPath("squares.png")));
  ASSERT_NE(texture1, nullptr);
  const draco_illixr::Texture *texture1_ptr = texture1.get();
  DRACO_ASSIGN_OR_ASSERT(
      auto texture2, draco_illixr::ReadTextureFromFile(
                         draco_illixr::GetTestFileFullPath("fully_transparent.png")));
  ASSERT_NE(texture2, nullptr);
  const draco_illixr::Texture *texture2_ptr = texture2.get();

  // Compute number of channels for occlusion-only texture.
  draco_illixr::MaterialLibrary library;
  draco_illixr::Material *const material0 = library.MutableMaterial(0);
  material0->SetTextureMap(std::move(texture0),
                           draco_illixr::TextureMap::AMBIENT_OCCLUSION, 0);
  ASSERT_EQ(
      draco_illixr::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      1);

  // Compute number of channels for occlusion-only texture with MR present but
  // not using the same texture.
  draco_illixr::Material *const material1 = library.MutableMaterial(1);
  material1->SetTextureMap(std::move(texture1),
                           draco_illixr::TextureMap::METALLIC_ROUGHNESS, 0);
  ASSERT_EQ(
      draco_illixr::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      1);

  // Compute number of channels for metallic-roughness texture.
  ASSERT_EQ(
      draco_illixr::TextureUtils::ComputeRequiredNumChannels(*texture1_ptr, library),
      3);

  // Compute number of channels texture that is used for occlusin map in one
  // material and also shared with metallic-roughness map in another material.
  draco_illixr::Material *const material2 = library.MutableMaterial(2);
  DRACO_ASSERT_OK(material2->SetTextureMap(
      texture0_ptr, draco_illixr::TextureMap::METALLIC_ROUGHNESS, 0));
  ASSERT_EQ(
      draco_illixr::TextureUtils::ComputeRequiredNumChannels(*texture0_ptr, library),
      3);

  // Compute number of channels for non-opaque texture.
  material0->SetTextureMap(std::move(texture2), draco_illixr::TextureMap::COLOR, 0);
  ASSERT_EQ(
      draco_illixr::TextureUtils::ComputeRequiredNumChannels(*texture2_ptr, library),
      4);
}

}  // namespace

#endif  // DRACO_TRANSCODER_SUPPORTED
