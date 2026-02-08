#include "engine/tachyon_console.h"
#include "engine/tachyon_loaders.h"

/**
 * --------------
 * AbstractLoader
 * --------------
 */
void AbstractLoader::fillBufferUntil(const std::string& end) {
  if (!isLoading) {
    return;
  }

  setChunkDelimiter(end);

  int c;

  while (!isAtDelimiter() && !isAtEOL() && (c = nextChar()) != EOF) {
    buffer += (char)c;
  }

  if (c == EOF) {
    fclose(file);

    isLoading = false;
  } else if (isAtDelimiter()) {
    int pos = (int)(buffer.length() - delimiter.length());
    int len = delimiter.length();

    // Ignore the delimiter string at the end of the buffer
    // so chunks can be cleanly parsed with only their contents.
    buffer.erase(pos, len);
  }
}

bool AbstractLoader::bufferEndsWith(const std::string& str) {
  int pos = std::max((int)(buffer.length() - str.length()), 0);
  int len = str.length();

  return buffer.length() > 0 && buffer.compare(pos, len, str) == 0;
}

bool AbstractLoader::isAtDelimiter() {
  return bufferEndsWith(delimiter);
}

bool AbstractLoader::isAtEOL() {
  return bufferEndsWith("\n");
}

void AbstractLoader::load(const char* filePath) {
  FILE* f = fopen(filePath, "r");

  if (!f) {
    add_console_message("Failed to load file: " + std::string(filePath), tVec3f(1.f, 0, 0));

    return;
  }

  file = f;
  isLoading = true;
}

int AbstractLoader::nextChar() {
  return fgetc(file);
}

const std::string& AbstractLoader::readNextChunk() {
  buffer.clear();

  fillBufferUntil(delimiter);

  return buffer.size() == 0 && isLoading ? readNextChunk() : buffer;
}

void AbstractLoader::nextLine() {
  fillBufferUntil("\n");

  buffer.clear();
}

void AbstractLoader::setChunkDelimiter(const std::string& delimiter) {
  this->delimiter = delimiter;
}

/**
 * ---------
 * ObjLoader
 * ---------
 */
static std::string VERTEX_LABEL = "v";
static std::string TEXTURE_COORDINATE_LABEL = "vt";
static std::string NORMAL_LABEL = "vn";
static std::string FACE_LABEL = "f";

ObjLoader::ObjLoader(const char* path) {
  load(path);

  if (!isLoading) {
    printf("\033[33m" "[ObjLoader] OBJ file failed to load: %s\n" "\033[0m", path);
  }

  while (isLoading) {
    setChunkDelimiter(" ");

    const std::string& label = readNextChunk();

    if (label == VERTEX_LABEL) {
      handleVertex();
    } else if (label == TEXTURE_COORDINATE_LABEL) {
      handleTextureCoordinate();
    } else if (label == NORMAL_LABEL) {
      handleNormal();
    } else if (label == FACE_LABEL) {
      handleFace();
    }

    nextLine();
  }

  if (vertices.size() == 0 || faces.size() == 0) {
    printf("[ObjLoader] No data read from file: %s\n", path);
  }
}

ObjLoader::~ObjLoader() {
  vertices.clear();
  textureCoordinates.clear();
  normals.clear();
  faces.clear();
}

void ObjLoader::handleFace() {
  Face face;

  face.v1 = parseVertexData(readNextChunk());
  face.v2 = parseVertexData(readNextChunk());
  face.v3 = parseVertexData(readNextChunk());

  faces.push_back(face);
}

void ObjLoader::handleNormal() {
  float x = stof(readNextChunk());
  float y = stof(readNextChunk());
  float z = stof(readNextChunk());

  normals.push_back({ x, y, z });
}

void ObjLoader::handleVertex() {
  float x = stof(readNextChunk());
  float y = stof(readNextChunk());
  float z = stof(readNextChunk());

  vertices.push_back({ x, y, z });
}

void ObjLoader::handleTextureCoordinate() {
  float u = stof(readNextChunk());
  float v = stof(readNextChunk());

  textureCoordinates.push_back({ u, 1.f - v });
}

/**
 * Attempts to parse the primary vertex index, texture coordinate
 * index, and normal index of a polygonal face. A data chunk can
 * be structured in any of the following ways:
 *
 *   v
 *   v/vt
 *   v/vt/vn
 *   v//vn
 *
 * Where v is the primary index, vt the texture coordinate index,
 * and vn the normal index, with respect to previously listed
 * vertex/texture coordinate/normal values.
 */
VertexData ObjLoader::parseVertexData(const std::string& chunk) {
  VertexData vertexData;
  int offset = 0;
  int indexes[3];

  for (int i = 0; i < 3; i++) {
    int next = chunk.find("/", offset);
    bool hasNext = next > -1;

    if (next - offset == 0 || offset >= chunk.length()) {
      // If the next '/' is immediately after the last,
      // or we've reached the end of the chunk with
      // cycles to spare, this type of vertex index isn't
      // defined.
      indexes[i] = -1;
    } else {
      // As long as characters are found in between the
      // previous '/' and the next, or we still have extra
      // characters in the chunk, attempt to parse the index.
      int len = hasNext ? next : std::string::npos;

      indexes[i] = stoi(chunk.substr(offset, len)) - 1;
    }

    offset = hasNext ? next + 1 : chunk.length();
  }

  vertexData.vertexIndex = indexes[0];
  vertexData.textureCoordinateIndex = indexes[1];
  vertexData.normalIndex = indexes[2];

  return vertexData;
}

/**
 * ----------
 * GltfLoader
 * ----------
 */
GltfLoader::GltfLoader(const char* path) {
  load(path);

  while (isLoading) {
    setChunkDelimiter("\n");

    auto& line = readNextChunk();

    if (line == "    \"nodes\" : [") {
      parseNodes();
    }
  }
}

GltfLoader::~GltfLoader() {

}

void GltfLoader::parseNodes() {
  std::string node_json;
  int32 node_index = 0;

  while (isLoading) {
    auto& line = readNextChunk();

    node_json += line;
    node_json += "\n";

    if (line.starts_with("    \"")) {
      // If we reach another root property beside "nodes", terminate here
      break;
    }

    if (line.ends_with("},")) {
      // auto name = readStringProperty(node_json, "name");

      tBone bone;
      bone.index = node_index;

      // Parse rotation
      {
        auto rotation = readArrayProperty(node_json, "rotation");
        auto rotation_values = parseFloatArray(rotation);

        if (rotation_values.size() == 4) {
          bone.rotation.x = rotation_values[0];
          bone.rotation.y = rotation_values[1];
          bone.rotation.z = rotation_values[2];
          bone.rotation.w = rotation_values[3];
        } else {
          bone.rotation = Quaternion(1.f, 0, 0, 0);
        }
      }

      // Parse scale
      {
        auto scale = readArrayProperty(node_json, "scale");
        auto scale_values = parseFloatArray(scale);

        if (scale_values.size() == 3) {
          bone.scale.x = scale_values[0];
          bone.scale.y = scale_values[1];
          bone.scale.z = scale_values[2];
        }
      }

      // Parse translation
      {
        auto translation = readArrayProperty(node_json, "translation");
        auto translation_values = parseFloatArray(translation);

        if (translation_values.size() == 3) {
          bone.translation.x = translation_values[0];
          bone.translation.y = translation_values[1];
          bone.translation.z = translation_values[2];
        }
      }

      // Parse child bones by index
      {
        auto children = readArrayProperty(node_json, "children");

        bone.child_bone_indexes = parseIntArray(children);
      }

      // Add bone to skeleton
      skeleton.bones.push_back(bone);

      // Allow us to proceed to the next bone
      node_json.clear();
      node_index++;
    }
  }

  // Define child -> parent bone relationships
  for (auto& bone : skeleton.bones) {
    for (auto index : bone.child_bone_indexes) {
      auto& child_bone = skeleton.bones[index];

      child_bone.parent_bone_index = bone.index;
    }
  }
}

// @todo refactor
// @todo @optimize
std::vector<float> GltfLoader::parseFloatArray(const std::string& array_string) {
  std::vector<float> values;
  int32 current_index = 1;
  bool ended = false;

  if (array_string == "[]") {
    return values;
  }

  while (!ended) {
    auto next_index = array_string.find(",", current_index);

    if (next_index == std::string::npos) {
      auto final_index = array_string.find("]", current_index);
      auto length = final_index - current_index;
      auto value_string = array_string.substr(current_index, length);
      float value = stof(value_string);

      values.push_back(value);

      ended = true;
    } else {
      auto length = next_index - current_index;
      auto value_string = array_string.substr(current_index, length);
      float value = stof(value_string);

      values.push_back(value);
    }

    current_index = next_index + 1;
  }

  return values;
}

// @todo refactor
// @todo @optimize
std::vector<int32> GltfLoader::parseIntArray(const std::string& array_string) {
  std::vector<int32> values;
  int32 current_index = 1;
  bool ended = false;

  if (array_string == "[]") {
    return values;
  }

  while (!ended) {
    auto next_index = array_string.find(",", current_index);

    if (next_index == std::string::npos) {
      auto final_index = array_string.find("]", current_index);
      auto length = final_index - current_index;
      auto value_string = array_string.substr(current_index, length);
      int32 value = stoi(value_string);

      values.push_back(value);

      ended = true;
    } else {
      auto length = next_index - current_index;
      auto value_string = array_string.substr(current_index, length);
      int32 value = stoi(value_string);

      values.push_back(value);
    }

    current_index = next_index + 1;
  }

  return values;
}

std::string GltfLoader::readArrayProperty(const std::string& json_string, const std::string& property_name) {
  std::string property_term = "\"" + property_name + "\"";
  auto index = json_string.find(property_term);

  if (index == std::string::npos) {
    return "[]";
  } else {
    auto start_index = json_string.find("[", index);
    auto end_index = json_string.find("]", start_index);
    auto length = (end_index - start_index) + 1;

    return json_string.substr(start_index, length);
  }
}

std::string GltfLoader::readStringProperty(const std::string& json_string, const std::string& property_name) {
  std::string property_term = "\"" + property_name + "\"";
  auto index = json_string.find(property_term);

  if (index == std::string::npos) {
    return "-";
  } else {
    auto start_index = json_string.find(" : ", index) + 3;
    auto end_index = json_string.find(",\n", index);
    auto length = end_index - start_index;

    return json_string.substr(start_index, length);
  }
}

/**
 * ----------
 * SkinLoader
 * ----------
 */
SkinLoader::SkinLoader(const char* path) {
  load(path);

  if (!isLoading) {
    printf("\033[33m" "[SkinLoader] Mesh skin failed to load: %s\n" "\033[0m", path);
  }

  while (isLoading) {
    setChunkDelimiter(" ");

    std::string chunk = readNextChunk();

    if (chunk.starts_with("V")) {
      // Parse bone indexes
      uint32 b1 = stoi(readNextChunk());
      uint32 b2 = stoi(readNextChunk());
      uint32 b3 = stoi(readNextChunk());
      uint32 b4 = stoi(readNextChunk());

      uint32 indexes = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;

      // Parse bone weights
      tVec4f weights;
      weights.x = stof(readNextChunk());
      weights.y = stof(readNextChunk());
      weights.z = stof(readNextChunk());
      weights.w = stof(readNextChunk());

      bone_indexes_packed.push_back(indexes);
      bone_weights.push_back(weights);
    }
  }
}

SkinLoader::~SkinLoader() {

}