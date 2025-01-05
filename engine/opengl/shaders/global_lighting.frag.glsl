#version 460 core

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
uniform sampler2D in_temporal_data;

uniform sampler2D in_shadow_map_cascade_1;
uniform sampler2D in_shadow_map_cascade_2;
uniform sampler2D in_shadow_map_cascade_3;
uniform sampler2D in_shadow_map_cascade_4;
uniform mat4 light_matrix_cascade_1;
uniform mat4 light_matrix_cascade_2;
uniform mat4 light_matrix_cascade_3;
uniform mat4 light_matrix_cascade_4;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 previous_view_matrix;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;
uniform float scene_time;
uniform float running_time;
// @temporary
// @todo allow multiple directional lights
uniform vec3 directional_light_direction;

// @todo dev mode only
uniform bool use_high_visibility_mode;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_temporal_data;

/**
 * Returns a value within the range -1.0 - 1.0, constant
 * in screen space, acting as a noise filter.
 */
float noise(float seed) {
  return 2.0 * (fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * seed * 43758.545312) - 0.5);
}

/**
 * Returns the 2D screen coordinates, normalized to the range
 * [0.0, 1.0], corresponding to a point in view space.
 */
vec2 GetScreenCoordinates(vec3 view_position, mat4 projection_matrix) {
  vec4 projection = projection_matrix * vec4(view_position, 1.0);
  vec3 clip = projection.xyz / projection.w;

  return clip.xy * 0.5 + 0.5;
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
 * Converts a nonlinear [0, 1] depth value to a linearized
 * depth between 0 and 1.
 */
float GetLinearDepth(float depth, float near, float far) {
  return 2.0 * near / (far + near - depth * (far - near));
}

/**
 * Returns a value clamped between 0 and 1.
 */
float saturate(float value) {
  return clamp(value, 0.0, 1.0);
}

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

const float PI = 3.141592;

float DistributionGGX(float NdotH, float roughness) {
  float r4 = roughness*roughness * roughness*roughness;
  float d = (NdotH*NdotH * (r4 - 1.0) + 1.0);

  return r4 / (PI * d*d);
}

float GeometryGGX(float NdotH, float roughness, float metalness) {
  return pow(NdotH, 2 * roughness) * pow(1.0 - metalness, 4);
}

float Clearcoat(float NdotH, float NdotV, float clearcoat) {
  return clearcoat * (DistributionGGX(NdotH, 0.1) + pow(1.0 - NdotV, 8));
}

float Subsurface(float NdotV, float subsurface) {
  return subsurface * (4 * NdotV + 16 * pow(1.0 - NdotV, 4));
}

vec3 GetDirectionalLightRadiance(
  vec3 light_direction,
  vec3 light_color,
  vec3 albedo,
  vec3 position,
  vec3 N,
  vec3 V,
  float NdotV,
  float roughness,
  float metalness,
  float clearcoat,
  float subsurface,
  float shadow_factor
) {
  vec3 L = -light_direction;
  vec3 H = normalize(V + L);

  float NdotH = max(dot(N, H), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float light_factor = NdotL * shadow_factor;

  float sD = DistributionGGX(NdotH, roughness);
  float sG = GeometryGGX(NdotH, roughness, metalness);

  float D = (1.0 - metalness) * (1.0 - roughness * 0.5) * light_factor;
  float Sp = (sD + sG) * light_factor;
  float C = Clearcoat(NdotH, NdotV, clearcoat) * light_factor;
  // @todo pass the additional terms into Subsurface()
  float Sc = Subsurface(NdotV, subsurface) * (light_factor + 0.05) * (1.0 - metalness * 0.95);

  return light_color * (albedo * D + albedo * Sp + C + albedo * albedo * Sc) / PI;
}

const mat4[] light_matrices = {
  light_matrix_cascade_1,
  light_matrix_cascade_2,
  light_matrix_cascade_3,
  light_matrix_cascade_4
};

int GetCascadeIndex(float depth) {
  float world_depth = GetWorldDepth(depth, 500.0, 10000000.0);

  if (world_depth < 10000.0) {
    return 0;
  } else if (world_depth < 50000.0) {
    return 1;
  } else if (world_depth < 100000.0) {
    return 2;
  } else {
    return 3;
  }
}

float GetAverageShadowFactor(sampler2D shadow_map, vec3 light_space_position) {
  const vec2 texel_size = 1.0 / vec2(2048.0);
  const float spread = 2.0;

  float t = fract(running_time);

  const vec2[] offsets = {
    vec2(0.0),
    vec2(noise(1.0 + t), noise(2.0 + t)),
    vec2(noise(3.0 + t), noise(4.0 + t)),
    vec2(noise(5.0 + t), noise(6.0 + t)),
    vec2(noise(7.0 + t), noise(8.0 + t)),
  };

  const float bias = 0.001;
  float shadow_factor = 0.0;

  for (int i = 0; i < 5; i++) {
    vec4 shadow_sample = texture(shadow_map, light_space_position.xy + spread * offsets[i] * texel_size);

    if (shadow_sample.r >= light_space_position.z - bias) {
      shadow_factor += 1.0;
    }
  }

  return shadow_factor / 5.0;
}

float GetPrimaryLightShadowFactor(vec3 world_position, float depth) {
  int cascade_index = GetCascadeIndex(depth);
  vec4 light_space_position = light_matrices[cascade_index] * vec4(world_position, 1.0);

  light_space_position.xyz /= light_space_position.w;
  light_space_position.xyz *= 0.5;
  light_space_position.xyz += 0.5;

  if (light_space_position.z > 0.999) {
    // If the position-to-light space transform depth
    // is out of range, we've sampled outside the
    // shadow map and can just render the fragment
    // with full illumination.
    return 1.0;
  }

  return GetAverageShadowFactor(
    cascade_index == 0 ? in_shadow_map_cascade_1 :
    cascade_index == 1 ? in_shadow_map_cascade_2 :
    cascade_index == 2 ? in_shadow_map_cascade_3 :
    in_shadow_map_cascade_4,
    light_space_position.xyz
  );
}

vec3 GetAmbientFresnel(float NdotV) {
  return 0.02 * vec3(pow(1.0 - NdotV, 5.0));
}

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
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

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float simplex_noise(vec2 v) {
  // Precompute values for skewed triangular grid
  const vec4 C = vec4(0.211324865405187,
                      // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,
                      // 0.5*(sqrt(3.0)-1.0)
                      -0.577350269189626,
                      // -1.0 + 2.0 * C.x
                      0.024390243902439);
                      // 1.0 / 41.0

  // First corner (x0)
  vec2 i  = floor(v + dot(v, C.yy));
  vec2 x0 = v - i + dot(i, C.xx);

  // Other two corners (x1, x2)
  vec2 i1 = vec2(0.0);
  i1 = (x0.x > x0.y)? vec2(1.0, 0.0):vec2(0.0, 1.0);
  vec2 x1 = x0.xy + C.xx - i1;
  vec2 x2 = x0.xy + C.zz;

  // Do some permutations to avoid
  // truncation effects in permutation
  i = mod289(i);
  vec3 p = permute(
          permute( i.y + vec3(0.0, i1.y, 1.0))
              + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(
                      dot(x0,x0),
                      dot(x1,x1),
                      dot(x2,x2)
                      ), 0.0);

  m = m*m ;
  m = m*m ;

  // Gradients:
  //  41 pts uniformly over a line, mapped onto a diamond
  //  The ring size 17*17 = 289 is close to a multiple
  //      of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

  // Normalise gradients implicitly by scaling m
  // Approximation of: m *= inversesqrt(a0*a0 + h*h);
  m *= 1.79284291400159 - 0.85373472095314 * (a0*a0+h*h);

  // Compute final noise value at P
  vec3 g = vec3(0.0);
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * vec2(x1.x,x2.x) + h.yz * vec2(x1.y,x2.y);
  return 130.0 * dot(m, g);
}

vec3 RotateAroundAxis(vec3 axis, vec3 vector, float angle) {
  vec4 q;
  float sa = sin(angle / 2.0);

  q.x = axis.x * sa;
  q.y = axis.y * sa;
  q.z = axis.z * sa;
  q.w = cos(angle / 2.0);

  return vector + 2.0 * cross(cross(vector, q.xyz) + q.w * vector, q.xyz);
}

const vec3 ORBITAL_AXIS = normalize(vec3(0.5, 0, -1.0));

// @todo allow game-specific definitions
vec3 GetSkyColor(vec3 sky_direction) {
  vec3 sun_direction = -directional_light_direction;
  vec3 planet_direction = vec3(0, -1, 0);

  float sun_dot = max(dot(sun_direction, sky_direction), 0.0);
  vec3 sun_base_color = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 0.95, 0.9), pow(sun_dot, 100));
  float sun_alpha = clamp(pow(sun_dot, 250) * 2.0, 0.0, 1.1);
  vec3 sun_color = mix(vec3(0), sun_base_color, sun_alpha);

  float planet_dot = max(dot(planet_direction, sky_direction), 0.0);
  vec3 planet_atmosphere_base_color = mix(vec3(0.0, 0.1, 1.0), vec3(0.5, 0.7, 0.9), pow(planet_dot, 20));
  planet_atmosphere_base_color = mix(planet_atmosphere_base_color, vec3(1, 0.9, 0.6), pow(sun_dot, 20));
  float planet_atmosphere_sunlight_factor = 0.2 + 0.8 * pow(sun_dot, 10);
  float planet_atmosphere_alpha = clamp(pow(planet_dot, 40) * 40.0, 0, 1) * planet_atmosphere_sunlight_factor;
  vec3 planet_atmosphere_color = mix(vec3(0), planet_atmosphere_base_color, planet_atmosphere_alpha);

  // @todo cleanup
  vec3 bg_direction = RotateAroundAxis(ORBITAL_AXIS, sky_direction, scene_time * 0.001);

  float bg_noise = simplex_noise((sky_direction.xy + sky_direction.yz) * 1.0);
  bg_noise = clamp(bg_noise, 0.0, 1.0);

  float bg_noise2 = simplex_noise(sky_direction.zx * sky_direction.zy + sky_direction.zz);
  bg_noise2 = clamp(bg_noise2, 0.0, 1.0);

  float bg_noise3 = simplex_noise((sky_direction.xy * sky_direction.zy + sky_direction.xz) * vec2(1.0, 3.0));
  bg_noise3 = clamp(bg_noise3, 0.0, 1.0);

  vec3 space_color = vec3(0);

  space_color += vec3(bg_noise) * mix(vec3(1.0), vec3(0.0, 0.5, 1.0), bg_noise) * 0.1;
  space_color += vec3(bg_noise2) * vec3(0.0, 0.75, 1.0) * 0.1;
  space_color += vec3(bg_noise3) * vec3(1.0, 0.2, 0.6) * 0.1 * (2.0 * pow(abs(sky_direction.y), 3.0));

  float earth_dot = max(0.0, dot(sky_direction, vec3(0, -1.0, 0)));
  // Increase space color intensity near the earth/sun
  space_color *= 1.0 + pow(earth_dot, 5.0) + pow(sun_dot, 10.0);
  // Avoid awkward color bleeding between space/sun color
  space_color = mix(space_color, vec3(0.0), pow(sun_dot, 50.0));

  float stars_x = atan(bg_direction.x, bg_direction.z) + 3.141592;
  float stars_y = atan(length(bg_direction.xz), bg_direction.y);

  float stars_noise =
    simplex_noise(vec2(stars_x, stars_y) * 100.0) *
    simplex_noise(vec2(stars_x, stars_y) * 60.0);

  stars_noise = clamp(stars_noise, 0.0, 1.0);
  stars_noise = pow(stars_noise, 20.0) * 500.0;
  vec3 stars_color = vec3(stars_noise);

  vec3 sky_color = planet_atmosphere_color + sun_color + space_color + stars_color;

  sky_color.x = clamp(sky_color.x, 0.0, 1.0);
  sky_color.y = clamp(sky_color.y, 0.0, 1.0);
  sky_color.z = clamp(sky_color.z, 0.0, 1.0);

  return sky_color;
}

vec3 GetReflectionColor(vec3 R) {
  return GetSkyColor(R);
}

const vec3[] ssao_sample_points = {
  vec3(0.021429, 0.059112, 0.07776),
  vec3(0.042287, -0.020052, 0.092332),
  vec3(0.087243, -0.025947, 0.068744),
  vec3(-0.001496, -0.027043, 0.128824),
  vec3(-0.088486, -0.099508, 0.081747),
  vec3(0.108582, 0.02973, 0.15043),
  vec3(-0.131723, 0.143868, 0.115246),
  vec3(-0.137142, -0.089451, 0.21753),
  vec3(-0.225405, 0.214709, 0.09337),
  vec3(-0.152747, 0.129375, 0.328595),
  vec3(0.155238, 0.146029, 0.398102),
  vec3(-0.205277, 0.358875, 0.3242),
  vec3(0.129059, 0.579216, 0.124064),
  vec3(-0.427557, -0.123908, 0.53261),
  vec3(0.661657, -0.296235, 0.311568),
  vec3(0.175674, 0.013574, 0.87342)
};

float GetSSAO(int total_samples, float depth, vec3 position, vec3 normal, float seed) {
  float linear_depth = GetLinearDepth(depth, 500.0, 10000000.0);
  float radius = mix(500.0, 500000.0, linear_depth);
  float ssao = 0.0;

  vec3 random_vector = vec3(noise(1.0 + seed), noise(2.0 + seed), noise(3.0 + seed));
  vec3 tangent = normalize(random_vector - normal * dot(random_vector, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 tbn = mat3(tangent, bitangent, normal);

  for (int i = 0; i < total_samples; i++) {
    vec3 tbn_sample_position = tbn * ssao_sample_points[i];
    vec3 world_sample_position = position + tbn_sample_position * radius;
    vec3 view_sample_position = (view_matrix * vec4(world_sample_position, 1.0)).xyz;
    vec2 screen_sample_uv = GetScreenCoordinates(view_sample_position, projection_matrix);
    // float sample_depth = textureLod(texColorAndDepth, screen_sample_uv, 1).w;
    float sample_depth = texture(in_normal_and_depth, screen_sample_uv).w;
    float world_depth = GetWorldDepth(sample_depth, 500.0, 10000000.0);

    if (world_depth < -view_sample_position.z) {
      float occluder_distance = -view_sample_position.z - world_depth;
      float occlusion_factor = mix(1.0, 0.0, saturate(occluder_distance / (0.5 * radius)));

      ssao += occlusion_factor;
    }
  }

  const float near_ssao = 0.5;
  const float far_ssao = 0.6;

  float ssao_intensity = mix(near_ssao, far_ssao, pow(depth, 30.0));
  ssao_intensity = mix(ssao_intensity, 0.0, pow(depth, 1024.0));

  return ssao / float(total_samples) * ssao_intensity;
}

vec2 GetDenoisedTemporalData(float ssao, float shadow, float depth, vec2 temporal_uv) {
  #define USE_SPATIAL_DENOISING 1

  if (temporal_uv.x < 0.0 || temporal_uv.x > 1.0 || temporal_uv.y < 0.0 || temporal_uv.y > 1.0) {
    return vec2(0.0, 0.0);
  }

  // @todo use screen size
  const vec2 texel_size = 1.0 / vec2(1920.0, 1080.0);
  const float temporal_weight = 1.0;
  const float spatial_spread = 2.0;

  float world_depth = GetWorldDepth(depth, 500.0, 10000000.0);

  // @hack don't use the temporal UV coordinates for
  // objects close to the camera, e.g. the player ship.
  // This eliminates smearing/ghosting.
  if (world_depth < 2500.0) {
    temporal_uv = fragUv;
  }

  #if USE_SPATIAL_DENOISING
    const vec2[] offsets = {
      vec2(1.0, 0),
      vec2(-1.0, 0),
      vec2(0, 1.0),
      vec2(0, -1.0)
    };

    float base_ssao = ssao;
    float base_shadow = shadow;

    for (int i = 0; i < 4; i++) {
      vec2 offset = spatial_spread * offsets[i];
      vec4 temporal_data = texture(in_temporal_data, temporal_uv + texel_size * offset);
      float temporal_depth = temporal_data.w;
      float depth_distance = abs(world_depth - GetWorldDepth(temporal_depth, 500.0, 10000000.0));

      if (depth_distance < 500.0) {
        ssao += temporal_data.x * temporal_weight;
        shadow += temporal_data.y * temporal_weight;
      } else {
        ssao += base_ssao * temporal_weight;
        shadow += base_shadow * temporal_weight;
      }
    }

    return vec2(ssao, shadow) / (temporal_weight * 4.0 + 1.0);
  #else
    vec4 temporal_data = texture(in_temporal_data, temporal_uv);

    ssao += temporal_data.x * temporal_weight;
    shadow += temporal_data.y * temporal_weight;

    return vec2(ssao, shadow) / (temporal_weight + 1.0);
  #endif
}

void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 D = normalize(position - camera_position);

  if (frag_normal_and_depth.w == 1.0) {
    out_color_and_depth = vec4(GetSkyColor(D), frag_normal_and_depth.w);
    out_temporal_data = vec4(0, 0, 0, 1.0);

    return;
  }

  uvec4 frag_color_and_material = texture(in_color_and_material, fragUv);

  vec3 N = frag_normal_and_depth.xyz;
  vec4 color = UnpackColor(frag_color_and_material);
  vec3 albedo = color.rgb;
  float emissive = color.a;
  Material material = UnpackMaterial(frag_color_and_material);

  vec3 V = normalize(camera_position - position);
  vec3 L = normalize(directional_light_direction);

  float NdotV = max(dot(N, V), 0.0);

  float roughness = material.roughness;
  float metalness = material.metalness;
  float clearcoat = material.clearcoat;
  float subsurface = material.subsurface;

  if (roughness < 0.05) roughness = 0.05;

  // Temporal data
  vec3 previous_view_position = (previous_view_matrix * vec4(position, 1.0)).xyz;
  vec2 temporal_uv = GetScreenCoordinates(previous_view_position, projection_matrix);
  vec4 last_temporal_sample = texture(in_temporal_data, temporal_uv);

  // Denoised SSAO/shadow
  float ssao = GetSSAO(12, frag_normal_and_depth.w, position, frag_normal_and_depth.xyz, fract(running_time));
  float shadow = GetPrimaryLightShadowFactor(position, frag_normal_and_depth.w);
  vec2 denoised_temporal_data = GetDenoisedTemporalData(ssao, shadow, frag_normal_and_depth.w, temporal_uv);

  ssao = denoised_temporal_data.x;
  shadow = denoised_temporal_data.y;

  ssao = clamp(ssao, 0.0, 1.0);
  shadow = clamp(shadow, 0.0, 1.0);

  vec3 out_color = vec3(0.0);

  // Primary directional light
  {
    const vec3 primary_light_color = vec3(1.0);

    out_color += GetDirectionalLightRadiance(L, primary_light_color, albedo, position, N, V, NdotV, roughness, metalness, clearcoat, subsurface, shadow);
  }

  // Anti-light (for improved visibility in dark areas)
  {
    const vec3 light_color = vec3(0.2, 0.3, 1.0);
    float depth_input = max(0.99, frag_normal_and_depth.w);
    vec3 direction = RotateAroundAxis(ORBITAL_AXIS, L, 3.141592 * 0.6);
    float intensity = 0.25 * (1.0 - pow(depth_input, 200.0));

    out_color += GetDirectionalLightRadiance(direction, light_color * intensity, albedo, position, N, V, NdotV, 1.0, metalness, 0.0, subsurface, 1.0);
  }

  // Earth bounce light
  // @todo make customizable
  {
    const vec3 earth_light_direction = vec3(0, 1.0, 0);
    const vec3 earth_light_color = vec3(0.2, 0.5, 1.0) * 0.2;
 
    out_color += GetDirectionalLightRadiance(earth_light_direction, earth_light_color, albedo, position, N, V, NdotV, mix(roughness, 1.0, 0.5), metalness, 0.0, 0.0, 1.0);
  }

  // Ambient light (based on the primary directional light)
  // @todo cleanup
  {
    float NdotL = max(dot(N, L), 0.0);

    out_color += albedo * vec3(0.1, 0.2, 0.5) * (0.005 + 0.02 * (1.0 - NdotL));
  }

  // Reflections
  {
    vec3 R = reflect(-V, N);
    vec3 reflection = GetReflectionColor(R);

    out_color += albedo * reflection * metalness * (1.0 - roughness) * 0.2;
  }

  // @todo dev mode only
  if (use_high_visibility_mode) {
    out_color = albedo * pow(NdotV, 2.0);
  }

  out_color += GetAmbientFresnel(NdotV);
  out_color = mix(out_color, albedo, pow(emissive, 1.5));

  if (frag_normal_and_depth.w >= 1.0) out_color = vec3(0);

  // @todo move to post shader
  float exposure = 1.5 + 4.0 * emissive;

  out_color = vec3(1.0) - exp(-out_color * exposure);
  out_color = pow(out_color, vec3(1.0 / 2.2));

  out_color -= ssao;

  out_color_and_depth = vec4(out_color, frag_normal_and_depth.w);
  out_temporal_data = vec4(ssao, shadow, 0, frag_normal_and_depth.w);
}