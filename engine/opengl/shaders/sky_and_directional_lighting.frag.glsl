#version 460 core

uniform sampler2D in_color_and_depth;
uniform sampler2D in_normal_and_material;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

/**
 * Reconstructs a fragment's world position from depth,
 * using the inverse projection/view matrices to transform
 * the fragment coordinates back into world space.
 */
vec3 GetWorldPosition(float depth, vec2 frag_uv, mat4 inverse_projection, mat4 inverse_view) {
  float z = depth * 2.0 - 1.0;
  vec4 clip = vec4(frag_uv * 2.0 - 1.0, z, 1.0);
  vec4 view_position = inverse_projection * clip;

  view_position /= view_position.w;

  vec4 world_position = inverse_view * view_position;

  return world_position.xyz;
}

vec3 GetDirectionalLightRadiance(vec3 albedo, vec3 normal, vec3 position) {
  // @temporary @todo pass as parameters
  vec3 light_direction = vec3(-1.0, -1.0, -1.0);
  vec3 light_color = vec3(1.0);
  float roughness = 0.6;
  float metalness = 1.0;

  vec3 L = -normalize(light_direction);
  vec3 C = normalize(camera_position - position);
  vec3 H = normalize(C + L);

  float I = max(dot(normal, L), 0.0) * roughness;
  float S = pow(max(dot(normal, H), 0.0), 50) * (1.0 - roughness);

  vec3 fD = albedo;
  vec3 fS = mix(light_color, albedo, metalness);

  return fD * light_color * I + fS * vec3(1.0) * S;
}

void main() {
  vec4 frag_color_and_depth = texture(in_color_and_depth, fragUv);
  vec4 frag_normal_and_material = texture(in_normal_and_material, fragUv);

  vec3 albedo = frag_color_and_depth.rgb;
  vec3 normal = frag_normal_and_material.xyz;
  vec3 position = GetWorldPosition(frag_color_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 color = GetDirectionalLightRadiance(albedo, normal, position);

  out_color_and_depth = vec4(color, frag_color_and_depth.w);
}