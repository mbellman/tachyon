#version 460 core

uniform vec4 offset_and_scale;
uniform float rotation;

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexUv;

noperspective out vec2 fragUv;

void main() {
  vec4 offset = vec4(offset_and_scale.x, offset_and_scale.y, 0.0, 0.0);
  vec4 scale = vec4(offset_and_scale.z, offset_and_scale.w, 1.0, 1.0);

  float x = vertexPosition.x * cos(rotation) - vertexPosition.y * sin(rotation);
  float y = vertexPosition.x * sin(rotation) + vertexPosition.y * cos(rotation);

  gl_Position = offset + vec4(x, y, 0.0, 1.0) * scale;
  fragUv = vertexUv;
}