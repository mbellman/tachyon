#include "engine/tachyon_console.h"
#include "engine/tachyon_loaders.h"

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

// ---------
// ObjLoader
// ---------
static std::string VERTEX_LABEL = "v";
static std::string TEXTURE_COORDINATE_LABEL = "vt";
static std::string NORMAL_LABEL = "vn";
static std::string FACE_LABEL = "f";

ObjLoader::ObjLoader(const char* path) {
  load(path);

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

// ----------
// GltfLoader
// ----------
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

    if (line.ends_with("},")) {
      auto children = readArrayProperty(node_json, "children");
      auto name = readStringProperty(node_json, "name");
      auto rotation = readArrayProperty(node_json, "rotation");
      auto scale = readArrayProperty(node_json, "scale");
      auto translation = readArrayProperty(node_json, "translation");

      // @temporary
      printf("Bone (%d):\n", node_index);
      // printf("Children: %s\n", children.c_str());
      printf("Name: %s\n", name.c_str());
      printf("Rotation: %s\n", rotation.c_str());
      printf("Scale: %s\n", scale.c_str());
      printf("Translation: %s\n", translation.c_str());

      auto rotation_values = parseFloatArray(rotation);
      auto scale_values = parseFloatArray(scale);
      auto translation_values = parseFloatArray(translation);

      Quaternion r;

      if (rotation_values.size() == 4) {
        r.x = rotation_values[0];
        r.y = rotation_values[1];
        r.z = rotation_values[2];
        r.w = rotation_values[3];
      }

      tVec3f s;

      if (scale_values.size() == 3) {
        s.x = scale_values[0];
        s.y = scale_values[1];
        s.z = scale_values[2];
      }

      tVec3f t;

      if (translation_values.size() == 3) {
        t.x = translation_values[0];
        t.y = translation_values[1];
        t.z = translation_values[2];
      }

      // console_log(r);
      // console_log(s);
      // console_log(t);

      printf("\n\n");

      node_json.clear();
      node_index++;
    }
  }
}

// @optimize
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

std::vector<int32> GltfLoader::parseIntArray(const std::string& array_string) {
  // @todo
  return {};
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