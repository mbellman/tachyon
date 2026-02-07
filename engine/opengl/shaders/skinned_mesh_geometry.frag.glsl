#version 460 core

uniform bool has_texture;
uniform sampler2D albedo_texture;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_normal_and_depth;
layout (location = 1) out uvec4 out_color_and_material;

void main() {
  out_normal_and_depth = vec4(normalize(fragNormal), gl_FragCoord.z);
  out_color_and_material = fragSurface;

  if (has_texture) {
    vec4 albedo = texture(albedo_texture, fragUv);

    // @todo factor
    uint r = uint(albedo.r * 15);
    uint g = uint(albedo.g * 15);
    uint b = uint(albedo.b * 15);

    uint x = (r << 4) | g;
    uint y = (b << 4) | (fragSurface.y & 0x0F);

    out_color_and_material = uvec4(x, y, fragSurface.z, fragSurface.w);
  }
}