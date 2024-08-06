#include "engine/tachyon_mesh_manager.h"

tMesh Tachyon_LoadMesh(const char* path) {
  tMesh mesh;

  // @todo load model file and add vertices

  return mesh;
}

void Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh) {
  auto& pack = tachyon->mesh_pack;
  tMeshRecord record;

  // Register the mesh vertex bounds
  record.start = pack.vertex_stream.size();
  record.end = record.start + mesh.vertices.size();

  pack.mesh_records.push_back(record);

  // Add vertices to the main stream
  for (auto& v : mesh.vertices) {
    pack.vertex_stream.push_back(v);
  }
}