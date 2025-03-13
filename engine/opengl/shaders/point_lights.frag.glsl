#version 460 core

#define USE_GAMMA_CORRECTION 1

struct Light {
  vec3 position;
  float radius;
  vec3 color;
  float power;
};

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
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

struct Material {
  float roughness;
  float metalness;
  float clearcoat;
  float subsurface;
};

Material UnpackMaterial(uvec4 surface) {
  float roughness = float((surface.z & 0xF0) >> 4) / 15.0;
  float metalness = float(surface.z & 0x0F) / 15.0;
  float clearcoat = float((surface.w & 0xF0) >> 4) / 15.0;
  float subsurface = float(surface.w & 0x0F) / 15.0;

  return Material(roughness, metalness, clearcoat, subsurface);
}

float GetGlowDistanceFactor(float light_distance) {
  if (light_distance < 100000.0) {
    return 1.0;
  }

  return max(0.0, mix(1.0, 0.2, light_distance / 2000000.0));
}

float GetGlowFactor(vec3 world_position) {
  float dx = (1920.0 / 1080.0) * (fragUv.x - center.x);
  float dy = (fragUv.y - center.y);
  float light_distance_from_camera = length(light.position - camera_position);
  float surface_distance_from_camera = length(world_position - camera_position);
  float distance_from_light_disc_center = sqrt(dx*dx + dy*dy) * (light_distance_from_camera / light.radius);
  if (distance_from_light_disc_center > 1.0) distance_from_light_disc_center = 1.0;

  float radius = distance_from_light_disc_center * light_distance_from_camera * 0.0001;
  float glow_factor = clamp(pow(1.0 - radius, 40.0), 0.0, 1.0);

  if (light_distance_from_camera < light.radius * 3.0) {
    glow_factor *= pow(light_distance_from_camera / (light.radius * 3.0), 3);
  }

  float occlusion_distance = max(0.0, light_distance_from_camera - surface_distance_from_camera);
  float occlusion_distance_factor = 1.0 - min(1.0, occlusion_distance / 500.0);

  // Volumetric glow
  glow_factor += 0.1 * pow(1.0 - distance_from_light_disc_center, 5.0);

  // Hide behind closer objects
  glow_factor *= occlusion_distance_factor;
  glow_factor *= 1.5;

  #if USE_GAMMA_CORRECTION == 1
    const float disc_exponent = 10.0;
  #else
    const float disc_exponent = 4.0;
  #endif

  // Diffraction spikes
  float diffraction_factor =
    mix(1.0, 5.0, light_distance_from_camera / 500000.0) *
    occlusion_distance_factor *
    pow(1.0 - distance_from_light_disc_center, disc_exponent);

  glow_factor += diffraction_factor * pow(1.0 - abs(dy), 2048.0);
  glow_factor += diffraction_factor * pow(1.0 - abs(dx), 2048.0);

  if (isnan(glow_factor)) glow_factor = 0.0;

  float distance_factor = GetGlowDistanceFactor(light_distance_from_camera);

  glow_factor *= distance_factor;

  return glow_factor;
}

vec3 GetPointLightRadiance(vec3 world_position, float light_distance, vec3 N, vec3 L, vec3 V, Material material) {
  vec3 H = normalize(V + L);
  float NdotL = max(0.0, dot(N, L));
  float NdotH = max(dot(N, H), 0.0);

  vec3 radiant_flux = light.color * light.power;
  float distance_factor = 1.0 - min(1.0, light_distance / light.radius);

  #if USE_GAMMA_CORRECTION == 1
    distance_factor *= distance_factor;
    distance_factor *= distance_factor;
  #endif

  // @todo use surface color
  vec3 albedo = vec3(1.0);

  // @todo PBR
  vec3 D = albedo * radiant_flux * distance_factor * NdotL;
  vec3 S = radiant_flux * pow(NdotH, 50.0) * distance_factor;

  // return D + 2.0 * S;

  return D + 3.0 * (material.metalness + 2.0 * material.clearcoat + 2.0 * (1.0 - material.roughness)) * S;
}

void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  uvec4 frag_color_and_material = texture(in_color_and_material, fragUv);

  Material material = UnpackMaterial(frag_color_and_material);

  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 surface_to_light = light.position - position;
  float light_distance = length(surface_to_light);
  float light_distance_from_camera = length(light.position - camera_position);
  vec3 out_color = vec3(0.0);

  vec3 N = frag_normal_and_depth.xyz;
  vec3 V = normalize(camera_position - position);
  vec3 L = surface_to_light / light_distance;

  out_color += GetPointLightRadiance(position, light_distance, N, L, V, material);
  out_color += light.color * GetGlowFactor(position) * min(1.0, light.power);
  // out_color += light.color * 0.1;

  #if USE_GAMMA_CORRECTION == 1
    out_color *= light.color;
    out_color = pow(out_color, vec3(1.0 / 2.2));
  #endif

  out_color_and_depth = vec4(out_color, 0);
}