#version 460 core

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;

in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

const float Z_NEAR = 500.0;
const float Z_FAR = 100000000.0;

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

float LinearDepth(float depth, float near, float far) {
  return 2.0 * near / (far + near - depth * (far - near));
}

vec3 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;

  return vec3(r, g, b);
}

struct Material {
  float roughness;
  float metalness;
  float clearcoat;
  float subsurface;
};

Material UnpackMaterial(uvec4 surface) {
  float r = float((surface.z & 0xF0) >> 4) / 15.0;
  float m = float(surface.z & 0x0F) / 15.0;
  float c = float((surface.w & 0xF0) >> 4) / 15.0;
  float s = float(surface.w & 0x0F) / 15.0;

  return Material(r, m, c, s);
}

void main() {
  if (fragUv.x < 0.5 && fragUv.y >= 0.5) {
    // Top left (color)
    uvec4 color_and_material = texture(in_color_and_material, 2.0 * (fragUv - vec2(0, 0.5)));
    vec3 color = UnpackColor(color_and_material);

    out_color_and_depth = vec4(color, 1.0);
  } else if (fragUv.x >= 0.5 && fragUv.y >= 0.5) {
    // Top right (depth/position)
    vec2 uv = 2.0 * (fragUv - vec2(0.5));
    float frag_depth = texture(in_normal_and_depth, uv).w;
    float depth_color = 0.7 * pow(1.0 - LinearDepth(frag_depth, Z_NEAR, Z_FAR), 50);
    vec3 position = GetWorldPosition(frag_depth, uv, inverse_projection_matrix, inverse_view_matrix);
    vec3 position_color = position * 0.001;

    position_color.x = clamp(position_color.x, 0, 1);
    position_color.y = clamp(position_color.y, 0, 1);
    position_color.z = clamp(position_color.z, 0, 1);
    position_color *= 0.5;

    out_color_and_depth = vec4(depth_color + position_color, 1.0);
  } else if (fragUv.x < 0.5 && fragUv.y < 0.5) {
    // Bottom left (normal)
    vec3 normal = texture(in_normal_and_depth, fragUv * 1.999).xyz;

    out_color_and_depth = vec4(normal, 1.0);
  } else {
    // Bottom right (material)
    uvec4 color_and_material = texture(in_color_and_material, 2.0 * (fragUv - vec2(0.5, 0)));
    Material material = UnpackMaterial(color_and_material);

    out_color_and_depth = vec4(material.roughness, material.metalness, material.subsurface, material.clearcoat);
  }
}