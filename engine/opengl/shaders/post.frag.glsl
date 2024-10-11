#version 460 core

uniform sampler2D in_color_and_depth;

in vec2 fragUv;

layout (location = 0) out vec4 out_color;

void main() {
  vec4 color_and_depth = texture(in_color_and_depth, fragUv);

  out_color = color_and_depth;
}