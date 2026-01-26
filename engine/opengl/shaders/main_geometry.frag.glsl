#version 460 core

uniform bool has_texture;
uniform sampler2D albedo_texture;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_normal_and_depth;
layout (location = 1) out uvec4 out_color_and_material;

// @todo pass in as a uniform
vec2 RESOLUTION = vec2(2560.0, 1440.0);
const float Z_NEAR = 500.0;
const float Z_FAR = 100000000.0;

/**
 * Maps a nonlinear [0, 1] depth value to a linearized
 * depth between the near and far planes.
 */
float GetWorldDepth(float depth, float near, float far) {
  float clip_depth = 2.0 * depth - 1.0;

  return 2.0 * near * far / (far + near - clip_depth * (far - near));
}

// @todo relocate
mat4 dither_kernels[] = {
  // Level 0
  mat4(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  ),

  // Level 1
  mat4(
    0, 0, 0, 1,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
  ),

  // Level 2
  mat4(
    0, 0, 0, 1,
    0, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 0, 0
  ),

  // Level 3
  mat4(
    0, 0, 0, 1,
    0, 1, 0, 0,
    0, 0, 0, 1,
    0, 1, 0, 0
  ),

  // Level 4
  mat4(
    0, 1, 0, 1,
    0, 0, 0, 0,
    0, 1, 0, 1,
    0, 0, 0, 0
  ),

  // Level 5
  mat4(
    0, 1, 0, 1,
    0, 0, 1, 0,
    0, 1, 0, 1,
    0, 0, 0, 0
  ),

  // Level 6
  mat4(
    0, 1, 0, 1,
    1, 0, 0, 0,
    0, 1, 0, 1,
    1, 0, 0, 0
  ),

  // Level 7
  mat4(
    0, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 0, 1,
    1, 0, 0, 0
  ),

  // Level 8
  mat4(
    0, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 0, 1,
    1, 0, 1, 0
  ),

  // Level 9
  mat4(
    0, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 0, 1,
    1, 0, 1, 0
  ),

  // Level 10
  mat4(
    0, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 1, 1,
    1, 0, 1, 0
  ),

  // Level 11
  mat4(
    1, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 1, 1,
    1, 0, 1, 0
  ),

  // Level 12
  mat4(
    1, 1, 1, 1,
    1, 0, 1, 0,
    1, 1, 1, 1,
    1, 0, 1, 0
  ),

  // Level 13
  mat4(
    1, 1, 1, 1,
    1, 1, 1, 0,
    1, 1, 1, 1,
    1, 0, 1, 0
  ),

  // Level 14
  mat4(
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 0, 1, 0
  ),

  // Level 15
  mat4(
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 0
  )
};

void main() {
  bool is_close_to_camera = GetWorldDepth(gl_FragCoord.z, Z_NEAR, Z_FAR) < 9000.0;
  float screen_center_distance = length(gl_FragCoord.xy - RESOLUTION / 2.0);
  bool is_center_frame = screen_center_distance < 900.0;
  int dither_level = clamp(int(screen_center_distance / 50.0) - 1, 0, 15);
  mat4 dither_kernel = dither_kernels[dither_level];
  float dither_value = dither_kernel[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4];

  if (
    is_close_to_camera &&
    is_center_frame &&
    dither_value == 0.0
  ) {
    discard;
  }

  out_normal_and_depth = vec4(normalize(fragNormal), gl_FragCoord.z);
  out_color_and_material = fragSurface;

  if (has_texture) {
    vec4 albedo = texture(albedo_texture, fragUv);

    // @todo factor
    uint r = uint(albedo.r * 15);
    uint g = uint(albedo.g * 15);
    uint b = uint(albedo.b * 15);

    uint x = (r << 4) | g;
    uint y = (b << 4) | (fragSurface.y & 0x0F);

    out_color_and_material = uvec4(x, y, fragSurface.z, fragSurface.w);
  }
}