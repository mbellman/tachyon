#version 460 core

uniform sampler2D in_normal_and_depth;
uniform usampler2D in_color_and_material;
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

vec3 DirectionalLightRadiance(vec3 albedo, vec3 position, vec3 N, vec3 V, float roughness, float metalness, vec3 F0) {
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

float FastDistributionGGX(float NdotH, float roughness) {
  float r4 = roughness*roughness * roughness*roughness;
  float d = (NdotH*NdotH * (r4 - 1.0) + 1.0);

  return r4 / (PI * d*d);
}

float FastGeometryGGX(float NdotH, float roughness, float metalness) {
  return pow(NdotH, 2 * roughness) * pow(1.0 - metalness, 4);
}

float FastClearcoat(float NdotH, float NdotV, float clearcoat) {
  return clearcoat * (FastDistributionGGX(NdotH, 0.1) + pow(1.0 - NdotV, 8));
}

float FastSubsurface(float NdotV, float subsurface) {
  return subsurface * (4 * NdotV + 16 * pow(1.0 - NdotV, 4));
}

vec3 FastDirectionalLightRadiance(
  vec3 albedo,
  vec3 position,
  vec3 N,
  vec3 V,
  float NdotV,
  float roughness,
  float metalness,
  float clearcoat,
  float subsurface
) {
  vec3 L = -normalize(light_direction);
  vec3 H = normalize(V + L);

  float NdotH = max(dot(N, H), 0.0);
  float NdotL = max(dot(N, L), 0.0);

  float sD = FastDistributionGGX(NdotH, roughness);
  float sG = FastGeometryGGX(NdotH, roughness, metalness);

  float D = (1.0 - metalness) * (1.0 - roughness * 0.5) * NdotL;
  float S = (sD + sG) * NdotL;
  float C = FastClearcoat(NdotH, NdotV, clearcoat) * NdotL;
  float Sc = FastSubsurface(NdotV, subsurface) * (NdotL + 0.05) * (1.0 - metalness * 0.95);

  return light_color * (albedo * D + albedo * S + C + albedo * albedo * Sc) / PI;
}

vec3 AmbientFresnel(float NdotV) {
  return 0.002 * vec3(pow(1 - NdotV, 5));
}

vec4 UnpackColor(uvec4 surface) {
  float r = float((surface.x & 0xF0) >> 4) / 15.0;
  float g = float(surface.x & 0x0F) / 15.0;
  float b = float((surface.y & 0xF0) >> 4) / 15.0;
  float a = float(surface.y & 0x0F) / 15.0;

  return vec4(r, g, b, a);
}

struct Material {
  float roughness;
  float metalness;
  float clearcoat;
  float subsurface;
};

Material UnpackMaterial(uvec4 surface) {
  float roughness = float((surface.z & 0xF0) >> 4) / 15.0;
  float metalness = float(surface.z & 0x0F) / 15.0;
  float clearcoat = float((surface.w & 0xF0) >> 4) / 15.0;
  float subsurface = float(surface.w & 0x0F) / 15.0;

  return Material(roughness, metalness, clearcoat, subsurface);
}

void main() {
  vec4 frag_normal_and_depth = texture(in_normal_and_depth, fragUv);
  uvec4 frag_color_and_material = texture(in_color_and_material, fragUv);

  vec3 N = frag_normal_and_depth.xyz;
  vec4 color = UnpackColor(frag_color_and_material);
  vec3 albedo = color.rgb;
  float emissive = color.a;
  Material material = UnpackMaterial(frag_color_and_material);
  vec3 position = GetWorldPosition(frag_normal_and_depth.w, fragUv, inverse_projection_matrix, inverse_view_matrix);

  vec3 V = normalize(camera_position - position);
  float NdotV = max(dot(N, V), 0.0);

  float roughness = material.roughness;
  float metalness = material.metalness;
  float clearcoat = material.clearcoat;
  float subsurface = material.subsurface;

  if (roughness < 0.05) roughness = 0.05;

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metalness);
  // vec3 color = DirectionalLightRadiance(albedo, position, N, V, roughness, metalness, F0);
  vec3 out_color = FastDirectionalLightRadiance(albedo, position, N, V, NdotV, roughness, metalness, clearcoat, subsurface);

  out_color += AmbientFresnel(NdotV);
  out_color = mix(out_color, albedo, emissive);

  if (frag_normal_and_depth.w >= 1.0) out_color = vec3(0);


  // @todo move to post shader
  out_color = out_color / (out_color + vec3(1.0));
  out_color = pow(out_color, vec3(1.0 / 2.2));

  out_color_and_depth = vec4(out_color, frag_normal_and_depth.w);
}