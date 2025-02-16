#version 460 core

uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 vertPosition;

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

  out_color *= (1.0 + 0.1 * sin(vertPosition.z * 20.0 - scene_time * 5.0));

  out_color.rgb *= 5.0 * clamp(pow(1.0 / (vertPosition.z * 1.5), 12.0), 0.0, 1.0);
  out_color.rgb *= out_color.w;
  out_color.rgb *= out_color.w;

  out_color_and_depth = vec4(out_color.rgb, 0.0);
}