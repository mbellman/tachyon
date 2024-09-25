#include <map>
#include <math.h>

#include "engine/tachyon_constants.h"
#include "engine/tachyon_loaders.h"
#include "engine/tachyon_mesh_manager.h"

static inline float EaseInOut(float t) {
  return -(cosf(t_PI * t) - 1.f) / 2.f;
}

static void ComputeNormals(tMesh& mesh) {
  auto& vertices = mesh.vertices;
  auto& face_elements = mesh.face_elements;

  for (auto& vertex : vertices) {
    vertex.normal = tVec3f(0.f);
  }

  for (uint32 i = 0; i < face_elements.size(); i += 3) {
    tVertex& v1 = vertices[face_elements[i]];
    tVertex& v2 = vertices[face_elements[i + 1]];
    tVertex& v3 = vertices[face_elements[i + 2]];

    tVec3f normal = tVec3f::cross(v2.position - v1.position, v3.position - v1.position).unit();

    v1.normal += normal;
    v2.normal += normal;
    v3.normal += normal;
  }

  for (auto& vertex : vertices) {
    vertex.normal = vertex.normal.unit();
  }
}

static void ComputeTangents(tMesh& mesh) {
  auto& vertices = mesh.vertices;
  auto& face_elements = mesh.face_elements;

  for (uint32 i = 0; i < face_elements.size(); i += 3) {
    tVertex& v1 = vertices[face_elements[i]];
    tVertex& v2 = vertices[face_elements[i + 1]];
    tVertex& v3 = vertices[face_elements[i + 2]];

    tVec3f e1 = v2.position - v1.position;
    tVec3f e2 = v3.position - v1.position;

    float delta_u1 = v2.uv.x - v1.uv.x;
    float delta_v1 = v2.uv.y - v1.uv.y;
    float delta_u2 = v3.uv.x - v1.uv.x;
    float delta_v2 = v3.uv.y - v1.uv.y;

    float d = (delta_u1 * delta_v2 - delta_u2 * delta_v1);

    // Prevent division by zero when vertices are in identical positions,
    // and there is no delta between uv coordinates
    float f = 1.0f / (d == 0.f ? 0.001f : d);

    tVec3f tangent = {
      f * (delta_v2 * e1.x - delta_v1 * e2.x),
      f * (delta_v2 * e1.y - delta_v1 * e2.y),
      f * (delta_v2 * e1.z - delta_v1 * e2.z)
    };

    v1.tangent += tangent;
    v2.tangent += tangent;
    v3.tangent += tangent;
  }

  for (auto& vertex : vertices) {
    vertex.tangent = vertex.tangent.unit();
  }
}

// @todo compute vertex normals + tangents when not defined in the obj file
tMesh Tachyon_LoadMesh(const char* path, const tVec3f& axis_factors) {
  tMesh mesh;

  ObjLoader obj(path);

  // @todo move the below into its own function
  auto& vertices = mesh.vertices;
  auto& face_elements = mesh.face_elements;

  if (obj.textureCoordinates.size() == 0 && obj.normals.size() == 0) {
    // Only vertex positions defined, so simply load in vertices,
    // and then load in face element indexes
    for (const auto& position : obj.vertices) {
      tVertex vertex;

      vertex.position = position;
      vertex.position *= axis_factors;

      vertices.push_back(vertex);
    }

    for (const auto& face : obj.faces) {
      face_elements.push_back(face.v1.vertexIndex);
      face_elements.push_back(face.v2.vertexIndex);
      face_elements.push_back(face.v3.vertexIndex);
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
          face_elements.push_back(indexRecord->second);
        } else {
          // Vertex doesn't exist, so we need to create it
          tVertex vertex;
          uint32 index = vertices.size();

          // @todo Have the option to run through existing vertices,
          // compare position, and re-use any which are within a minuscule
          // distance threshold of this one to avoid duplicates. Certain
          // obj files may do this on purpose, so it should be opt-in.
          vertex.position = obj.vertices[std::get<0>(vertexTuple)];
          vertex.position *= axis_factors;

          if (obj.textureCoordinates.size() > 0) {
            vertex.uv = obj.textureCoordinates[std::get<1>(vertexTuple)];
          }

          if (obj.normals.size() > 0) {
            vertex.normal = obj.normals[std::get<2>(vertexTuple)];
            vertex.normal *= axis_factors;
          }

          vertices.push_back(vertex);
          face_elements.push_back(index);

          vertexTupleToIndexMap.emplace(vertexTuple, index);
        }
      }
    }
  }

  return mesh;
}

tMesh Tachyon_CreatePlaneMesh() {
  tMesh mesh;

  mesh.vertices.resize(4);
  mesh.face_elements.resize(6);

  mesh.vertices[0].position = tVec3f(-1.f, 0.f, 1.f);
  mesh.vertices[1].position = tVec3f(-1.f, 0.f, -1.f);
  mesh.vertices[2].position = tVec3f(1.f, 0.f, 1.f);
  mesh.vertices[3].position = tVec3f(1.f, 0.f, -1.f);

  mesh.vertices[0].normal = tVec3f(0, 1.f, 0);
  mesh.vertices[1].normal = tVec3f(0, 1.f, 0);
  mesh.vertices[2].normal = tVec3f(0, 1.f, 0);
  mesh.vertices[3].normal = tVec3f(0, 1.f, 0);

  mesh.vertices[0].tangent = tVec3f(0, 0, -1.f);
  mesh.vertices[1].tangent = tVec3f(0, 0, -1.f);
  mesh.vertices[2].tangent = tVec3f(0, 0, -1.f);
  mesh.vertices[3].tangent = tVec3f(0, 0, -1.f);

  mesh.vertices[0].uv = tVec2f(0, 0);
  mesh.vertices[1].uv = tVec2f(0, 1.f);
  mesh.vertices[2].uv = tVec2f(1.f, 0);
  mesh.vertices[3].uv = tVec2f(1.f, 1.f);

  mesh.face_elements[0] = 0;
  mesh.face_elements[1] = 1;
  mesh.face_elements[2] = 2;

  mesh.face_elements[3] = 1;
  mesh.face_elements[4] = 3;
  mesh.face_elements[5] = 2;

  return mesh;
}

tMesh Tachyon_CreateCubeMesh() {
  /**
   * Defines positions for each corner of the unit cube
   */
  const static tVec3f cube_corner_positions[8] = {
    { -1.0f, -1.0f, -1.0f },  // rear side, bottom left
    { 1.0f, -1.0f, -1.0f },   // rear side, bottom right
    { 1.0f, 1.0f, -1.0f },    // rear side, top right
    { -1.0f, 1.0f, -1.0f },   // rear side, top left
    { -1.0f, -1.0f, 1.0f },   // far side, bottom left
    { 1.0f, -1.0f, 1.0f },    // far side, bottom right
    { 1.0f, 1.0f, 1.0f },     // far side, top right
    { -1.0f, 1.0f, 1.0f }     // far side, top left
  };

  /**
   * Defines UV coordinates for each cube side
   */
  const static tVec2f cube_uvs[4] = {
    { 1.0f, 1.0f },           // bottom right
    { 0.0f, 1.0f },           // bottom left
    { 0.0f, 0.0f },           // top left
    { 1.0f, 0.0f }            // top right
  };

  /**
   * Maps the corners of each cube side to corner position indexes
   */
  const static int cube_faces[6][4] = {
    { 1, 0, 3, 2 },           // back
    { 7, 6, 2, 3 },           // top
    { 4, 5, 6, 7 },           // front
    { 0, 1, 5, 4 },           // bottom
    { 0, 4, 7, 3 },           // left
    { 5, 1, 2, 6 }            // right
  };

  tMesh mesh;

  auto& vertices = mesh.vertices;
  auto& face_elements = mesh.face_elements;

  vertices.resize(24);
  face_elements.resize(36);

  // For each cube side
  for (uint8 i = 0; i < 6; i++) {
    auto& face = cube_faces[i];
    uint32 f_offset = i * 6;
    uint32 v_offset = i * 4;

    // Define vertex indexes for the two triangle faces on each cube side
    face_elements[f_offset] = v_offset;
    face_elements[f_offset + 1] = v_offset + 1;
    face_elements[f_offset + 2] = v_offset + 2;

    face_elements[f_offset + 3] = v_offset;
    face_elements[f_offset + 4] = v_offset + 2;
    face_elements[f_offset + 5] = v_offset + 3;

    // For each corner on this side
    for (uint8 j = 0; j < 4; j++) {
      auto& vertex = vertices[v_offset++];

      // Define the corner vertex position/uvs
      vertex.position = cube_corner_positions[face[j]];
      vertex.uv = cube_uvs[j];
    }
  }

  ComputeNormals(mesh);
  ComputeTangents(mesh);

  return mesh;
}

tMesh Tachyon_CreateSphereMesh(uint8 divisions) {
  tMesh mesh;

  auto& vertices = mesh.vertices;
  auto& face_elements = mesh.face_elements;
  uint8 horizontal_divisions = uint8(float(divisions) * 1.5f);

  // Pole vertices
  tVertex pole1, pole2;

  // Top pole vertex
  pole1.position = tVec3f(0, 1.f, 0);

  vertices.push_back(pole1);

  // Surface vertices
  for (uint8 i = 1; i < divisions - 1; i++) {
    for (uint8 j = 0; j < horizontal_divisions; j++) {
      float y_progress = float(i) / float(divisions - 1);
      float radius = sinf(y_progress * t_PI);
      float x = radius * cosf(float(j) / float(horizontal_divisions) * t_TAU);
      float y = 1.f - 2.f * EaseInOut(y_progress);
      float z = radius * sinf(float(j) / float(horizontal_divisions) * t_TAU);

      tVertex vertex;

      vertex.position = tVec3f(x, y, z);

      vertices.push_back(vertex);
    }
  }

  // Bottom pole vertex
  pole2.position = tVec3f(0, -1.f, 0);

  vertices.push_back(pole2);

  // Top cap faces
  for (uint8 i = 0; i < horizontal_divisions; i++) {
    uint32 f1 = 0;
    uint32 f2 = (i + 1) % horizontal_divisions + 1;
    uint32 f3 = i + 1;

    face_elements.push_back(f1);
    face_elements.push_back(f2);
    face_elements.push_back(f3);
  }

  // Mid-section faces
  for (uint8 i = 1; i < divisions - 2; i++) {
    uint32 v_start = 1 + (i - 1) * horizontal_divisions;
    uint32 v_end = v_start + horizontal_divisions;

    for (uint8 j = 0; j < horizontal_divisions; j++) {
      uint32 v_offset = v_start + j;

      uint32 f1 = v_offset;
      uint32 f2 = v_offset + 1;
      uint32 f3 = v_offset + horizontal_divisions;

      // Ensure that the second vertex index stays on
      // the same horizontal 'line' of the sphere
      if (f2 >= v_end) f2 -= horizontal_divisions;

      uint32 f4 = f2;
      uint32 f5 = f2 + horizontal_divisions;
      uint32 f6 = f1 + horizontal_divisions;

      face_elements.push_back(f1);
      face_elements.push_back(f2);
      face_elements.push_back(f3);

      face_elements.push_back(f4);
      face_elements.push_back(f5);
      face_elements.push_back(f6);
    }
  }

  // Bottom cap faces
  uint32 last_vertex_index = vertices.size() - 1;

  for (uint8 i = 0; i < horizontal_divisions; i++) {
    uint32 f1 = last_vertex_index;
    uint32 f2 = last_vertex_index - (i + 1) % horizontal_divisions - 1;
    uint32 f3 = last_vertex_index - (i + 1);

    face_elements.push_back(f1);
    face_elements.push_back(f2);
    face_elements.push_back(f3);
  }

  ComputeNormals(mesh);
  ComputeTangents(mesh);

  return mesh;
}

uint32 Tachyon_AddMesh(Tachyon* tachyon, const tMesh& mesh, uint16 total) {
  auto& pack = tachyon->mesh_pack;
  tMeshRecord record;

  // Register the mesh vertex/face element bounds
  record.vertex_start = pack.vertex_stream.size();
  record.vertex_end = record.vertex_start + mesh.vertices.size();

  record.face_element_start = pack.face_element_stream.size();
  record.face_element_end = record.face_element_start + mesh.face_elements.size();

  record.group.total = total;
  record.group.id_to_index = new uint16[total];

  pack.mesh_records.push_back(record);

  // Add vertices/face elements to the main stream
  for (auto& vertex : mesh.vertices) {
    pack.vertex_stream.push_back(vertex);
  }

  for (auto& element : mesh.face_elements) {
    pack.face_element_stream.push_back(element);
  }

  return pack.mesh_records.size() - 1;
}

void Tachyon_InitializeObjects(Tachyon* tachyon) {
  uint32 total_objects = 0;

  for (auto& record : tachyon->mesh_pack.mesh_records) {
    total_objects += record.group.total;
  }

  tachyon->objects.resize(total_objects);
  tachyon->surfaces.resize(total_objects);
  tachyon->matrices.resize(total_objects);

  uint16 mesh_index = 0;
  uint32 object_offset = 0;

  for (auto& record : tachyon->mesh_pack.mesh_records) {
    // Set object group pointers into global arrays
    record.group.objects = &tachyon->objects[object_offset];
    record.group.surfaces = &tachyon->surfaces[object_offset];
    record.group.matrices = &tachyon->matrices[object_offset];
    record.group.object_offset = object_offset;

    // Set mesh/object indexes on each object
    for (uint16 i = 0; i < record.group.total; i++) {
      auto& object = record.group[i];

      object.mesh_index = mesh_index;
      object.object_id = i;
    }

    object_offset += record.group.total;
    mesh_index++;
  }
}

tObject& Tachyon_CreateObject(Tachyon* tachyon, uint16 mesh_index) {
  auto& group = tachyon->mesh_pack.mesh_records[mesh_index].group;

  if (group.total_visible >= group.total) {
    // @todo throw an error and exit
  }

  group.total_visible++;
  group.total_active++;

  auto index = group.total_active - 1;
  auto& object = group[index];

  if (object.object_id > group.highest_used_id) {
    group.highest_used_id = object.object_id;
  }

  group.id_to_index[object.object_id] = index;
  group.buffered = false;

  return object;
}

void Tachyon_RemoveObject(Tachyon* tachyon, tObject& object) {
  auto& group = tachyon->mesh_pack.mesh_records[object.mesh_index].group;

  if (group.total_active == 0) {
    // @todo throw an error?
    return;
  }

  uint16 removed_id = object.object_id;
  uint16 removed_index = group.id_to_index[removed_id];
  uint16 last_active_index = group.total_active - 1;

  // Copy the last object in the active objects set
  tObject last_active_object = group.objects[last_active_index];
  uint32 last_active_surface = group.surfaces[last_active_index];
  tMat4f last_active_matrix = group.matrices[last_active_index];

  // Move it to the removed index
  group.objects[removed_index] = last_active_object;
  group.surfaces[removed_index] = last_active_surface;
  group.matrices[removed_index] = last_active_matrix;

  // Update its id -> index mapping
  group.id_to_index[last_active_object.object_id] = removed_index;

  // Swap the object IDs
  group.objects[last_active_index].object_id = removed_id;

  // Truncate total active objects by 1
  group.total_active--;

  if (removed_index < group.total_visible) {
    // If the removed index is within the visible objects set,
    // truncate that by 1 too
    group.total_visible--;
  }

  group.buffered = false;
}

void Tachyon_CommitObject(Tachyon* tachyon, const tObject& object) {
  auto& group = tachyon->mesh_pack.mesh_records[object.mesh_index].group;
  auto index = group.id_to_index[object.object_id];

  group.surfaces[index] = (uint32(object.color.rgba) << 16) | (uint32)object.material.data;
  group.matrices[index] = tMat4f::transformation(object.position, object.scale, object.rotation).transpose();
  group.buffered = false;
}

tObject* Tachyon_GetOriginalObject(Tachyon* tachyon, tObject& object) {
  auto& group = tachyon->mesh_pack.mesh_records[object.mesh_index].group;
  auto index = group.id_to_index[object.object_id];

  if (index > group.total_active - 1) {
    return nullptr;
  }

  return &group.objects[index];
}