#version 460 core

flat in vec3 fragColor;
flat in vec4 fragMaterial;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_normal_and_depth;
layout (location = 1) out vec4 out_color_and_material;

void main() {
  vec3 color = fragColor;

  out_normal_and_depth = vec4(normalize(fragNormal), gl_FragCoord.z);
  // @todo handle material
  out_color_and_material = vec4(color.rgb, 0.0);
}