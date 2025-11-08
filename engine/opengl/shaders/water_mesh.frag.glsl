#version 460 core

uniform vec3 camera_position;
uniform vec3 primary_light_direction;
uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragPosition;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

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

void main() {
  vec3 N = normalize(fragNormal);

  // @temporary
  float water_speed = scene_time;
  float ripple_speed = 2.0 * scene_time;
  float big_wave = sin(fragPosition.z * 0.0002 + fragPosition.x * 0.0002 + ripple_speed);
  float small_wave = sin(fragPosition.z * 0.001 + fragPosition.x * 0.001 + ripple_speed);
  float tiny_wave = sin(fragPosition.z * 0.01 + 3.0 * ripple_speed);

  N.z += 0.2 * sin(fragPosition.z * 0.001 - fragPosition.x * 0.001 + water_speed);
  N.x += 0.2 * cos(fragPosition.z * 0.005 - fragPosition.x * 0.002 + water_speed + small_wave);
  N.x += 0.1 * sin(fragPosition.z * 0.001 - fragPosition.x * 0.005 + water_speed + tiny_wave);
  N = normalize(N);

  vec3 V = normalize(camera_position - fragPosition);
  vec3 D = -V;
  vec3 L = normalize(-primary_light_direction);
  vec3 R = reflect(D, N);

  float NdotV = max(0.0, dot(N, V));
  float NdotL = max(0.0, dot(N, L));
  float DdotL = max(0.0, dot(D, L));

  vec3 out_color = vec3(0.0);

  const vec3 base_water_color = vec3(0, 0.1, 0.2);

  // @todo refine
  vec3 reflection_color = vec3(0);
  reflection_color += GetReflectionColor(R);
  reflection_color += 5.0 * pow(max(0.0, dot(R, L)), 5.0);

  // @todo refine
  float fresnel_factor = pow(max(0.0, dot(R, -V)), 4.0);

  out_color = mix(base_water_color, reflection_color, fresnel_factor);
  out_color = mix(out_color, vec3(0.4), 0.2);


  out_color_and_depth = vec4(out_color, gl_FragCoord.z);
}