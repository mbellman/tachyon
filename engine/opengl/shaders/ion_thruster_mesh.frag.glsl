#version 460 core

uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
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
  vec4 out_color = UnpackColor(fragSurface);
  const float TAU = 2.0 * 3.141592;
  float t = fract(scene_time);

  out_color *= 1.0 + 0.02 * sin(TAU * (vertPosition.z * 5.0 - t * 4.0));

  float up_dot = abs(dot(fragNormal, upDirection));
  float intensity = 0.8 + 0.2 * sin(TAU * (2.0 * t + 2.0 * up_dot));

  out_color.rgb = mix(out_color.rgb, vec3(1.0), 0.15 * intensity);

  out_color.rgb *= 5.0 * intensity * clamp(pow(1.0 / (vertPosition.z * 1.3), 15.0), 0.0, 1.0);
  out_color.rgb *= out_color.w;
  out_color.rgb *= out_color.w;
  out_color.rgb *= out_color.w;

  out_color_and_depth = vec4(out_color.rgb, 0.0);
}