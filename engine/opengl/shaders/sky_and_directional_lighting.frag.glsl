#version 460 core

uniform sampler2D in_color_and_depth;
uniform sampler2D in_normal_and_material;
uniform mat4 inverse_projection_matrix;
uniform mat4 inverse_view_matrix;
uniform vec3 camera_position;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

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

const float PI = 3.141592;

// https://learnopengl.com/PBR/Lighting
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a      = roughness*roughness;
  float a2     = a*a;
  float NdotH  = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float num   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float num   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2  = GeometrySchlickGGX(NdotV, roughness);
  float ggx1  = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// @temporary @todo pass as parameters
const vec3 light_direction = vec3(-1.0, -1.0, -1.0);
const vec3 light_color = vec3(1.0);

vec3 GetDirectionalLightRadiancePBR(vec3 albedo, vec3 position, vec3 N, vec3 V, float roughness, float metalness, vec3 F0) {
  vec3 L = -normalize(light_direction);
  vec3 H = normalize(V + L);

  float NDF = DistributionGGX(N, H, roughness);
  float G = GeometrySmith(N, V, L, roughness);
  vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metalness;

  vec3 numerator    = NDF * G * F;
  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
  vec3 specular     = numerator / denominator;  

  float NdotL = max(dot(N, L), 0.0);
  return (kD * albedo / PI + specular) * light_color * NdotL;
}

float EaseInOut(float t) {
  return t < 0.5 ? 4.0 * t * t * t : 1.0 - pow(-2.0 * t + 2.0, 3) / 2.0;
}

// @WIP
vec3 GetDirectionalLightRadianceFastPBR(vec3 albedo, vec3 position, vec3 N, vec3 V, float roughness, float metalness, vec3 F0) {
  vec3 L = -normalize(light_direction);
  vec3 H = normalize(V + L);

  float NdotH = max(dot(N, H), 0.0);
  float NdotL = max(dot(N, L), 0.0);

  float D = NdotL * (1.0 - metalness);

  // @todo write an optimized distribution GGX
  float sD = DistributionGGX(N, H, roughness);
  // Geometry specular GGX
  float sG = pow(NdotH, 10 * roughness) * pow(1.0 - metalness, 4);
  // Combined specular term
  float S = (sD + sG) * NdotL;

  return light_color * ((D * albedo) / PI + albedo * S);
}

void main() {
  vec4 frag_color_and_depth = texture(in_color_and_depth, fragUv);
  vec4 frag_normal_and_material = texture(in_normal_and_material, fragUv);

  vec3 albedo = frag_color_and_depth.rgb;
  vec3 N = frag_normal_and_material.xyz;
  vec3 position = GetWorldPosition(frag_color_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);

  vec3 V = normalize(camera_position - position);
  float roughness = 0.2;
  float metalness = 1.0;

  if (roughness < 0.05) roughness = 0.05;

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metalness);

  vec3 fresnel = 0.002 * vec3(pow(1 - dot(N, V), 5.0));

  // vec3 color = fresnel + GetDirectionalLightRadiancePBR(albedo, position, N, V, roughness, metalness, F0);
  vec3 color = fresnel + GetDirectionalLightRadianceFastPBR(albedo, position, N, V, roughness, metalness, F0);

  if (frag_color_and_depth.w >= 1.0) color = vec3(0);

  // @todo move to post shader
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  out_color_and_depth = vec4(color, frag_color_and_depth.w);
}