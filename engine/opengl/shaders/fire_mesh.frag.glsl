#version 460 core

uniform vec3 camera_position;
uniform float scene_time;

flat in uvec4 fragSurface;
flat in vec3 modelPosition;
flat in vec3 basePosition;
flat in vec3 topPosition;
in vec3 vertPosition;
in vec3 fragPosition;
in vec3 fragNormal;

layout (location = 0) out vec4 out_color_and_depth;

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

vec3 ClosestPointOnLine(vec3 a, vec3 b, vec3 p) {
  vec3 AB = b - a;
  vec3 AP = p - a;
  float ab_length = AB.x * AB.x + AB.y * AB.y;
  float t = (AP.x * AB.x + AP.y * AB.y) / ab_length;

  return a + t * AB;
}

// https://www.shadertoy.com/view/WllXzB
float Noise( vec3 P )
{
    vec3 Pi = floor(P);
    vec3 Pf = P - Pi;
    vec3 Pf_min1 = Pf - 1.0;
    Pi.xyz = Pi.xyz - floor(Pi.xyz * ( 1.0 / 69.0 )) * 69.0;
    vec3 Pi_inc1 = step( Pi, vec3( 69.0 - 1.5 ) ) * ( Pi + 1.0 );
    vec4 Pt = vec4( Pi.xy, Pi_inc1.xy ) + vec2( 50.0, 161.0 ).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    vec2 hash_mod = vec2( 1.0 / ( 635.298681 + vec2( Pi.z, Pi_inc1.z ) * 48.500388 ) );
    vec4 hash_lowz = fract( Pt * hash_mod.xxxx );
    vec4 hash_highz = fract( Pt * hash_mod.yyyy );
    vec3 blend = Pf * Pf * Pf * (Pf * (Pf * 6.0 - 15.0) + 10.0);
    vec4 res0 = mix( hash_lowz, hash_highz, blend.z );
    vec4 blend2 = vec4( blend.xy, vec2( 1.0 - blend.xy ) );
    return dot( res0, blend2.zxzx * blend2.wwyy );
}

float Fire3D(vec3 p) {
  const float speed = 1.0;

  float v = 0.0;
  vec3 line = normalize(basePosition - modelPosition);

  p *= 0.5;

  v += Noise(p * 0.001 + line * scene_time * speed * 1.0) * 0.5;
  p *= 2.0;
  v += Noise(p * 0.001 + line * scene_time * speed * 2.0) * 0.3;
  p *= 2.0;
  v += Noise(p * 0.001 + line * scene_time * speed * 3.0) * 0.2;
  p *= 2.0;
  v += Noise(p * 0.001 + line * scene_time * speed * 4.0) * 0.1;

  float f = fract(v);
  v *= f;
  v *= f;
  v *= f;
  v *= f;
  v *= 5.0;

  return clamp(v, 0.0, 1.0);
}

void main() {
  vec3 N = normalize(fragNormal);
  vec3 V = normalize(camera_position - fragPosition);
  vec3 D = -V;

  float NdotV = max(0.0, dot(N, V));

  vec3 object_color = UnpackColor(fragSurface).rgb;
  vec3 hot_color = object_color * 2.0;

  hot_color.r = clamp(hot_color.r, 0.0, 1.0);
  hot_color.g = clamp(hot_color.g, 0.0, 1.0);
  hot_color.b = clamp(hot_color.b, 0.0, 1.0);

  vec3 out_color = vec3(0.0);
  vec3 sample_position = fragPosition - N * 1500.0;
  float t = scene_time * 0.5 + modelPosition.x;
  float intensity = 5.0 * (0.5 * sin(t) + 0.5);

  for (int i = 0; i < 5; i++) {
    float base_distance = 0.0002 * length(sample_position - basePosition);
    float top_distance = 0.0002 * length(sample_position - topPosition);
    vec3 center_position = ClosestPointOnLine(basePosition, topPosition, sample_position);
    float center_distance = 0.0005 * length(sample_position - center_position);

    float base_factor = pow(1.0 / base_distance, 2.0);
    float top_factor = 0.5 + 0.5 * max(0.0, 1.0 - 1.0 / top_distance);
    float center_factor = pow(min(1.0, 1.0 / center_distance), 3.0);
    float density_factor = Fire3D(sample_position);

    vec3 fire_color = mix(object_color.rgb * 0.5, hot_color, base_factor);
    fire_color = mix(fire_color, hot_color, 0.5 * pow(density_factor, 2.0));
    fire_color = mix(fire_color, vec3(0.0), pow(density_factor, 4.0));

    if (density_factor < 0.3) out_color *= 0.75;

    out_color +=
      fire_color *
      base_factor *
      center_factor *
      top_factor *
      density_factor;

    // @todo use object scale for this
    float step_size = 4000.0 * NdotV;

    sample_position += D * step_size;
  }

  out_color *= pow(NdotV, 2.0) * intensity;
  out_color *= out_color * 100.0;
  // out_color = pow(out_color, vec3(1.0 / 2.2));
  // out_color += vec3(0.2);

  out_color_and_depth = vec4(out_color, 0);
}