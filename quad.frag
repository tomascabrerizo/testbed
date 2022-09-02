#version 330 core

out vec4 fragment;
in vec2 pos;
in vec2 dim;
flat in uint flags;

#define COMMAND_RECT uint(1)
#define COMMAND_TEXTURE uint(2)
#define COMMAND_BORDER_RECT uint(4)

bool has_flag(uint flags, uint flag) {
  uint result = (flags / flag);
  return (result - ((result / uint(2))*uint(2))) == uint(1);
}

void main() {
  if(has_flag(flags, COMMAND_RECT)) {
    fragment = vec4(1.0, 0, 0.0, 1.0);
  } else if(has_flag(flags, COMMAND_TEXTURE)){
    fragment = vec4(0, 1.0, 0.0, 1.0);
  } else if(has_flag(flags, COMMAND_BORDER_RECT)){
    fragment = vec4(0, 0.0, 1.0, 1.0);
  }
}
