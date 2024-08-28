#version 460 core

uniform sampler2D in_color_and_depth;
uniform sampler2D in_normal_and_material;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;

in vec2 fragUv;

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

void main() {
  if (fragUv.x < 0.5 && fragUv.y >= 0.5) {
    // Top left (color)
    vec3 frag_color = texture(in_color_and_depth, 2.0 * (fragUv - vec2(0, 0.5))).rgb;

    out_color_and_depth = vec4(frag_color, 1.0);
  } else if (fragUv.x >= 0.5 && fragUv.y >= 0.5) {
    // Top right (depth/position)
    vec2 uv = 2.0 * (fragUv - vec2(0.5));
    float frag_depth = texture(in_color_and_depth, uv).w;
    float depth_color = 1.0 - pow(frag_depth, 100);
    vec3 position = GetWorldPosition(frag_depth, uv, inverse_projection_matrix, inverse_view_matrix);
    vec3 position_color = position * 0.001;

    position_color.x = clamp(position_color.x, 0, 1);
    position_color.y = clamp(position_color.y, 0, 1);
    position_color.z = clamp(position_color.z, 0, 1);
    position_color *= 0.5;

    out_color_and_depth = vec4(vec3(depth_color) + position_color, 1.0);
  } else if (fragUv.x < 0.5 && fragUv.y < 0.5) {
    // Bottom left (normals)
    vec3 normals = texture(in_normal_and_material, fragUv * 1.999).rgb;

    out_color_and_depth = vec4(normals, 1.0);
  } else {
    // Bottom right (@todo material)
    out_color_and_depth = vec4(vec3(0), 1.0);
  }
}