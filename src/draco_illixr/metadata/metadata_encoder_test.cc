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
#include "draco_illixr/metadata/metadata_encoder.h"

#include "draco_illixr/core/decoder_buffer.h"
#include "draco_illixr/core/draco_test_base.h"
#include "draco_illixr/core/encoder_buffer.h"
#include "draco_illixr/metadata/metadata.h"
#include "draco_illixr/metadata/metadata_decoder.h"

namespace {

class MetadataEncoderTest : public ::testing::Test {
 protected:
  MetadataEncoderTest() {}

  void TestEncodingMetadata() {
    ASSERT_TRUE(encoder.EncodeMetadata(&encoder_buffer, &metadata));

    draco_illixr::Metadata decoded_metadata;
    decoder_buffer.Init(encoder_buffer.data(), encoder_buffer.size());
    ASSERT_TRUE(decoder.DecodeMetadata(&decoder_buffer, &decoded_metadata));
    CheckMetadatasAreEqual(metadata, decoded_metadata);
  }

  void TestEncodingGeometryMetadata() {
    ASSERT_TRUE(
        encoder.EncodeGeometryMetadata(&encoder_buffer, &geometry_metadata));

    draco_illixr::GeometryMetadata decoded_metadata;
    decoder_buffer.Init(encoder_buffer.data(), encoder_buffer.size());
    ASSERT_TRUE(
        decoder.DecodeGeometryMetadata(&decoder_buffer, &decoded_metadata));
    CheckGeometryMetadatasAreEqual(geometry_metadata, decoded_metadata);
  }

  void CheckBlobOfDataAreEqual(const std::vector<uint8_t> &data0,
                               const std::vector<uint8_t> &data1) {
    ASSERT_EQ(data0.size(), data1.size());
    for (int i = 0; i < data0.size(); ++i) {
      ASSERT_EQ(data0[i], data1[i]);
    }
  }

  void CheckGeometryMetadatasAreEqual(
      const draco_illixr::GeometryMetadata &metadata0,
      const draco_illixr::GeometryMetadata &metadata1) {
    ASSERT_EQ(metadata0.attribute_metadatas().size(),
              metadata1.attribute_metadatas().size());
    const std::vector<std::unique_ptr<draco_illixr::AttributeMetadata>>
        &att_metadatas0 = metadata0.attribute_metadatas();
    const std::vector<std::unique_ptr<draco_illixr::AttributeMetadata>>
        &att_metadatas1 = metadata1.attribute_metadatas();
    // Compare each attribute metadata.
    for (int i = 0; i < metadata0.attribute_metadatas().size(); ++i) {
      CheckMetadatasAreEqual(
          static_cast<const draco_illixr::Metadata &>(*att_metadatas0[i]),
          static_cast<const draco_illixr::Metadata &>(*att_metadatas1[i]));
    }
    // Compare entries and sub metadata.
    CheckMetadatasAreEqual(static_cast<const draco_illixr::Metadata &>(metadata0),
                           static_cast<const draco_illixr::Metadata &>(metadata1));
  }

  void CheckMetadatasAreEqual(const draco_illixr::Metadata &metadata0,
                              const draco_illixr::Metadata &metadata1) {
    ASSERT_EQ(metadata0.num_entries(), metadata1.num_entries());
    const std::map<std::string, draco_illixr::EntryValue> &entries0 =
        metadata0.entries();
    const std::map<std::string, draco_illixr::EntryValue> &entries1 =
        metadata1.entries();
    for (const auto &entry : entries0) {
      const std::string &entry_name = entry.first;
      const std::vector<uint8_t> &data0 = entry.second.data();
      const auto entry1_ptr = entries1.find(entry_name);
      ASSERT_NE(entry1_ptr, entries1.end());
      const std::vector<uint8_t> &data1 = entry1_ptr->second.data();
      CheckBlobOfDataAreEqual(data0, data1);
    }
    // Check nested metadata.
    ASSERT_EQ(metadata0.sub_metadatas().size(),
              metadata1.sub_metadatas().size());
    const std::map<std::string, std::unique_ptr<draco_illixr::Metadata>>
        &sub_metadatas0 = metadata0.sub_metadatas();
    // Encode each sub-metadata
    for (auto &&sub_metadata_entry0 : sub_metadatas0) {
      const auto sub_metadata_ptr1 =
          metadata1.GetSubMetadata(sub_metadata_entry0.first);
      ASSERT_NE(sub_metadata_ptr1, nullptr);
      CheckMetadatasAreEqual(*sub_metadata_entry0.second, *sub_metadata_ptr1);
    }
  }

  draco_illixr::MetadataEncoder encoder;
  draco_illixr::MetadataDecoder decoder;
  draco_illixr::EncoderBuffer encoder_buffer;
  draco_illixr::DecoderBuffer decoder_buffer;
  draco_illixr::Metadata metadata;
  draco_illixr::GeometryMetadata geometry_metadata;
};

TEST_F(MetadataEncoderTest, TestSingleEntry) {
  metadata.AddEntryInt("int", 100);
  ASSERT_EQ(metadata.num_entries(), 1);

  TestEncodingMetadata();
}

TEST_F(MetadataEncoderTest, TestMultipleEntries) {
  metadata.AddEntryInt("int", 100);
  metadata.AddEntryDouble("double", 1.234);
  const std::string entry_value = "test string entry";
  metadata.AddEntryString("string", entry_value);
  ASSERT_EQ(metadata.num_entries(), 3);

  TestEncodingMetadata();
}

TEST_F(MetadataEncoderTest, TestEncodingArrayEntries) {
  std::vector<int32_t> int_array({1, 2, 3});
  metadata.AddEntryIntArray("int_array", int_array);
  std::vector<double> double_array({0.1, 0.2, 0.3});
  metadata.AddEntryDoubleArray("double_array", double_array);
  ASSERT_EQ(metadata.num_entries(), 2);

  TestEncodingMetadata();
}

TEST_F(MetadataEncoderTest, TestEncodingBinaryEntry) {
  const std::vector<uint8_t> binarydata({0x1, 0x2, 0x3, 0x4});
  metadata.AddEntryBinary("binary_data", binarydata);

  TestEncodingMetadata();
}

TEST_F(MetadataEncoderTest, TestEncodingNestedMetadata) {
  metadata.AddEntryDouble("double", 1.234);
  std::unique_ptr<draco_illixr::Metadata> sub_metadata =
      std::unique_ptr<draco_illixr::Metadata>(new draco_illixr::Metadata());
  sub_metadata->AddEntryInt("int", 100);
  metadata.AddSubMetadata("sub0", std::move(sub_metadata));

  TestEncodingMetadata();
}

TEST_F(MetadataEncoderTest, TestEncodingGeometryMetadata) {
  std::unique_ptr<draco_illixr::AttributeMetadata> att_metadata =
      std::unique_ptr<draco_illixr::AttributeMetadata>(new draco_illixr::AttributeMetadata);
  att_metadata->AddEntryInt("int", 100);
  att_metadata->AddEntryString("name", "pos");
  ASSERT_TRUE(geometry_metadata.AddAttributeMetadata(std::move(att_metadata)));

  TestEncodingGeometryMetadata();
}
}  // namespace
