#version 460 core

uniform vec3 camera_position;
uniform vec3 primary_light_direction;
uniform float scene_time;

// @todo not the correct name; this is in_normal_and_depth
uniform sampler2D previous_color_and_depth;

flat in uvec4 fragSurface;
in vec3 fragPosition;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

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

vec3 GetSkyColor(vec3 sky_direction, float sun_glare_factor) {
  float up_dot = 0.5 + 0.5 * max(0.0, dot(sky_direction, vec3(0, 1.0, 0)));

  return normalize(vec3(
    sqrt(1.0 - up_dot),
    0.2,
    pow(up_dot, 2.0)
  ));
}

vec3 GetReflectionColor(vec3 R) {
  return GetSkyColor(R, 0.0);
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

void main() {
  vec3 N = normalize(fragNormal);

  // @temporary
  float water_speed = scene_time;
  float ripple_speed = 2.0 * scene_time;
  float big_wave = sin(fragPosition.z * 0.0002 + fragPosition.x * 0.0002 + ripple_speed);
  float small_wave = sin(fragPosition.z * 0.001 + fragPosition.x * 0.001 + ripple_speed);

  // Directional waves
  N.z += 0.1 * sin(fragPosition.z * 0.001 - fragPosition.x * 0.001 + water_speed + big_wave);
  N.x += 0.1 * cos(fragPosition.z * 0.0025 - fragPosition.x * 0.001 + water_speed + small_wave);

  float wx = fragPosition.x;
  float wz = fragPosition.z;

  // Noise/turbulence
  N.xz += 0.1 * vec2(simplex_noise(vec2(scene_time * 0.5 + wx * 0.0005, scene_time * 0.5 + wz * 0.0005)));
  N.xz += 0.1 * vec2(simplex_noise(vec2(scene_time * 0.5 + wx * 0.002, scene_time * 0.5 + wz * 0.002)));
  N.xz *= 0.2;

  N = normalize(N);

  vec3 V = normalize(camera_position - fragPosition);
  vec3 D = -V;
  vec3 L = normalize(-primary_light_direction);
  vec3 R = reflect(D, N);

  float NdotV = max(0.0, dot(N, V));
  float NdotL = max(0.0, dot(N, L));
  float DdotL = max(0.0, dot(D, L));

  vec3 out_color = vec3(0.0);

  const vec3 base_water_color = vec3(0, 0.05, 0.2);
  const vec3 base_underwater_color = vec3(0.2, 0.3, 0.6);

  // Sample objects beneath the surface of the water.
  // For now we simply sample depth and fade to a light
  // blue color near the surface.
  {
    // @todo pass in as a uniform
    const vec2 resolution = vec2(2560.0, 1440.0);

    vec2 sample_coordinates = gl_FragCoord.xy;

    // @todo make wave intensity proportional to distance
    sample_coordinates.x += 5.0 * sin(fragPosition.z * 0.005 + scene_time);

    vec2 sample_uv = 2.0 * sample_coordinates / resolution - 1.0;
    sample_uv *= 0.5;
    sample_uv += 0.5;

    float water_surface_z = GetWorldDepth(gl_FragCoord.z, Z_NEAR, Z_FAR);
    float underwater_sample_z = texture(previous_color_and_depth, sample_uv).w;
    underwater_sample_z = GetWorldDepth(underwater_sample_z, Z_NEAR, Z_FAR);

    float underwater_visibility = clamp((underwater_sample_z - water_surface_z) / 2000.0, 0.0, 1.0);
    underwater_visibility = 1.0 - underwater_visibility;
    underwater_visibility *= underwater_visibility;

    // Prevent objects above the water from being sampled
    if (underwater_sample_z < water_surface_z) underwater_visibility = 0.0;

    out_color = mix(base_water_color, base_underwater_color, underwater_visibility);
  }

  // Reflect sky + primary light
  {
    // @todo refine
    vec3 reflection_color = vec3(0);

    // Sky reflection
    reflection_color += GetReflectionColor(R);

    // Light reflection
    reflection_color += 5.0 * pow(max(0.0, dot(R, L)), 5.0);

    // @todo refine
    float fresnel_factor = pow(max(0.0, dot(R, -V)), 2.0);

    out_color = mix(out_color, reflection_color, fresnel_factor);
    out_color = mix(out_color, vec3(0.4), 0.2);
  }

  // @todo fog
  {
    // float visibility_range = 40000.0;
    // float frag_distance_from_camera = length(fragPosition - camera_position);
    // float fog_thickness = clamp(frag_distance_from_camera / visibility_range, 0.0, 1.0);
    // out_color = mix(out_color, vec3(0.2, 0.2, 0.4), fog_thickness);
  }

  out_color_and_depth = vec4(out_color, gl_FragCoord.z);
}