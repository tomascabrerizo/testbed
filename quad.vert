#version 330 core

layout (location = 0) in vec2 aVert;
layout (location = 1) in vec2 aPos;
layout (location = 2) in vec2 aDim;
layout (location = 3) in uint aFlags;

uniform int res_x;
uniform int res_y;

out vec2 pos;
out vec2 dim;
flat out uint flags;

void main() {
  // output variables
  pos = aPos;
  dim = aDim;
  flags = aFlags;
  // scale and translate
  vec2 half_dim = aDim * 0.5;
  vec2 vertex = (aVert * half_dim) + half_dim + aPos;
  // scale into [-1, 1] coordinate space
  vec2 half_res = vec2(float(res_x) * 0.5, float(res_y) * 0.5);
  vec2 screen = (vertex / half_res) - 1;
  screen.y = -screen.y; // Invert y axis
  
  gl_Position = vec4(screen, 0, 1.0);
}
