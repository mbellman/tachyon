#version 460 core

struct Light {
  vec3 position;
  float radius;
  vec3 color;
  float power;
};

uniform sampler2D in_normal_and_depth;
uniform sampler2D in_color_and_material;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;

noperspective in vec2 fragUv;
flat in Light light;
flat in vec2 center;
in float intensity;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_temporal_data;

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

float GetGlowFactor(vec3 world_position) {
  float dx = (1920.0 / 1080.0) * (fragUv.x - center.x);
  float dy = (fragUv.y - center.y);
  float distance_from_light_disc_center = sqrt(dx*dx + dy*dy);
  float light_distance_from_camera = length(light.position - camera_position);
  float surface_distance_from_camera = length(world_position - camera_position);

  float radius = distance_from_light_disc_center * light_distance_from_camera * 0.0001;
  float glow_factor = clamp(pow(1.0 - radius, 30.0), 0.0, 1.0);

  if (light_distance_from_camera < light.radius * 3.0) {
    glow_factor *= pow(light_distance_from_camera / (light.radius * 3.0), 3);
  }

  float occlusion_distance = max(0.0, light_distance_from_camera - surface_distance_from_camera);
  float occlusion_distance_factor = 1.0 - min(1.0, occlusion_distance / 500.0);

  glow_factor *= occlusion_distance_factor;
  glow_factor *= 1.5;

  float diffraction_factor =
    0.035 *
    occlusion_distance_factor *
    pow(1.0 - distance_from_light_disc_center, 50.0) *
    (1.0 / (light_distance_from_camera * 0.000001));

  glow_factor += diffraction_factor * pow(1.0 - abs(dy), 500.0);
  glow_factor += diffraction_factor * pow(1.0 - abs(dx), 500.0);

  if (isnan(glow_factor)) glow_factor = 0.0;

  float distance_factor = min(1.0, light_distance_from_camera / 20000.0);
  distance_factor *= distance_factor;
  distance_factor *= distance_factor;

  glow_factor *= distance_factor;

  return glow_factor;
}

vec3 GetPointLightRadiance(vec3 world_position, float light_distance, vec3 N, vec3 L) {
  vec3 radiant_flux = light.color * light.power;
  float NdotL = max(0.0, dot(N, -L));
  float distance_factor = 1.0 - min(1.0, light_distance / light.radius);

  // distance_factor *= distance_factor;

  // @todo use surface color
  vec3 albedo = vec3(1.0);

  // @todo PBR
  vec3 D = albedo * radiant_flux * distance_factor * NdotL;

  return D;
}

void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 light_to_surface = position - light.position;
  float light_distance = length(light_to_surface);
  float light_distance_from_camera = length(light.position - camera_position);
  vec3 out_color = vec3(0.0);

  vec3 N = frag_normal_and_depth.xyz;
  vec3 L = light_to_surface / light_distance;

  out_color += GetPointLightRadiance(position, light_distance, N, L);
  out_color += light.color * GetGlowFactor(position);

  out_color_and_depth = vec4(out_color, 0);
}