#version 460 core

// in vec2 fragUv;

in vec3 fragWorldPosition;
in vec3 fragNormal;
in vec3 fragTangent;
// in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_normal_and_depth;
layout (location = 1) out uvec4 out_color_and_material;

void main() {
  out_normal_and_depth = vec4(normalize(fragNormal), gl_FragCoord.z);
  out_color_and_material = uvec4(0xFF, 0xF0, 0xF0, 0x00);
}