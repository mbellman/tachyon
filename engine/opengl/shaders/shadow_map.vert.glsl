#version 460 core

// uniform sampler2D meshTexture;
uniform mat4 light_matrix;
uniform vec3 transform_origin;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelSurface;
layout (location = 5) in mat4 modelMatrix;

// out vec2 fragUv;

void main() {
  // For the vertex transform, start by just applying rotation.
  // Translation should be offset by the transform origin.
  vec3 model_space_position = mat3(modelMatrix) * vertexPosition;
  vec3 translation = vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

  // Apply translation, offset by the origin
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  gl_Position = light_matrix * vec4(world_space_position, 1.0);
  // fragUv = vertexUv;
}