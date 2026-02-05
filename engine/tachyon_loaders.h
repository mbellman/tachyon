#pragma once

#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

class AbstractLoader {
public:
  virtual ~AbstractLoader() {};

protected:
  bool isLoading = false;

  void load(const char* filePath);
  const std::string& readNextChunk();
  void nextLine();
  void setChunkDelimiter(const std::string& delimiter);

private:
  std::string buffer = "";
  std::string delimiter = " ";
  FILE* file = 0;

  bool bufferEndsWith(const std::string& str);
  void fillBufferUntil(const std::string& end);
  int getDelimiterOffset();
  bool isAtDelimiter();
  bool isAtEOL();
  int nextChar();
};

/**
 * VertexData
 * ----------
 *
 * A small data container for a particular vertex, representing its
 * primary vertex index, texture coordinate index, and vertex normal
 * index among the lists defined in an .obj file.
 */
struct VertexData {
  uint32 vertexIndex;
  uint32 textureCoordinateIndex;
  uint32 normalIndex;
};

/**
 * Face
 * ----
 *
 * Contains vertex data for polygonal faces.
 */
struct Face {
  VertexData v1;
  VertexData v2;
  VertexData v3;
};

/**
 * ObjLoader
 * ---------
 *
 * Opens and parses .obj files into an intermediate representation
 * for conversion into Model instances.
 *
 * Usage:
 *
 *  ObjLoader modelObj("path/to/file.obj");
 */
class ObjLoader : public AbstractLoader {
public:
  std::vector<tVec3f> vertices;
  std::vector<tVec2f> textureCoordinates;
  std::vector<tVec3f> normals;
  std::vector<Face> faces;

  ObjLoader(const char* path);
  ~ObjLoader();

private:
  void handleFace();
  void handleNormal();
  void handleVertex();
  void handleTextureCoordinate();
  VertexData parseVertexData(const std::string& data);
};

/**
 * GltfLoader
 * ----------
 *
 * Opens and parses .gltf files into an intermediate representation
 * for conversion into Model instances.
 *
 * Usage:
 *
 *  GltfLoader modelGltf("path/to/file.gltf");
 */
class GltfLoader : public AbstractLoader {
public:
  GltfLoader(const char* path);
  ~GltfLoader();

private:
  void parseNodes();
  std::string readArrayProperty(const std::string& json_string, const std::string& property_name);
  std::string readStringProperty(const std::string& json_string, const std::string& property_name);
};