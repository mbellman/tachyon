#version 460 core

uniform int debugView;

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_material;

void main() {
  vec3 color = fragColor;

  // @todo do this instead as a code path tweak in the renderer,
  // skipping lighting/post steps whenever the debug view is != 0
  if (debugView == 0) {
    out_color_and_depth = vec4(color.rgb, gl_FragCoord.z);
  } else if (debugView == 1) {
    out_color_and_depth = vec4(fragNormal, gl_FragCoord.z);
  } else if (debugView == 2) {
    // @todo use a better depth gradient
    out_color_and_depth = vec4(vec3(gl_FragCoord.z), gl_FragCoord.z);
  } else if (debugView == 3) {
    out_color_and_depth = vec4(vec3(1), gl_FragCoord.z);
  }

  out_normal_and_material = vec4(normalize(fragNormal), 1.0);
}