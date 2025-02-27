#version 460 core

uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelSurface;
layout (location = 5) in mat4 modelMatrix;

flat out uvec4 fragSurface;
out vec3 fragPosition;
out vec3 fragNormal;

// Used in fire/ion thruster frag shaders
flat out vec3 modelPosition;
flat out vec3 basePosition;
flat out vec3 topPosition;
flat out vec3 upDirection;
out vec3 vertPosition;

uvec4 SurfaceToUVec4(uint surface) {
  uint rg = ((surface & 0xFF000000) >> 24);
  uint ba = ((surface & 0x00FF0000) >> 16);
  uint roughness_metalness = ((surface & 0x0000FF00) >> 8);
  uint clearcoat_subsurface = surface & 0x000000FF;

  return uvec4(rg, ba, roughness_metalness, clearcoat_subsurface);
}

void main() {
  mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));

  // For the vertex transform, start by just applying rotation.
  // Translation should be offset by the transform origin.
  vec3 model_space_position = mat3(modelMatrix) * vertexPosition;
  vec3 translation = vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

  // Apply translation, offset by the origin
  vec3 transform_space_position = model_space_position + (translation - transform_origin);

  gl_Position = view_projection_matrix * vec4(transform_space_position, 1.0);

  fragSurface = SurfaceToUVec4(modelSurface);
  modelPosition = translation;
  basePosition = (modelMatrix * vec4(0, -1.0, 0, 1.0)).xyz;
  topPosition = (modelMatrix * vec4(0, 1.0, 0, 1.0)).xyz;
  upDirection = mat3(modelMatrix) * vec4(0, 1.0, 0, 1.0).xyz;
  vertPosition = vertexPosition;
  fragPosition = model_space_position + translation;
  fragNormal = normal_matrix * vertexNormal;
}