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
  std::string bone_data;

  while (isLoading) {
    auto& line = readNextChunk();

    bone_data += line;
    bone_data += "\n";

    if (line.ends_with("},")) {
      auto children = readArrayProperty(bone_data, "children");
      auto name = readStringProperty(bone_data, "name");
      auto rotation = readArrayProperty(bone_data, "rotation");
      auto scale = readArrayProperty(bone_data, "scale");
      auto translation = readArrayProperty(bone_data, "translation");

      // @temporary
      printf("Bone:\n\n");
      printf("Children: %s\n", children.c_str());
      printf("Name: %s\n", name.c_str());
      printf("Rotation: %s\n", rotation.c_str());
      printf("Scale: %s\n", scale.c_str());
      printf("Translation: %s\n", translation.c_str());
      printf("\n\n");

      bone_data.clear();
    }
  }
}

std::string GltfLoader::readArrayProperty(const std::string& bone_data, const std::string& property_name) {
  std::string property_term = "\"" + property_name + "\"";
  auto index = bone_data.find(property_term);

  if (index == std::string::npos) {
    return "[]";
  } else {
    // @todo
    return "Array!";
  }
}

std::string GltfLoader::readStringProperty(const std::string& bone_data, const std::string& property_name) {
  std::string property_term = "\"" + property_name + "\"";
  auto index = bone_data.find(property_term);

  if (index == std::string::npos) {
    return "-";
  } else {
    // @todo
    return "String!";
  }
}