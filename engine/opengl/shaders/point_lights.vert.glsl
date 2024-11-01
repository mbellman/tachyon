#version 460 core

struct Light {
  vec3 position;
  float radius;
  vec3 color;
  float power;
};

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 disc_offset;
layout (location = 2) in vec2 disc_scale;
layout (location = 3) in vec3 lightPosition;
layout (location = 4) in float lightRadius;
layout (location = 5) in vec3 lightColor;
layout (location = 6) in float lightPower;

noperspective out vec2 fragUv;
flat out Light light;
flat out vec2 center;
out float intensity;

void main() {
  gl_Position = vec4(vertexPosition * disc_scale + disc_offset, 0.0, 1.0);

  fragUv = gl_Position.xy * 0.5 + 0.5;
  light = Light(lightPosition, lightRadius, lightColor, lightPower);
  center = disc_offset * 0.5 + 0.5;
  intensity = 1.0 - sqrt(vertexPosition.x*vertexPosition.x + vertexPosition.y*vertexPosition.y);
}