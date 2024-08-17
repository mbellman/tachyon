#version 460 core

uniform vec4 transform;

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexUv;

noperspective out vec2 fragUv;

void main() {
  vec4 offset = vec4(transform.x, transform.y, 0.0, 0.0);
  vec4 scale = vec4(transform.z, transform.w, 1.0, 1.0);

  gl_Position = offset + vec4(vertexPosition, 0.0, 1.0) * scale;
  fragUv = vertexUv;
}