#version 460 core

uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;
uniform bool is_grass;
uniform bool is_foliage;
uniform vec3 foliage_mover_position;
uniform vec3 foliage_mover_velocity;
uniform float scene_time;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelSurface;
layout (location = 5) in mat4 modelMatrix;

flat out uvec4 fragSurface;
out vec3 fragWorldPosition;
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

vec3 GetFoliageCollisionDisplacement(vec3 world_position, vec3 model_position) {
  const float capsule_size = 200.0;
  const float movement_falloff_radius = 2500.0;

  vec3 adjusted_position = (world_position + model_position) / 2.0;

  vec3 displacement_direction = adjusted_position - foliage_mover_position;
  displacement_direction.y *= 0.0;

  float mover_distance = max(capsule_size, length(displacement_direction));
  float mover_y_distance = max(0.0, world_position.y - (foliage_mover_position.y - 1000.0));

  float foliage_mover_factor = clamp(1.0 - mover_distance / movement_falloff_radius, 0.0, 1.0);
  foliage_mover_factor *= foliage_mover_factor;

  foliage_mover_factor *= min(1.0, mover_y_distance / 500.0);

  float foliage_move_intensity = max(0.5, sqrt(length(foliage_mover_velocity) / 1800.0));

  return displacement_direction * foliage_move_intensity * foliage_mover_factor;
}

void main() {
  mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));
  vec3 N = vertexNormal;

  // For the vertex transform, start by just applying rotation + scale
  vec3 model_space_position = mat3(modelMatrix) * vertexPosition;

  // Then apply translation, offset by the transform origin
  vec3 translation = vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  // @todo handle grass behavior in shadow map pass
  // @todo rewrite this, this is borderline incomprehensible
  if (is_grass) {
    const float wind_strength = 300.0;
    const float wind_speed = 2.0;

    float vertex_y = vertexPosition.y;

    // Calculate wind
    float local_wind = GetFoliageLocalWind(modelMatrix, wind_speed, wind_strength);

    // Calculate the local drift intensity
    float drift_factor = GetFoliageDriftIntensity(local_wind, vertex_y);

    float alpha = 2.0 * scene_time + modelMatrix[3][0] * 0.01;

    world_space_position.x += drift_factor * sin(alpha);
    world_space_position.z += drift_factor * cos(1.5 * alpha);

    world_space_position += 1.5 * GetFoliageCollisionDisplacement(world_space_position, translation);
  }

  // @todo handle foliage behavior in shadow map pass
  if (is_foliage) {
    // @hack!!!!!!!!!!!
    bool is_flag_banner = (modelSurface & 0x00000F00) != 0;

    const float wind_strength = is_flag_banner ? 100.0 : 20.0;
    const float wind_speed = 1.5;

    float alpha = 2.0 * scene_time + (world_space_position.z + world_space_position.y) * 0.0015;

    float min_intensity = is_flag_banner ? 0.0 : 0.5;
    float max_intensity = 2.0;
    float core_intensity = clamp(length(vertexPosition.xz), min_intensity, max_intensity);

    float S = sin(wind_speed * alpha);
    float C = cos(1.3 * wind_speed * alpha);

    world_space_position.x += wind_strength * core_intensity * S;
    // world_space_position.y += wind_strength * core_intensity * S;
    world_space_position.z += wind_strength * core_intensity * C;

    world_space_position += GetFoliageCollisionDisplacement(world_space_position, translation);

    // @temporary
    // @todo export meshes with custom normals
    if (world_space_position.y > -2500.0) {
      vec3 p = vec3(vertexPosition.x, 0.2 * vertexPosition.y, vertexPosition.z);
      vec3 oN = normalize(p);
      N = normalize(mix(N, oN, 0.45));
    }
  }

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragSurface = SurfaceToUVec4(modelSurface);
  fragWorldPosition = world_space_position;
  fragNormal = normal_matrix * N;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);
  fragUv = vertexUv;
}