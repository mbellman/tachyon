#version 460 core

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
uniform sampler2D in_accumulation;
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;
uniform float time;

noperspective in vec2 fragUv;

layout (location = 0) out vec3 out_color;

const vec3[] ssao_sample_points = {
  vec3(0.021429, 0.059112, 0.07776),
  vec3(0.042287, -0.020052, 0.092332),
  vec3(0.087243, -0.025947, 0.068744),
  vec3(-0.001496, -0.027043, 0.128824),
  vec3(-0.088486, -0.099508, 0.081747),
  vec3(0.108582, 0.02973, 0.15043),
  vec3(-0.131723, 0.143868, 0.115246),
  vec3(-0.137142, -0.089451, 0.21753),
  vec3(-0.225405, 0.214709, 0.09337),
  vec3(-0.152747, 0.129375, 0.328595),
  vec3(0.155238, 0.146029, 0.398102),
  vec3(-0.205277, 0.358875, 0.3242),
  vec3(0.129059, 0.579216, 0.124064),
  vec3(-0.427557, -0.123908, 0.53261),
  vec3(0.661657, -0.296235, 0.311568),
  vec3(0.175674, 0.013574, 0.87342)
};

/**
 * Returns a value within the range -1.0 - 1.0, constant
 * in screen space, acting as a noise filter.
 */
float noise(float seed) {
  return 2.0 * (fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * seed * 43758.545312) - 0.5);
}

/**
 * Reconstructs a fragment's world position from depth,
 * using the inverse projection/view matrices to transform
 * the fragment coordinates back into world space.
 */
vec3 GetWorldPosition(float depth, vec2 frag_uv, mat4 inverse_projection, mat4 inverse_view) {
  float z = depth * 2.0 - 1.0;
  vec4 clip = vec4(frag_uv * 2.0 - 1.0, z, 1.0);
  vec4 view_position = inverse_projection * clip;

  view_position /= view_position.w;

  vec4 world_position = inverse_view * view_position;

  return world_position.xyz;
}

/**
 * Returns the 2D screen coordinates, normalized to the range
 * [0.0, 1.0], corresponding to a point in view space.
 */
vec2 GetScreenCoordinates(vec3 view_position, mat4 projection_matrix) {
  vec4 projection = projection_matrix * vec4(view_position, 1.0);
  vec3 clip = projection.xyz / projection.w;

  return clip.xy * 0.5 + 0.5;
}

/**
 * Maps a nonlinear [0, 1] depth value to a linearized
 * depth between the near and far planes.
 */
float GetLinearDepth(float depth, float near, float far) {
  float clip_depth = 2.0 * depth - 1.0;

  return 2.0 * near * far / (far + near - clip_depth * (far - near));
}

float LinearDepth(float depth, float near, float far) {
  return 2.0 * near / (far + near - depth * (far - near));
}

float saturate(float value) {
  return clamp(value, 0.0, 1.0);
}

float GetSSAO(int total_samples, float depth, vec3 position, vec3 normal, float seed) {
  float linear_depth = LinearDepth(depth, 500.0, 10000000.0);
  float radius = mix(100.0, 30000.0, pow(linear_depth, 0.333));
  float ssao = 0.0;

  vec3 random_vector = vec3(noise(1.0 + seed), noise(2.0 + seed), noise(3.0 + seed));
  vec3 tangent = normalize(random_vector - normal * dot(random_vector, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 tbn = mat3(tangent, bitangent, normal);

  for (int i = 0; i < total_samples; i++) {
    vec3 tbn_sample_position = tbn * ssao_sample_points[i];
    vec3 world_sample_position = position + tbn_sample_position * radius;
    vec3 view_sample_position = (view_matrix * vec4(world_sample_position, 1.0)).xyz;
    vec2 screen_sample_uv = GetScreenCoordinates(view_sample_position, projection_matrix);
    // float sample_depth = textureLod(texColorAndDepth, screen_sample_uv, 1).w;
    float sample_depth = texture(in_normal_and_depth, screen_sample_uv).w;
    float linear_sample_depth = GetLinearDepth(sample_depth, 500.0, 10000000.0);

    if (linear_sample_depth < -view_sample_position.z) {
      float occluder_distance = -view_sample_position.z - linear_sample_depth;
      float occlusion_factor = mix(1.0, 0.0, saturate(occluder_distance / radius));

      ssao += occlusion_factor;
    }
  }

  const float max_ssao = 0.7;
  const float min_ssao = 0.0;

  float factor = mix(max_ssao, min_ssao, pow(depth, 50.0));

  return ssao / float(total_samples) * factor;
}

void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);
  vec3 color = texture(in_accumulation, fragUv).xyz;
  float ssao = GetSSAO(8, frag_normal_and_depth.w, position, frag_normal_and_depth.xyz, fract(time));

  out_color = color - ssao;
}