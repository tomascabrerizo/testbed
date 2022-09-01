#version 330 core

layout (location = 0) in vec2 aVert;
layout (location = 1) in vec2 aPos;
layout (location = 2) in vec2 aDim;

uniform int res_x;
uniform int res_y;

void main() {
  vec2 half_res = vec2(res_x * 0.5, res_y * 0.5);
  vec2 half_dim = aDim * 0.5;
  vec2 vertex = (aVert * half_dim) + half_dim + aPos;
  gl_Position = vec4((vertex - half_res) / half_res, 0, 1.0);
}
