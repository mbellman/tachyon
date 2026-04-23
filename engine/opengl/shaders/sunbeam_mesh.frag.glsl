#version 460 core

uniform vec3 camera_position;
uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragPosition;
in vec3 vertPosition;

flat in vec3 topPosition;
flat in vec3 upDirection;

layout (location = 0) out vec3 out_color;

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

void main() {
  // @todo base on the object color
  vec3 sunbeam_color = vec3(1.0, 0.7, 0.2);

  vec3 camera_to_fragment = fragPosition - camera_position;
  float camera_distance = length(camera_to_fragment);

  vec3 N = normalize(fragNormal);
  vec3 V = camera_to_fragment / camera_distance;
  float NdotV = max(0.0, dot(N, -V));

  float distance_from_top = distance(fragPosition, topPosition);
  float phase_variance = 2.0 * sin(5.0 * acos(dot(N, vec3(0, 0, 1))));
  float phase_alpha = 0.002 * distance_from_top - 2.0 * scene_time + phase_variance;

  float vertical_phase = 1.0 + 0.2 * sin(phase_alpha);
  float horizontal_phase = 1.0 + 0.2 * sin(scene_time + 6.0 * atan(N.z, N.x));

  float oscillation_alpha = 1.5 * scene_time;
  float oscillation = 0.8 + 0.2 * sin(oscillation_alpha + 0.001 * topPosition.x);

  float top_falloff = clamp(1.0 - distance_from_top / 16000.0, 0, 1);
  float edge_falloff = pow(NdotV, 4.0);

  float distance_falloff = clamp(camera_distance / 6000.0, 0, 1);
  distance_falloff *= distance_falloff;
  distance_falloff *= distance_falloff;

  float alpha = (
    top_falloff *
    edge_falloff *
    distance_falloff *
    oscillation *
    vertical_phase *
    horizontal_phase
  );

  out_color = mix(vec3(0), sunbeam_color, alpha);
}