#version 460 core

#define ENABLE_IMAGE_SMOOTHING 1
#define ENABLE_DEPTH_OF_FIELD_BLUR 1
#define ENABLE_CHROMATIC_ABERRATION 0

#define ENABLE_COSMODRONE_FX 0
#define ENABLE_ASTRO_FX 1

uniform sampler2D in_color_and_depth;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;
uniform vec3 primary_light_direction;

// Fx: Cosmodrone
uniform float scan_time;

// Fx: Alchemist's Astrolabe
uniform vec3 player_position;
uniform float astro_time_warp;
uniform float astro_time_warp_start_radius;
uniform float astro_time_warp_end_radius;
uniform float haze_intensity;
uniform float vignette_intensity;
uniform float dialogue_overlay_opacity;

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

struct BilinearSample {
  vec4 original;
  vec4 filtered;
};

/**
 * Sourced from https://iquilezles.org/articles/hwinterpolation/
 * with slight modifications.
 */
BilinearSample GetBilinearTextureSample( sampler2D sam, vec2 uv ) {
  ivec2 ires = textureSize( sam, 0 );
  vec2  fres = vec2( ires );

  vec2 st = (fract(uv)-0.5/fres)*fres;
  ivec2 i = ivec2( floor( st ) );
  vec2  w = fract( st );

  vec4 a = texelFetch( sam, (i+ivec2(0,0)), 0 );
  vec4 b = texelFetch( sam, (i+ivec2(1,0)), 0 );
  vec4 c = texelFetch( sam, (i+ivec2(0,1)), 0 );
  vec4 d = texelFetch( sam, (i+ivec2(1,1)), 0 );

  BilinearSample s;
  s.original = a;
  s.filtered = mix(mix(a, b, w.x), mix(c, d, w.x), w.y);

  return s;
}

float Saturate(float value) {
  return clamp(value, 0.0, 1.0);
}

float Compare(vec3 c1, vec3 c2) {
  vec3 delta = vec3(c1.r - c2.r, c1.g - c2.g, c1.b - c2.b);

  return Saturate(1.0 - length(delta));
}

vec3 GetToonShadedColor(vec3 current_out_color, vec2 uv, float depth, float linear_frag_depth) {
  #define USE_INSET_OUTLINE 0

  const float OUTLINE_THICKNESS = 4.0;

  // Get the depth values for the top/left/right/bottom pixels
  vec2 texel_size = 1.0 / textureSize(in_color_and_depth, 0);
  vec2 uv1 = uv + texel_size * vec2(-1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv2 = uv + texel_size * vec2(1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv3 = uv + texel_size * vec2(0, -1.0) * OUTLINE_THICKNESS;
  vec2 uv4 = uv + texel_size * vec2(0, 1.0) * OUTLINE_THICKNESS;

  vec4 t1 = texture(in_color_and_depth, uv1);
  vec4 t2 = texture(in_color_and_depth, uv2);
  vec4 t3 = texture(in_color_and_depth, uv3);
  vec4 t4 = texture(in_color_and_depth, uv4);

  float depth1 = GetWorldDepth(t1.w, Z_NEAR, Z_FAR);
  float depth2 = GetWorldDepth(t2.w, Z_NEAR, Z_FAR);
  float depth3 = GetWorldDepth(t3.w, Z_NEAR, Z_FAR);
  float depth4 = GetWorldDepth(t4.w, Z_NEAR, Z_FAR);

  #if USE_INSET_OUTLINE == 1
    float d1 = depth1 - linear_frag_depth;
    float d2 = depth2 - linear_frag_depth;
    float d3 = depth3 - linear_frag_depth;
    float d4 = depth4 - linear_frag_depth;
  #else
    float d1 = linear_frag_depth - depth1;
    float d2 = linear_frag_depth - depth2;
    float d3 = linear_frag_depth - depth3;
    float d4 = linear_frag_depth - depth4;
  #endif

  vec3 c1 = t1.rgb;
  vec3 c2 = t2.rgb;
  vec3 c3 = t3.rgb;
  vec3 c4 = t4.rgb;

  // Determine how sharp of an angle the surface is being viewed at
  vec3 world_position = GetWorldPosition(depth, uv, inverse_projection_matrix, inverse_view_matrix);
  vec3 normalized_frag_to_camera = normalize(camera_position - world_position);
  float grazing_factor = 0.17;

  // Calculate a distance threshold representing how far away a neighboring
  // pixel must be to consider this a silhouette edge
  float depth_ratio = linear_frag_depth / Z_FAR;
  float threshold_factor = 1000.0 + (Z_FAR / 2.0) * grazing_factor;
  float distance_threshold = threshold_factor * depth_ratio;

  bool dt1 = d1 > distance_threshold;
  bool dt2 = d2 > distance_threshold;
  bool dt3 = d3 > distance_threshold;
  bool dt4 = d4 > distance_threshold;

  if (dt1 || dt2 || dt3 || dt4) {
    float compared_depth = min(depth1, min(depth2, min(depth3, depth4)));
    float falloff_alpha = smoothstep(0.8, 0.9, Saturate(compared_depth / 20000.0));
    vec3 compared_color = dt1 ? c1 : dt2 ? c2 : dt3 ? c3 : c4;
    float color_similarity = Compare(current_out_color, compared_color);

    #if USE_INSET_OUTLINE == 1
      float brightness = mix(0.0, 0.9, color_similarity);
      vec3 outline_color = current_out_color * brightness;

      current_out_color = mix(outline_color, current_out_color, falloff_alpha);
    #else
      float brightness = mix(0.0, 1.0, color_similarity);
      vec3 outline_color = compared_color * pow(brightness, 2.0);

      current_out_color = mix(outline_color, current_out_color, falloff_alpha);
    #endif
  }

  return current_out_color;
}

void main() {
  #if ENABLE_IMAGE_SMOOTHING
    BilinearSample color_and_depth_sample = GetBilinearTextureSample(in_color_and_depth, fragUv);

    vec4 color_and_depth;
    // Use the filtered color sample
    color_and_depth.rgb = color_and_depth_sample.filtered.rgb;
    // Use the original depth sample to avoid erroneously-resolved world depth values
    color_and_depth.w = color_and_depth_sample.original.w;
  #else
    vec4 color_and_depth = texture(in_color_and_depth, fragUv);
  #endif

  vec3 post_color = color_and_depth.rgb;

  float world_depth = GetWorldDepth(color_and_depth.w, Z_NEAR, Z_FAR);
  vec3 world_position = GetWorldPosition(color_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);

  #if ENABLE_DEPTH_OF_FIELD_BLUR
    // Depth-of-field blur
    {
      const float max_blur = 2.0;

      float depth = color_and_depth.w;
      float adjusted_depth = pow(depth, 50.0);
      float blur = max_blur * smoothstep(0.2, 0.3, adjusted_depth);

      const vec2[] offsets = {
        vec2(0.0, -1.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(-1.0, 0.0),
      };

      for (int i = 0; i < 4; i++) {
        vec2 uv = fragUv + blur * TEXEL_SIZE * offsets[i];

        #if ENABLE_IMAGE_SMOOTHING
          post_color += GetBilinearTextureSample(in_color_and_depth, uv).filtered.rgb;
        #else
          post_color += texture(in_color_and_depth, uv).rgb;
        #endif
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
    vec3 D = normalize(world_position - camera_position);
    float VdotD = max(0.0, -dot(D, primary_light_direction));

    #if ENABLE_COSMODRONE_FX
      vec4 volumetric_fog = GetVolumetricFogColorAndThickness(world_depth, D);

      post_color = mix(post_color, volumetric_fog.rgb, volumetric_fog.w);
    #endif

    // Depth haze
    #if ENABLE_ASTRO_FX
      float depth_factor = haze_intensity * pow(color_and_depth.w, 20.0);
      vec3 fog_color = vec3(0.2, 0.4, 0.5);

      post_color = mix(post_color, fog_color, depth_factor);
    #elif ENABLE_COSMODRONE_FX
      float depth_factor = 0.25 * pow(color_and_depth.w, 300.0);

      post_color = mix(post_color, vec3(0.2, 0.4, 0.6), depth_factor);
      post_color = mix(post_color, vec3(0.8, 0.9, 1.0), depth_factor * pow(VdotD, 10.0));
      post_color = mix(post_color, vec3(2.0), depth_factor * pow(VdotD, 300.0));
    #else
      float depth_factor = 0.25 * pow(color_and_depth.w, 300.0);
    #endif

    if (color_and_depth.w < 1.0) {
      post_color += mix(vec3(0), vec3(0.05, 0.1, 0.2), min(1.0, world_depth / 10000000.0));
    }
  }

  // ---------------------
  // Game-specific effects
  // ---------------------

  // ---------------------
  // Cosmodrone
  // ---------------------
  #if ENABLE_COSMODRONE_FX
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

      // Color inside the scan area
      if (world_depth < distance_per_second * scan_time) {
        vec3 area_color = post_color * scan_area_color;
        float area_alpha = 1.0 - clamp(scan_time * 0.25, 0.0, 1.0);
        area_alpha *= min(1.0, world_depth / 10000.0);

        post_color = mix(post_color, area_color, area_alpha);
      }

      float line_alpha = 1.0 - clamp(abs(scan_distance - world_depth) / scan_line_thickness, 0.0, 1.0);
      line_alpha *= max(0.0, 1.0 - pow(scan_time / 4.0, 5.0));
      line_alpha *= pow(color_and_depth.w, 6.0);

      // Scan line
      post_color = mix(post_color, scan_line_color, line_alpha);

      // Pulse
      float pulse_alpha = 0.25 * (1.0 - min(1.0, scan_time / 0.5));

      post_color = mix(post_color, scan_area_color, pulse_alpha);
    }
  #endif

  // ---------------------
  // Alchemist's Astrolabe
  // ---------------------
  #if ENABLE_ASTRO_FX
    // Artstyle
    {
      post_color = GetToonShadedColor(post_color, fragUv, color_and_depth.w, world_depth);

      // @todo formalize/improve
      float compared = Compare(post_color, vec3(0));

      if (compared > 0.6) {
        float offset =
          gl_FragCoord.x * 0.4 +
          10.0 * sin(gl_FragCoord.x * 0.05) +
          world_depth * 0.005;

        int line_y = int(gl_FragCoord.y + offset);

        if (line_y % 4 == 0 || line_y % 4 == 1) {
          post_color *= (1.0 - compared * compared);
        } else {
          post_color *= 1.1;
        }
      }
    }

    // Dialogue overlay
    {
      if (fragUv.y < 0.3) {
        const vec3 overlay_color = vec3(0.0, 0.0, 0.1);

        float top_opacity = smoothstep(0.3, 0.18, fragUv.y);
        float bottom_opacity = smoothstep(0.0, 0.12, fragUv.y);
        float left_opacity = smoothstep(0.15, 0.25, fragUv.x);
        float right_opacity = smoothstep(0.85, 0.75, fragUv.x);
        float opacity = dialogue_overlay_opacity * top_opacity * bottom_opacity * left_opacity * right_opacity;

        post_color = mix(post_color, overlay_color, opacity);
      }
    }

    // Time warping
    // @todo we may have to relocate this into global_lighting!
    // These effects are not accumulated for blurring, so some
    // of the blur effects don't look correct.
    {
      const float haze_radius = 30000.0;

      vec3 player_to_fragment = world_position - player_position;
      float frag_distance_from_player = length(player_to_fragment);
      float start_bubble_radius = astro_time_warp_start_radius;
      float end_bubble_radius = astro_time_warp_end_radius;
      float haze_factor = frag_distance_from_player / haze_radius;

      float ring_rotation = -2.0 * (start_bubble_radius / haze_radius) * sign(astro_time_warp);
      float ring_thickness = 5000.0 * (0.5 + 0.5 * sin(12.0 * (atan(player_to_fragment.z, player_to_fragment.x) + ring_rotation)));
      float frag_distance_from_ring = distance(frag_distance_from_player, start_bubble_radius);
      float ring_intensity = max(0.0, 1.0 - frag_distance_from_ring / ring_thickness);
      float ring_factor = mix(0.0, 1.0, ring_intensity);

      // Adjustment: fade the ring closer to the player to avoid striping artifacts
      ring_factor *= pow(min(1.0, frag_distance_from_player / 10000.0), 2.0);
      // Adjustment: fade the ring in as the warp bubble grows
      ring_factor *= min(1.0, pow(start_bubble_radius / 10000.0, 3.0));

      if (frag_distance_from_player > start_bubble_radius) {
        haze_factor = 0.0;
      }

      if (end_bubble_radius < start_bubble_radius || astro_time_warp == 0.0) {
        float falloff = 1.0 - min(1.0, end_bubble_radius / haze_radius);

        haze_factor = falloff * frag_distance_from_player / haze_radius;
        ring_factor = 0.0;
      }

      if (world_depth < 2500.0 || start_bubble_radius == 0.0) {
        haze_factor = 0.0;
        ring_factor = 0.0;
      }

      haze_factor *= haze_factor;

      post_color = mix(post_color, vec3(1.0, 0.8, 0.4), haze_factor);
      post_color = mix(post_color, vec3(1.0, 1.0, 0.7), ring_factor);
    }

    // Vignette
    {
      float corner_distance = length(fragUv - 0.5);

      float alpha = corner_distance / 0.75;
      alpha *= alpha;
      alpha *= vignette_intensity;

      if (world_depth > 2200.0) {
        post_color = mix(post_color, vec3(0), alpha);
      }
    }
  #endif

  const float contrast = 1.25;
  const float brightness = 1.05;

  post_color.rgb = contrast * (post_color.rgb - 0.5) + 0.5;
  post_color.rgb += (brightness - 1.0);

  out_color = post_color;
}