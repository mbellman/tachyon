#version 460 core

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  vec3 color = fragColor;

  out_color_and_depth = vec4(color.rgb, gl_FragCoord.z);  
}