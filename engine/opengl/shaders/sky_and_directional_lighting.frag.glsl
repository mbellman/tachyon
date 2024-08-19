#version 460 core

uniform sampler2D in_color_and_depth;
uniform sampler2D in_normal_and_material;

in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  vec4 frag_color_and_depth = texture(in_color_and_depth, fragUv);
  vec4 frag_normal_and_material = texture(in_normal_and_material, fragUv);

  out_color_and_depth = vec4(frag_color_and_depth.rgb, frag_color_and_depth.w);
}