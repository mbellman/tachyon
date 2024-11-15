#version 460 core

flat in uvec4 fragSurface;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  out_color_and_depth = vec4(1.0);
}