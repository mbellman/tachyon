#version 460 core

uniform mat4 light_matrix;
uniform vec3 transform_origin;
uniform mat4 model_matrix;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint bone_indexes_packed;
layout (location = 5) in vec4 bone_weights;

uniform mat4 bones[32];

void main() {
  uint bone_1_index = (bone_indexes_packed & 0xFF000000) >> 24;
  uint bone_2_index = (bone_indexes_packed & 0x00FF0000) >> 16;
  uint bone_3_index = (bone_indexes_packed & 0x0000FF00) >> 8;
  uint bone_4_index = (bone_indexes_packed & 0x000000FF);

  mat4 bone_1 = bones[bone_1_index];
  mat4 bone_2 = bones[bone_2_index];
  mat4 bone_3 = bones[bone_3_index];
  mat4 bone_4 = bones[bone_4_index];

  // Apply bone transforms
  vec3 position = vec3(0.0);

  position += (bone_weights.x * (bone_1 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.y * (bone_2 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.z * (bone_3 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.w * (bone_4 * vec4(vertexPosition, 1.0))).xyz;

  // For the vertex transform, start by just applying rotation.
  // Translation should be offset by the transform origin.
  vec3 model_space_position = mat3(model_matrix) * position;
  vec3 translation = vec3(model_matrix[3][0], model_matrix[3][1], model_matrix[3][2]);

  // Apply translation, offset by the origin
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  gl_Position = light_matrix * vec4(world_space_position, 1.0);
}