#include <glew.h>
#include <SDL_opengl.h>

#include "engine/tachyon_constants.h"
#include "engine/opengl/tachyon_opengl_geometry.h"

tOpenGLMeshPack Tachyon_CreateOpenGLMeshPack(Tachyon* tachyon) {
  #define VERTEX_POSITION 0
  #define VERTEX_NORMAL 1
  #define VERTEX_TANGENT 2
  #define VERTEX_UV 3
  #define MODEL_MATRIX 5
  #define MODEL_SURFACE 4

  auto& pack = tachyon->mesh_pack;
  auto& vertices = pack.vertex_stream;
  auto& faceElements = pack.face_element_stream;
  tOpenGLMeshPack glPack;

  glGenVertexArrays(1, &glPack.vao);
  glGenBuffers(3, &glPack.buffers[0]);
  glGenBuffers(1, &glPack.ebo);

  glBindVertexArray(glPack.vao);

  // Buffer vertex data
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[VERTEX_BUFFER]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(tVertex), vertices.data(), GL_STATIC_DRAW);

  // Buffer vertex element data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glPack.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceElements.size() * sizeof(uint32), faceElements.data(), GL_STATIC_DRAW);

  // Define vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[VERTEX_BUFFER]);

  glEnableVertexAttribArray(VERTEX_POSITION);
  glVertexAttribPointer(VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, position));

  glEnableVertexAttribArray(VERTEX_NORMAL);
  glVertexAttribPointer(VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, normal));

  glEnableVertexAttribArray(VERTEX_TANGENT);
  glVertexAttribPointer(VERTEX_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, tangent));

  glEnableVertexAttribArray(VERTEX_UV);
  glVertexAttribPointer(VERTEX_UV, 2, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, uv));

  // Define surface attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[SURFACE_BUFFER]);
  glEnableVertexAttribArray(MODEL_SURFACE);
  glVertexAttribIPointer(MODEL_SURFACE, 1, GL_UNSIGNED_INT, sizeof(uint32), (void*)0);
  glVertexAttribDivisor(MODEL_SURFACE, 1);

  // Define matrix attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[MATRIX_BUFFER]);

  for (uint32 i = 0; i < 4; i++) {
    glEnableVertexAttribArray(MODEL_MATRIX + i);
    glVertexAttribPointer(MODEL_MATRIX + i, 4, GL_FLOAT, GL_FALSE, sizeof(tMat4f), (void*)(i * 4 * sizeof(float)));
    glVertexAttribDivisor(MODEL_MATRIX + i, 1);
  }

  return glPack;
}

tOpenGLScreenQuad Tachyon_CreateOpenGLScreenQuad(Tachyon* tachyon) {
  #define QUAD_POSITION 0
  #define QUAD_UV 1

  const static float SCREEN_QUAD_DATA[] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f
  };

  tOpenGLScreenQuad quad;

  glGenVertexArrays(1, &quad.vao);
  glGenBuffers(1, &quad.vbo);

  glBindVertexArray(quad.vao);

  // Buffer quad data
  glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), SCREEN_QUAD_DATA, GL_STATIC_DRAW);

  // Define vertex attributes
  glEnableVertexAttribArray(QUAD_POSITION);
  glVertexAttribPointer(QUAD_POSITION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(QUAD_UV);
  glVertexAttribPointer(QUAD_UV, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  return quad;
}

tOpenGLLightDisc Tachyon_CreateOpenGLPointLightDisc(Tachyon* tachyon) {
  constexpr static uint32 DISC_SLICES = 16;
  constexpr static float slice_angle = 360.f / (float)DISC_SLICES;
  constexpr static float DEGREES_TO_RADIANS = t_PI / 180.f;

  tOpenGLLightDisc disc;

  glGenVertexArrays(1, &disc.vao);
  glGenBuffers(1, &disc.vertex_buffer);
  glGenBuffers(1, &disc.light_buffer);
  glBindVertexArray(disc.vao);

  tVec2f vertex_positions[DISC_SLICES * 3];

  for (uint32 i = 0; i < DISC_SLICES; i++) {
    uint32 index = i * 3;

    // Add center vertex
    vertex_positions[index] = tVec2f(0.f);

    // Add corners
    const float a1 = float(i) * slice_angle * DEGREES_TO_RADIANS;
    const float a2 = float(i + 1) * slice_angle * DEGREES_TO_RADIANS;

    vertex_positions[index + 1] = tVec2f(sinf(a1), cosf(a1));

    if (i == DISC_SLICES - 1) {
      // @hack Ensure the final disc slice does not create any seam gaps
      // with the first slice. We actually get a subtle floating point
      // error accumulation by the time we've gone around the circle,
      // and manually-calculated corner positions are just slightly off.
      vertex_positions[index + 2].x = vertex_positions[1].x;
      vertex_positions[index + 2].y = vertex_positions[1].y;
    } else {
      vertex_positions[index + 2] = tVec2f(sinf(a2), cosf(a2));
    }
  }

  // Buffer disc vertices
  glBindBuffer(GL_ARRAY_BUFFER, disc.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tVec2f) * DISC_SLICES * 3, vertex_positions, GL_STATIC_DRAW);

  // Define disc vertex attributes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(tVec2f), (void*)0);

  // Define disc instance attributes
  glBindBuffer(GL_ARRAY_BUFFER, disc.light_buffer);

  typedef tOpenGLPointLightDiscInstance Disc;
  typedef tPointLight Light;

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)offsetof(Disc, offset));
  glVertexAttribDivisor(1, 1);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)offsetof(Disc, scale));
  glVertexAttribDivisor(2, 1);

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, position)));
  glVertexAttribDivisor(3, 1);

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, radius)));
  glVertexAttribDivisor(4, 1);

  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, color)));
  glVertexAttribDivisor(5, 1);

  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, power)));
  glVertexAttribDivisor(6, 1);

  glEnableVertexAttribArray(7);
  glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, glow_power)));
  glVertexAttribDivisor(7, 1);

  return disc;
}