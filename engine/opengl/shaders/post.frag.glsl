#version 460 core

#define ENABLE_DEPTH_OF_FIELD_BLUR 1
#define ENABLE_CHROMATIC_ABERRATION 1

uniform sampler2D in_color_and_depth;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

const vec2 texel_size = 1.0 / vec2(1920.0, 1080.0);

void main() {
  vec4 color_and_depth = texture(in_color_and_depth, fragUv);
  vec3 post_color = color_and_depth.rgb;

  #if ENABLE_DEPTH_OF_FIELD_BLUR
    // Depth-of-field blur
    {
      float depth = color_and_depth.w;
      float blur = mix(0.0, 1.0, pow(depth, 200.0));

      const vec2[] offsets = {
        vec2(1.0, 0.0),
        vec2(-1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(0.0, -1.0)
      };

      for (int i = 0; i < 4; i++) {
        vec2 uv = fragUv + blur * texel_size * offsets[i];

        post_color += texture(in_color_and_depth, uv).rgb;
      }

      post_color /= 5.0;
    }
  #endif

  #if ENABLE_CHROMATIC_ABERRATION
    // Chromatic aberration
    {
      const float intensity = 3.0;

      vec2 offset = intensity * (vec2(0.0) - 2.0 * (fragUv - 0.5));
      float r = texture(in_color_and_depth, fragUv + texel_size * offset).r;
      float g = texture(in_color_and_depth, fragUv + 0.5 * texel_size * offset).g;
      float b = texture(in_color_and_depth, fragUv + 0.2 * texel_size * offset).b;

      post_color += vec3(r, g, b);
      post_color /= 2.0;
    }
  #endif

  out_color = post_color;
}