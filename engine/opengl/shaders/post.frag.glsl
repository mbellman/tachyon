#version 460 core

#define ENABLE_DEPTH_OF_FIELD_BLUR 1
#define ENABLE_CHROMATIC_ABERRATION 1

uniform sampler2D in_color_and_depth;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;
uniform vec3 primary_light_direction;

// Fx: Cosmodrone
uniform float scan_time;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

const vec2 TEXEL_SIZE = 1.0 / vec2(1920.0, 1080.0);
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

// @todo allow positions + size to be defined externally
// ideally we should be able to create fog volume meshes
const vec3[] fog_volume_positions = {
  vec3(0.0, -300000.0, 0.0),
  vec3(32000.0, -170000.0, 30000.0),
  vec3(50000.0, -270000.0, 80000.0),
  vec3(-60000.0, -170000.0, 40000.0)
};

vec4 GetVolumetricFogColorAndThickness(float depth, vec3 direction) {
  const vec3 light_fog_color = vec3(0.3, 0.7, 1.0);
  const vec3 dark_fog_color = vec3(0.0, 0.0, 0.2);

  float sun_dot = max(0.0, dot(direction, -primary_light_direction));

  vec3 color = vec3(0.0);
  float thickness = 0.0;

  for (int i = 0; i < 4; i++) {
    float camera_to_volume_distance = length(fog_volume_positions[i] - camera_position);

    vec3 sample_position =
      camera_position +
      direction * camera_to_volume_distance -
      direction * 150000.0 +
      direction * noise(1.0) * 2000.0;

    float resolution_alpha = clamp(camera_to_volume_distance / 2000000.0, 0.0, 1.0);
    int total_steps = int(mix(10.0, 5.0, resolution_alpha));
    float step_length = mix(20000.0, 40000.0, resolution_alpha);

    for (int j = 0; j < total_steps; j++) {
      sample_position += direction * step_length;

      float camera_to_sample_distance = length(sample_position - camera_position);

      if (camera_to_sample_distance > depth) {
        thickness *= 0.8;
      }

      vec3 volume_to_sample = sample_position - fog_volume_positions[i];
      float volume_distance = length(volume_to_sample);
      vec3 sample_direction = volume_to_sample / volume_distance;
      float distance_ratio = min(1.0, volume_distance / 100000.0);
      float NdotL = max(0.0, dot(sample_direction, -primary_light_direction));

      color += mix(dark_fog_color, light_fog_color, NdotL);
      thickness += 0.005 * (1.0 - distance_ratio);
    }
  }

  color = mix(color, light_fog_color * 20.0, pow(sun_dot, 50.0));

  return vec4(color, thickness);
}

void main() {
  vec4 color_and_depth = texture(in_color_and_depth, fragUv);
  vec3 post_color = color_and_depth.rgb;

  #if ENABLE_DEPTH_OF_FIELD_BLUR
    // Depth-of-field blur
    {
      const float max_blur = 1.5;
      float depth = color_and_depth.w;
      float blur = mix(0.0, max_blur, pow(depth, 100.0));

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
      const float intensity = 4.0;

      vec2 offset = intensity * (vec2(0.0) - 2.0 * (fragUv - 0.5));
      float r = texture(in_color_and_depth, fragUv + TEXEL_SIZE * offset).r;
      float g = texture(in_color_and_depth, fragUv + 0.5 * TEXEL_SIZE * offset).g;
      float b = texture(in_color_and_depth, fragUv + 0.2 * TEXEL_SIZE * offset).b;

      post_color += vec3(r, g, b);
      post_color /= 2.0;
    }
  #endif

  // Fog
  {
    // Fog volumes
    vec3 position = GetWorldPosition(color_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
    vec3 D = normalize(position - camera_position);
    float VdotD = max(0.0, -dot(D, primary_light_direction));
    float world_depth = GetWorldDepth(color_and_depth.w, Z_NEAR, Z_FAR);
    vec4 volumetric_fog = GetVolumetricFogColorAndThickness(world_depth, D);

    post_color = mix(post_color, volumetric_fog.rgb, volumetric_fog.w);

    // Depth fog
    float depth_factor = 0.25 * pow(color_and_depth.w, 300.0);

    post_color = mix(post_color, vec3(0.2, 0.4, 0.6), depth_factor);
    post_color = mix(post_color, vec3(0.8, 0.9, 1.0), depth_factor * pow(VdotD, 10.0));
    post_color = mix(post_color, vec3(2.0), depth_factor * pow(VdotD, 300.0));
  }

  // ---------------------
  // Game-specific effects
  //
  // Cosmodrone
  // ---------------------
  {
    // Vignette
    {
      const vec3 vignette_color = vec3(0.1, 0.3, 0.4);

      float vignette_factor =
        fragUv.x > 0.75
          ? (fragUv.x - 0.75) * 4.0 :
        fragUv.x < 0.2
          ? 0.4 * (1.0 - fragUv.x * 5.0) :
        0.0;

      // Tweak the gradation
      vignette_factor = mix(vignette_factor, 1.0, vignette_factor);
      vignette_factor *= vignette_factor;

      post_color = mix(post_color, post_color * vignette_color, vignette_factor);
    }

    // Scanner
    {
      const float distance_per_second = 100000.0;
      const float scan_line_thickness = 2000.0;
      const vec3 scan_area_color = vec3(0, 0.5, 1);
      const vec3 scan_line_color = 2.0 * vec3(0, 0.7, 1.0);
      float scan_distance = scan_time * distance_per_second;
      float world_depth = GetWorldDepth(color_and_depth.w, Z_NEAR, Z_FAR);

      // Color inside the scan area
      if (world_depth < distance_per_second * scan_time) {
        vec3 area_color = post_color * scan_area_color;
        float area_alpha = 1.0 - clamp(scan_time * 0.25, 0.0, 1.0);
        area_alpha *= min(1.0, world_depth / 10000.0);

        post_color = mix(post_color, area_color, area_alpha);
      }

      float line_alpha = 1.0 - clamp(abs(scan_distance - world_depth) / scan_line_thickness, 0.0, 1.0);
      line_alpha *= max(0.0, 1.0 - pow(scan_time / 4.0, 5.0));

      // Scan line
      post_color = mix(post_color, scan_line_color, line_alpha);
    }
  }

  const float contrast = 1.15;
  const float brightness = 0.05;
  post_color.rgb = ((post_color.rgb - 0.5) * max(contrast, 0)) + 0.5;
  post_color.rgb += brightness;

  out_color = post_color;
}