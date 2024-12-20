#version 460 core

#define ENABLE_DEPTH_OF_FIELD_BLUR 1
#define ENABLE_CHROMATIC_ABERRATION 1

uniform sampler2D in_color_and_depth;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

const vec2 TEXEL_SIZE = 1.0 / vec2(1920.0, 1080.0);

void main() {
  vec4 color_and_depth = texture(in_color_and_depth, fragUv);
  vec3 post_color = color_and_depth.rgb;

  #if ENABLE_DEPTH_OF_FIELD_BLUR
    // Depth-of-field blur
    {
      const float max_blur = 1.5;
      float depth = color_and_depth.w;
      float blur = mix(0.0, max_blur, pow(depth, 200.0));

      const vec2[] offsets = {
        vec2(0.0, -1.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(-1.0, 0.0),
      };

      for (int i = 0; i < 4; i++) {
        vec2 uv = fragUv + blur * TEXEL_SIZE * offsets[i];

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
      float r = texture(in_color_and_depth, fragUv + TEXEL_SIZE * offset).r;
      float g = texture(in_color_and_depth, fragUv + 0.5 * TEXEL_SIZE * offset).g;
      float b = texture(in_color_and_depth, fragUv + 0.2 * TEXEL_SIZE * offset).b;

      post_color += vec3(r, g, b);
      post_color /= 2.0;
    }
  #endif

  // post_color = mix(post_color, vec3(0.6, 0.8, 1.0), 0.5 * pow(color_and_depth.w, 200.0));

  out_color = post_color;
}