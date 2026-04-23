#version 460 core

uniform float scene_time;

flat in uvec4 fragSurface;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragPosition;
in vec3 vertPosition;

flat in vec3 topPosition;
flat in vec3 upDirection;

layout (location = 0) out vec4 out_color_and_depth;

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

float wave(float t) {
  return 0.5 + 0.5 * sin(t);
}

void main() {
  vec4 out_color = UnpackColor(fragSurface);
  const float TAU = 2.0 * 3.141592;
  float t = mod(scene_time, TAU);

  const float pulse_speed = 5.0 * out_color.w;
  const float fluctuation_speed = 2.0 * out_color.w;

  out_color *= 1.0 + 0.05 * wave(TAU * (vertPosition.z * 6.0 - t * pulse_speed));

  float up_dot = abs(dot(fragNormal, upDirection));
  float intensity = 0.9 + 0.1 * wave(TAU * (fluctuation_speed * t + 2.0 * up_dot));

  out_color.rgb = mix(out_color.rgb, vec3(1.0), 0.15 * intensity);

  out_color.rgb *= 5.0 * intensity * clamp(pow(1.0 / (vertPosition.z * 1.3), 15.0), 0.0, 1.0);
  out_color.rgb *= out_color.w;
  out_color.rgb *= out_color.w;
  out_color.rgb *= out_color.w;

  // @temporary
  // @todo move to its own shader
  {
    vec3 N = normalize(fragNormal);

    float distance_from_top = distance(fragPosition, topPosition);
    float phase_rotation = sin(4.0 * acos(dot(N, vec3(0, 0, 1))));
    float phase_alpha = 0.003 * distance_from_top - 2.0 * scene_time + phase_rotation;
    float phase = 0.8 + 0.2 * sin(phase_alpha);

    vec3 light_color = vec3(1.0, 0.7, 0.2);
    float oscillation_alpha = 1.5 * scene_time;
    float oscillation = 0.8 + 0.2 * sin(oscillation_alpha);
    vec3 beam_color = light_color * oscillation * phase;

    float top_falloff = clamp(1.0 - distance_from_top / 5000.0, 0, 1);
    float edge_falloff = dot(N, vec3(0, 0, 1));
    float alpha = clamp(top_falloff * edge_falloff, 0, 1);

    out_color.rgb = mix(vec3(0), beam_color, alpha);

    // out_color.rgb = vec3(phase);
  }

  out_color_and_depth = vec4(out_color.rgb, 0.0);
}