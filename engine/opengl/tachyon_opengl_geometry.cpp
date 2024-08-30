#include <glew.h>
#include <SDL_opengl.h>

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