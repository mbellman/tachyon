#version 460 core

uniform vec3 camera_position;
uniform vec3 primary_light_direction;
uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragPosition;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

float permute(float x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
{
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

  return p;
}

// (sqrt(5) - 1)/4 = F4, used once below
#define F4 0.309016994374947451

float snoise(vec4 v) {
  const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                        0.276393202250021,  // 2 * G4
                        0.414589803375032,  // 3 * G4
                       -0.447213595499958); // -1 + 4 * G4

// First corner
  vec4 i  = floor(v + dot(v, vec4(F4)) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;
  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;
//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;
  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C.xxxx
  //  x1 = x0 - i1  + 1.0 * C.xxxx
  //  x2 = x0 - i2  + 2.0 * C.xxxx
  //  x3 = x0 - i3  + 3.0 * C.xxxx
  //  x4 = x0 - 1.0 + 4.0 * C.xxxx
  vec4 x1 = x0 - i1 + C.xxxx;
  vec4 x2 = x0 - i2 + C.yyyy;
  vec4 x3 = x0 - i3 + C.zzzz;
  vec4 x4 = x0 + C.wwww;

// Permutations
  i = mod289(i);
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

// Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.
  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

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

vec3 GetCloudDirection(vec3 normal) {
  const vec3 orbit_rotation_axis = normalize(vec3(0.5, 0, -1.0));

  vec2 p = normal.xz;
  float z = 1.0 - length(p);
  vec3 v = vec3(p, -sqrt(z));
  v *= 8.0;

  return RotateAroundAxis(orbit_rotation_axis, vec3(v.x, -v.z, v.y), scene_time * 0.001);
}

void main() {
  vec3 N = normalize(fragNormal);
  vec3 V = normalize(camera_position - fragPosition);
  vec3 D = -V;
  vec3 L = normalize(-primary_light_direction);

  float NdotV = max(0.0, dot(N, V));
  float NdotL = max(0.0, dot(N, L));
  float DdotL = max(0.0, dot(D, L));

  vec3 out_color = vec3(0.0);

  // Clouds
  vec3 d = GetCloudDirection(N);

  float t = scene_time * 0.01;
  float c1 = snoise(vec4(d, t));
  float c2 = snoise(vec4(d * 2.0, t));
  float c3 = snoise(vec4(d * 4.0, t));
  float c4 = snoise(vec4(d * 8.0, t));
  float c5 = snoise(vec4(d * 24.0, t));

  float clouds = c1 + c2 + c3 + c4 + c5;

  float density = clamp(
    sin(N.x * 1.5 + N.z * 2.0 + N.y * 2.5) * clouds,
    0.2,
    1.0
  );

  clouds = clamp(clouds, 0.0, 1.0) * density;

  // Cloud shadows
  vec3 d2 = GetCloudDirection(N * 1.02);
  float c3_2 = snoise(vec4(d2 * 2.0, t));
  float c4_2 = snoise(vec4(d2 * 8.0, t));
  c3_2 = clamp(c3_2, 0.0, 1.0);
  c4_2 = clamp(c4_2, 0.0, 1.0);

  float shadow = (c3_2 + c4_2) * (1.0 - NdotV * 0.75);

  // Our goal here is to have a rim of light which gradually intensifies
  // as we move toward the glancing edge of the atmosphere, and then rapidly
  // falls off.
  //
  // edge_alpha is a range over [0 - 1] representing progress toward the glancing edge.
  //
  // edge_falloff_threshold is the point within the range at which the light intensity
  // should peak, and then start falling off.
  float edge_alpha = 1.0 - NdotV;
  float edge_falloff_threshold = 0.725;

  // We use this to recover a [0 - 1] blend value for the region past
  // the falloff threshold.
  float edge_falloff_threshold_multiplier = 1.0 / (1.0 - edge_falloff_threshold);

  float edge_factor =
    edge_alpha < edge_falloff_threshold
      ? mix(0.0, 1.0, edge_alpha / edge_falloff_threshold)
      : mix(1.0, 0.0, edge_falloff_threshold_multiplier * (edge_alpha - edge_falloff_threshold));

  // Exponentially dampen the light intensity
  edge_factor *= edge_factor;
  edge_factor *= edge_factor;

  if (edge_alpha < edge_falloff_threshold) {
    edge_factor *= edge_factor;
    edge_factor *= edge_factor;
  }

  // Define a binary 1.0/0.0 factor for being inside or outside the planetary silhouette
  float inside_edge_factor =
    edge_alpha < edge_falloff_threshold
      ? 1.0
      : 0.0;

  // Clouds + cloud shadows
  out_color += vec3(clouds);
  out_color -= vec3(shadow) * 0.6 * density;

  // Incidence
  out_color *= (0.2 + 0.8 * pow(NdotL, 1.0 / 3.0));

  // Nighttime side
  out_color += mix(vec3(0.0), vec3(-0.8, -0.6, -0.1), pow(1.0 - NdotL, 10.0)) * inside_edge_factor;

  // Color adjustment
  out_color -= vec3(0.2, 0.1, 0.0);

  // Fade out clouds near the horizon
  out_color = mix(out_color, vec3(0.0), pow(1.0 - NdotV * NdotV, 2.0));

  // Edge scattering
  // Blend between blue/orange depending on how close the sun is to this pixel
  vec3 edge_color = mix(
    vec3(0.4, 0.7, 1.0),
    vec3(1.0, 0.8, 0.1),
    pow(DdotL, 2.0)
  );

  // Add a copper/reddish tint near the light/dark boundary
  edge_color = mix(edge_color, vec3(1.0, 0.6, 0.4), 1.0 - abs(dot(N, L)));

  // Reduce the light intensity on the far side of the atmosphere from the sun
  float sunlight_factor = 1.0 - abs(dot(N, L));

  out_color += edge_color * edge_factor * sunlight_factor;

  // Haze
  out_color += vec3(0.4) * (1.0 - NdotV) * inside_edge_factor;

  // Sunrise/sunset
  out_color +=
    5.0 *
    (vec3(1.0, 0.5, 0.2) + 2.0 * clouds) *
    pow(DdotL, 50.0) *
    pow(1.0 - NdotV, 2.0) *
    pow(1.0 - NdotL, 10.0) *
    edge_factor;

  out_color_and_depth = vec4(out_color, 0);
}