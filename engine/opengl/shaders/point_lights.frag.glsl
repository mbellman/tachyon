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

// @todo we still have to do actual lighting, not just the glow effect
void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 light_to_surface = position - light.position;
  float light_distance = length(light_to_surface);
  float light_distance_from_camera = length(light.position - camera_position);
  vec3 N = frag_normal_and_depth.xyz;
  vec3 L = light_to_surface / light_distance;
  float NdotL = max(0.0, dot(N, -L));

  // @todo cleanup
  float dx = (1920.0 / 1080.0) * (fragUv.x - center.x);
  float dy = (fragUv.y - center.y);
  float distance_from_light_disc_center = sqrt(dx*dx + dy*dy);

  float radius = distance_from_light_disc_center * light_distance_from_camera * 0.0001;
  float glow_factor = clamp(pow(1.0 - radius, 30), 0.0, 1.0);

  if (light_distance_from_camera < light.radius * 3.0) {
    glow_factor *= pow(light_distance_from_camera / (light.radius * 3.0), 3);
  }

  float occlusion_distance = max(0.0, light_distance_from_camera - length(position - camera_position));

  glow_factor *= 1.0 - min(1.0, occlusion_distance / 500.0);
  glow_factor *= 1.5;

  out_color_and_depth = vec4(light.color * glow_factor, 1.0);
}