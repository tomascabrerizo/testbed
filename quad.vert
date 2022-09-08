#version 330 core

layout (location = 0) in vec2 aVert;
layout (location = 1) in vec2 aUvs;
// NOTE: Update per instace 
layout (location = 2) in vec2 aDestPos1;
layout (location = 3) in vec2 aDestPos2;
layout (location = 4) in vec2 aSrcPos1;
layout (location = 5) in vec2 aSrcPos2;
layout (location = 6) in uint aFlags;
layout (location = 7) in float aInterpolator;
layout (location = 8) in vec3 aColor;

uniform int res_x;
uniform int res_y;

out vec2 pos;
out vec2 dim;
out vec2 uvs;
out vec3 color;
flat out uint command;
out float interpolator;

void main() {
  // NOTE: output variables
  pos = aDestPos1;
  dim = aDestPos2 - aDestPos1;
  uvs = mix(aSrcPos1, aSrcPos2, aUvs);
  color = aColor;
  command = aFlags;
  interpolator = aInterpolator;
  // NOTE: scale and translate
  vec2 half_dim = dim * 0.5;
  vec2 vertex = (aVert * half_dim) + half_dim + pos;
  // NOTE: scale into [-1, 1] coordinate space
  vec2 half_res = vec2(float(res_x) * 0.5, float(res_y) * 0.5);
  vec2 screen = (vertex / half_res) - 1;
  screen.y = -screen.y;
  // NOTE: screen.y = -screen.y; // Invert y axis
  
  gl_Position = vec4(screen, 0, 1.0);
}
