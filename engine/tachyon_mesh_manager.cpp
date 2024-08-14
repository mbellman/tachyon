#include <map>

#include "engine/tachyon_loaders.h"
#include "engine/tachyon_mesh_manager.h"

tMesh Tachyon_LoadMesh(const char* path) {
  tMesh mesh;

  ObjLoader obj(path);

  // @todo move the below into its own function
  auto& vertices = mesh.vertices;
  auto& faceElements = mesh.face_elements;

  if (obj.textureCoordinates.size() == 0 && obj.normals.size() == 0) {
    // Only vertex positions defined, so simply load in vertices,
    // and then load in face element indexes
    for (const auto& position : obj.vertices) {
      tVertex vertex;

      vertex.position = position;

      vertices.push_back(vertex);
    }

    for (const auto& face : obj.faces) {
      faceElements.push_back(face.v1.vertexIndex);
      faceElements.push_back(face.v2.vertexIndex);
      faceElements.push_back(face.v3.vertexIndex);      
    }
  } else {
    // Texture coordinates and/or normals defined, so we need
    // to create a unique vertex for each position/uv/normal
    // tuple, and add face elements based on created vertices
    typedef std::tuple<uint32, uint32, uint32> VertexTuple;

    std::map<VertexTuple, uint32> vertexTupleToIndexMap;

    for (const auto& face : obj.faces) {
      VertexTuple vertexTuples[3] = {
        { face.v1.vertexIndex, face.v1.textureCoordinateIndex, face.v1.normalIndex },
        { face.v2.vertexIndex, face.v2.textureCoordinateIndex, face.v2.normalIndex },
        { face.v3.vertexIndex, face.v3.textureCoordinateIndex, face.v3.normalIndex }
      };

      // Add face elements, creating vertices if necessary
      for (uint32 p = 0; p < 3; p++) {
        auto& vertexTuple = vertexTuples[p];
        auto indexRecord = vertexTupleToIndexMap.find(vertexTuple);

        if (indexRecord != vertexTupleToIndexMap.end()) {
          // Vertex tuple already exists, so we can just
          // add the stored face element index
          faceElements.push_back(indexRecord->second);
        } else {
          // Vertex doesn't exist, so we need to create it
          tVertex vertex;
          uint32 index = vertices.size();

          // @todo Have the option to run through existing vertices,
          // compare position, and re-use any which are within a minuscule
          // distance threshold of this one to avoid duplicates. Certain
          // obj files may do this on purpose, so it should be opt-in.
          vertex.position = obj.vertices[std::get<0>(vertexTuple)];

          if (obj.textureCoordinates.size() > 0) {
            vertex.uv = obj.textureCoordinates[std::get<1>(vertexTuple)];
          }

          if (obj.normals.size() > 0) {
            vertex.normal = obj.normals[std::get<2>(vertexTuple)];
          }

          vertices.push_back(vertex);
          faceElements.push_back(index);

          vertexTupleToIndexMap.emplace(vertexTuple, index);
        }
      }
    }
  }

  return mesh;
}

uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh) {
  auto& pack = tachyon->mesh_pack;
  tMeshRecord record;

  // Register the mesh vertex/face element bounds
  record.vertex_start = pack.vertex_stream.size();
  record.vertex_end = record.vertex_start + mesh.vertices.size();

  record.face_element_start = pack.face_element_stream.size();
  record.face_element_end = record.face_element_start + mesh.face_elements.size();

  // @todo use # of objects in the mesh
  record.group.total = 1;

  pack.mesh_records.push_back(record);

  // Add vertices/face elements to the main stream
  for (auto& vertex : mesh.vertices) {
    pack.vertex_stream.push_back(vertex);
  }

  for (auto& element : mesh.face_elements) {
    pack.face_element_stream.push_back(element);
  }

  return pack.mesh_records.size();
}

void Tachyon_InitializeObjects(Tachyon* tachyon) {
  uint32 totalObjects = 0;

  for (auto& record : tachyon->mesh_pack.mesh_records) {
    totalObjects += record.group.total;
  }

  tachyon->objects.reserve(totalObjects);
  tachyon->matrices.reserve(totalObjects);
  tachyon->colors.reserve(totalObjects);

  uint32 offset = 0;

  for (auto& record : tachyon->mesh_pack.mesh_records) {
    record.group.objects = &tachyon->objects[offset];

    offset += record.group.total;
  }
}