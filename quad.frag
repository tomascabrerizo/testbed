#version 330 core

out vec4 fragment;
in vec2 pos;
in vec2 dim;
in vec2 uvs;
flat in uint flags;
in float aspect;

#define COMMAND_RECT uint(1)
#define COMMAND_TEXTURE uint(2)
#define COMMAND_BORDER_RECT uint(4)

bool has_flag(uint flags, uint flag) {
  uint result = (flags / flag);
  return (result - ((result / uint(2))*uint(2))) == uint(1);
}

void main() {

  vec2 coords = uvs;
  coords.x = coords.x / aspect; /* TODO: Move this calculation to vertex shader */
  float r = 1.0;
  float sdf = length(coords - vec2(clamp(coords.x, 0.5, (1.0/aspect)- 0.5), 0.5)) * 2 - r; 
  vec3 color = (float(sdf <= 0) * vec3(0.0, 1.0, 0.0));
  color = (-sdf * vec3(0.0, 1.0, 0.0));

  fragment = vec4(color, 1.0);
}
