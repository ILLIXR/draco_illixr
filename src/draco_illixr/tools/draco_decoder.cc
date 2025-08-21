// Copyright 2016 The Draco Authors.
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
#include <cinttypes>

#include "draco_illixr/compression/decode.h"
#include "draco_illixr/core/cycle_timer.h"
#include "draco_illixr/io/file_utils.h"
#include "draco_illixr/io/obj_encoder.h"
#include "draco_illixr/io/parser_utils.h"
#include "draco_illixr/io/ply_encoder.h"
#include "draco_illixr/io/stl_encoder.h"

namespace {

struct Options {
  Options();

  std::string input;
  std::string output;
};

Options::Options() {}

void Usage() {
  printf("Usage: draco_decoder [options] -i input\n");
  printf("\n");
  printf("Main options:\n");
  printf("  -h | -?               show help.\n");
  printf("  -o <output>           output file name.\n");
}

int ReturnError(const draco_illixr::Status &status) {
  printf("Failed to decode the input file %s\n", status.error_msg());
  return -1;
}

}  // namespace

int main(int argc, char **argv) {
  Options options;
  const int argc_check = argc - 1;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp("-h", argv[i]) || !strcmp("-?", argv[i])) {
      Usage();
      return 0;
    } else if (!strcmp("-i", argv[i]) && i < argc_check) {
      options.input = argv[++i];
    } else if (!strcmp("-o", argv[i]) && i < argc_check) {
      options.output = argv[++i];
    }
  }
  if (argc < 3 || options.input.empty()) {
    Usage();
    return -1;
  }

  std::vector<char> data;
  draco_illixr::CycleTimer total_timer;
  total_timer.Start();
  if (!draco_illixr::ReadFileToBuffer(options.input, &data)) {
    printf("Failed opening the input file.\n");
    return -1;
  }

  if (data.empty()) {
    printf("Empty input file.\n");
    return -1;
  }

  // Create a draco decoding buffer. Note that no data is copied in this step.
  draco_illixr::DecoderBuffer buffer;

  buffer.Init(data.data(), data.size());

  draco_illixr::CycleTimer timer;
  //pyh add additional timers
  draco_illixr::CycleTimer store_mesh_timer;
  // Decode the input data into a geometry.
  std::unique_ptr<draco_illixr::PointCloud> pc;
  draco_illixr::Mesh *mesh = nullptr;
  auto type_statusor = draco_illixr::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return ReturnError(type_statusor.status());
  }
  const draco_illixr::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type == draco_illixr::TRIANGULAR_MESH) {
    timer.Start();
    draco_illixr::Decoder decoder;
    auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
    if (!statusor.ok()) {
      return ReturnError(statusor.status());
    }
    std::unique_ptr<draco_illixr::Mesh> in_mesh = std::move(statusor).value();
    timer.Stop();
    if (in_mesh) {
      mesh = in_mesh.get();
      pc = std::move(in_mesh);
    }
  } else if (geom_type == draco_illixr::POINT_CLOUD) {
    // Failed to decode it as mesh, so let's try to decode it as a point cloud.
    timer.Start();
    draco_illixr::Decoder decoder;
    auto statusor = decoder.DecodePointCloudFromBuffer(&buffer);
    if (!statusor.ok()) {
      return ReturnError(statusor.status());
    }
    pc = std::move(statusor).value();
    timer.Stop();
  }

  if (pc == nullptr) {
    printf("Failed to decode the input file.\n");
    return -1;
  }

  if (options.output.empty()) {
    // Save the output model into a ply file.
    options.output = options.input + ".ply";
  }

  // Save the decoded geometry into a file.
  const std::string extension = draco_illixr::parser::ToLower(
      options.output.size() >= 4
          ? options.output.substr(options.output.size() - 4)
          : options.output);

  store_mesh_timer.Start();
  if (extension == ".obj") {
    draco_illixr::ObjEncoder obj_encoder;
    if (mesh) {
      if (!obj_encoder.EncodeToFile(*mesh, options.output)) {
        printf("Failed to store the decoded mesh as OBJ.\n");
        return -1;
      }
    } else {
      if (!obj_encoder.EncodeToFile(*pc, options.output)) {
        printf("Failed to store the decoded point cloud as OBJ.\n");
        return -1;
      }
    }
  } else if (extension == ".ply") {
    draco_illixr::PlyEncoder ply_encoder;
    if (mesh) {
      if (!ply_encoder.EncodeToFile(*mesh, options.output)) {
        printf("Failed to store the decoded mesh as PLY.\n");
        return -1;
      }
    } else {
      if (!ply_encoder.EncodeToFile(*pc, options.output)) {
        printf("Failed to store the decoded point cloud as PLY.\n");
        return -1;
      }
    }
  } else if (extension == ".stl") {
    draco_illixr::StlEncoder stl_encoder;
    if (mesh) {
      draco_illixr::Status s = stl_encoder.EncodeToFile(*mesh, options.output);
      if (s.code() != draco_illixr::Status::OK) {
        printf("Failed to store the decoded mesh as STL.\n");
        return -1;
      }
    } else {
      printf("Can't store a point cloud as STL.\n");
      return -1;
    }
  } else {
    printf("Invalid output file extension. Use .obj .ply or .stl.\n");
    return -1;
  }
  store_mesh_timer.Stop();
  total_timer.Stop();
  printf("pyh: store decoded mesh to file takes %" PRId64 " ms.\n", store_mesh_timer.GetInMs());
  printf("pyh: total decoding takes %" PRId64 " ms.\n", total_timer.GetInMs());
  printf("Decoded geometry saved to %s (%" PRId64 " ms to decode)\n",
         options.output.c_str(), timer.GetInMs());
  return 0;
}
