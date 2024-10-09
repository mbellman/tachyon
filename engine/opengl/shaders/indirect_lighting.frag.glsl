#version 460 core

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
uniform sampler2D in_accumulation;
// uniform vec3 camera_position;

noperspective in vec2 fragUv;

layout (location = 0) out vec3 out_color;

void main() {
  vec4 normal_and_depth = texture(in_normal_and_depth, fragUv);
  vec3 color = texture(in_accumulation, fragUv).xyz;

  out_color = color;
}