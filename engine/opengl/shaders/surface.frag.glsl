#version 460 core

uniform sampler2D screenTexture;
uniform vec3 color;
uniform vec4 background;

in vec2 fragUv;

layout (location = 0) out vec4 out_color;

/**
 * A shader for rendering flat textures to the screen. Blending is
 * turned on to facilitate alpha transparency, with modifications
 * needed to avoid the destination color bleeding through the background
 * in semi-transparent areas.
 */
void main() {
  vec4 texture_color = texture(screenTexture, fragUv);

  if (texture_color.a == 0.0) {
    // If transparent, fall back to background color
    out_color = background;
  } else if (texture_color.a < 1.0 && background.a > 0.0) {
    // Fix destination color bleeding through the background
    // in semi-transparent areas. We only need to do this when
    // a custom background color is specified (alpha > 0).
    out_color = vec4(texture_color.rgb * color * texture_color.a, 1.0);
  } else {
    out_color = texture_color * vec4(color, 1.0);
  }
}