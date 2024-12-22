#version 460 core

#define ENABLE_DEPTH_OF_FIELD_BLUR 1
#define ENABLE_CHROMATIC_ABERRATION 1

uniform sampler2D in_color_and_depth;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

const vec2 TEXEL_SIZE = 1.0 / vec2(1920.0, 1080.0);

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

/**
 * Maps a nonlinear [0, 1] depth value to a linearized
 * depth between the near and far planes.
 */
float GetWorldDepth(float depth, float near, float far) {
  float clip_depth = 2.0 * depth - 1.0;

  return 2.0 * near * far / (far + near - clip_depth * (far - near));
}

/**
 * Returns a value within the range -1.0 - 1.0, constant
 * in screen space, acting as a noise filter.
 */
float noise(float seed) {
  return 2.0 * (fract(sin(dot(gl_FragCoord.xy * 0.01, vec2(12.9898, 78.233))) * seed * 43758.545312) - 0.5);
}

const vec3[] fog_volume_positions = {
  vec3(5000.0, 100000.0, 100000.0),
  vec3(75000.0, 90000.0, 40000.0),
  vec3(10000.0, 200000.0, 10000.0),
  vec3(10000.0, 270000.0, 36000.0),
};

float GetVolumetricLightStrength(float depth, vec3 direction) {
  vec3 sample_position = camera_position + direction * noise(1.0) * 1000.0;
  float step_length = 10000.0;
  float strength = 1.0;

  for (int i = 0; i < 25; i++) {
    sample_position += direction * step_length;

    if (length(sample_position - camera_position) > depth) {
      break;
    }

    for (int i = 0; i < 4; i++) {
      float volume_distance = length(sample_position - fog_volume_positions[i]);
      float distance_ratio = min(1.0, volume_distance / 100000.0);

      strength *= 1.0 - 0.05 * (1.0 - distance_ratio);
    }
  }

  return strength;
}

void main() {
  vec4 color_and_depth = texture(in_color_and_depth, fragUv);
  vec3 post_color = color_and_depth.rgb;

  #if ENABLE_DEPTH_OF_FIELD_BLUR
    // Depth-of-field blur
    {
      const float max_blur = 1.5;
      float depth = color_and_depth.w;
      float blur = mix(0.0, max_blur, pow(depth, 200.0));

      const vec2[] offsets = {
        vec2(0.0, -1.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(-1.0, 0.0),
      };

      for (int i = 0; i < 4; i++) {
        vec2 uv = fragUv + blur * TEXEL_SIZE * offsets[i];

        post_color += texture(in_color_and_depth, uv).rgb;
      }

      post_color /= 5.0;
    }
  #endif

  #if ENABLE_CHROMATIC_ABERRATION
    // Chromatic aberration
    {
      const float intensity = 3.0;

      vec2 offset = intensity * (vec2(0.0) - 2.0 * (fragUv - 0.5));
      float r = texture(in_color_and_depth, fragUv + TEXEL_SIZE * offset).r;
      float g = texture(in_color_and_depth, fragUv + 0.5 * TEXEL_SIZE * offset).g;
      float b = texture(in_color_and_depth, fragUv + 0.2 * TEXEL_SIZE * offset).b;

      post_color += vec3(r, g, b);
      post_color /= 2.0;
    }
  #endif

  vec3 position = GetWorldPosition(color_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 D = normalize(position - camera_position);
  float world_depth = GetWorldDepth(color_and_depth.w, 500.0, 10000000.0);
  float volumetric_light_strength = GetVolumetricLightStrength(world_depth, D);

  vec3 inner_fog_color = vec3(0.6, 0.6, 1.0);
  vec3 outer_fog_color = vec3(0.0, 0.1, 0.2);
  vec3 fog_color = mix(inner_fog_color, outer_fog_color, volumetric_light_strength);

  post_color = mix(fog_color, post_color, volumetric_light_strength);

  out_color = post_color;
}