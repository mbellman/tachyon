#version 460 core

uniform vec3 camera_position;
uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragPosition;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

void main() {
  vec3 N = normalize(fragNormal);
  vec3 V = normalize(camera_position - fragPosition);
  vec3 D = -V;

  float NdotV = max(0.0, dot(N, V));

  vec3 object_color = UnpackColor(fragSurface).rgb;
  vec3 hot_color = object_color * 3.0;
  hot_color.r = clamp(hot_color.r, 0.0, 1.0);
  hot_color.g = clamp(hot_color.g, 0.0, 1.0);
  hot_color.b = clamp(hot_color.b, 0.0, 1.0);
  vec3 fire_color = mix(object_color.rgb, hot_color, pow(NdotV, 10.0));

  vec3 out_color = fire_color * pow(NdotV, 5.0) * 2.0;

  out_color_and_depth = vec4(out_color, 0);
}