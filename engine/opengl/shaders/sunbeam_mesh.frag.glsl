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

layout (location = 0) out vec4 out_color_and_depth;

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

void main() {
  vec3 out_color = vec3(0.0);

  vec3 N = normalize(fragNormal);
  vec3 V = normalize((fragPosition - camera_position) * vec3(1, 0, 1));

  float distance_from_top = distance(fragPosition, topPosition);
  float phase_rotation = sin(4.0 * acos(dot(N, vec3(0, 0, 1))));
  float phase_alpha = 0.005 * distance_from_top - 2.0 * scene_time + phase_rotation;
  float phase = 0.8 + 0.2 * sin(phase_alpha);

  vec3 light_color = vec3(1.0, 0.7, 0.2);
  float oscillation_alpha = 1.5 * scene_time;
  float oscillation = 0.8 + 0.2 * sin(oscillation_alpha);
  vec3 beam_color = light_color * oscillation * phase;

  float top_falloff = clamp(1.0 - distance_from_top / 5000.0, 0, 1);
  float edge_falloff = pow(dot(N, -V), 2.0);
  float alpha = clamp(top_falloff * edge_falloff, 0, 1);

  out_color = mix(vec3(0), beam_color, alpha);

  out_color_and_depth = vec4(out_color, 0.0);
}