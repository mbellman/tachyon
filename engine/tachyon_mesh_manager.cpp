#include "engine/tachyon_mesh_manager.h"

tMesh Tachyon_LoadMesh(const char* path) {
  tMesh mesh;

  // @todo load model file and add vertices

  return mesh;
}

void Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh) {
  auto& pack = tachyon->mesh_pack;
  tMeshRecord record;

  // Register the mesh vertex/face element bounds
  record.vertex_start = pack.vertex_stream.size();
  record.vertex_end = record.vertex_start + mesh.vertices.size();

  record.face_element_start = pack.face_element_stream.size();
  record.face_element_end = record.face_element_start + mesh.face_elements.size();

  pack.mesh_records.push_back(record);

  // Add vertices/face elements to the main stream
  for (auto& vertex : mesh.vertices) {
    pack.vertex_stream.push_back(vertex);
  }

  for (auto& element : mesh.face_elements) {
    pack.face_element_stream.push_back(element);
  }
}