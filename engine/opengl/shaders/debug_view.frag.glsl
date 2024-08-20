#version 460 core

uniform sampler2D in_color_and_depth;
uniform sampler2D in_normal_and_material;

in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  vec4 frag_color_and_depth = texture(in_color_and_depth, fragUv);
  vec4 frag_normal_and_material = texture(in_normal_and_material, fragUv);

  // out_color_and_depth = vec4(frag_normal_and_material.rgb, 1.0);

  if (fragUv.x < 0.5 && fragUv.y >= 0.5) {
    // Top left (color)
    vec3 frag_color = texture(in_color_and_depth, 2.0 * (fragUv - vec2(0, 0.5))).rgb;

    out_color_and_depth = vec4(frag_color, 1.0);
  } else if (fragUv.x >= 0.5 && fragUv.y >= 0.5) {
    // Top right (depth)
    float frag_depth = texture(in_color_and_depth, 2.0 * (fragUv - vec2(0.5, 0.5))).w;
    frag_depth = 1.0 - pow(frag_depth, 50);

    out_color_and_depth = vec4(vec3(frag_depth), 1.0);
  } else if (fragUv.x < 0.5 && fragUv.y < 0.5) {
    // Bottom left (normals)
    vec3 normals = texture(in_normal_and_material, fragUv * 1.999).rgb;

    out_color_and_depth = vec4(normals, 1.0);
  } else {
    // Bottom right (@todo material)
    out_color_and_depth = vec4(vec3(0), 1.0);
  }
}