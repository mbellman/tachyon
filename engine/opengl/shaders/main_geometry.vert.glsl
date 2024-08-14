#version 460 core

// uniform sampler2D meshTexture;
uniform mat4 matViewProjection;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelColor;
layout (location = 5) in mat4 modelMatrix;

flat out vec3 fragColor;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;

/**
 * Returns a bitangent from potentially non-orthonormal
 * normal/tangent vectors using the Gram-Schmidt process.
 */
vec3 getFragBitangent(vec3 normal, vec3 tangent) {
  // Redefine the tangent by using the projection of the tangent
  // onto the normal line and defining a vector from that to the
  // original tangent, orthonormalizing the normal/tangent
  tangent = normalize(tangent - dot(tangent, normal) * normal);

  return cross(tangent, normal);
}

void main() {
  vec4 world_position = modelMatrix * vec4(vertexPosition, 1.0);

  gl_Position = matViewProjection * world_position;

  fragColor = vec3(1, 0, 0); // @todo
  fragPosition = world_position.xyz;
  fragNormal = vertexNormal;
  fragTangent = vertexTangent;
  // fragNormal = normal_matrix * vertexNormal;
  // fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);

  fragUv = vertexUv;
}