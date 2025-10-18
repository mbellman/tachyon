#version 460 core

uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;
uniform bool is_grass;
uniform float scene_time;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelSurface;
layout (location = 5) in mat4 modelMatrix;

flat out uvec4 fragSurface;
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

uvec4 SurfaceToUVec4(uint surface) {
  uint rg = ((surface & 0xFF000000) >> 24);
  uint ba = ((surface & 0x00FF0000) >> 16);
  uint roughness_metalness = ((surface & 0x0000FF00) >> 8);
  uint clearcoat_subsurface = surface & 0x000000FF;

  return uvec4(rg, ba, roughness_metalness, clearcoat_subsurface);
}

float GetFoliageLocalWind(mat4 model_matrix, float wind_speed, float wind_strength) {
  // Have wind "flow" across the environment along x,
  // using model_x as a base for determining periodic strength
  float model_x = model_matrix[3][0];

  return wind_strength * (0.7 + 0.3 * sin(wind_speed * scene_time - model_matrix[3][0] * 0.0005));
}

float GetFoliageDriftIntensity(float wind, float vertex_y) {
  return wind * min(1.0, vertex_y * vertex_y);
}

void main() {
  mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));

  // For the vertex transform, start by just applying rotation + scale
  vec3 model_space_position = mat3(modelMatrix) * vertexPosition;

  // Then apply translation, offset by the transform origin
  vec3 translation = vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  // @todo separate foliage shader?
  // @todo handle foliage behavior in shadow map pass
  if (is_grass) {
    float model_y = model_space_position.y;
    float vertex_y = vertexPosition.y;

    // Calculate wind
    float wind_strength = 200.0;
    float wind_speed = 2.0;
    float local_wind = GetFoliageLocalWind(modelMatrix, wind_speed, wind_strength);

    // Calculate the local drift intensity
    float drift_factor = GetFoliageDriftIntensity(local_wind, vertex_y);

    float alpha = 2.0 * scene_time + modelMatrix[3][0];

    world_space_position.x += drift_factor * sin(alpha);
    world_space_position.z += drift_factor * cos(1.5 * alpha);
  }

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragSurface = SurfaceToUVec4(modelSurface);
  fragNormal = normal_matrix * vertexNormal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);
  fragUv = vertexUv;
}