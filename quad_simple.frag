#version 330 core

in vec2 pos;
in vec2 dim;
in vec2 uvs;
in vec3 color;
flat in uint command;
in float interpolator;

out vec4 fragment;

#define COMMAND_RECT uint(1)
#define COMMAND_TEXTURE uint(2)
#define COMMAND_BORDER_RECT uint(4)

bool has_flag(uint flags, uint flag) {
  uint result = (flags / flag);
  return (result - ((result / uint(2))*uint(2))) == uint(1);
}

uniform int res_x;
uniform int res_y;
uniform sampler2D tex;

const vec2 tex_dim = vec2(512);

void main() {
  if(has_flag(command, COMMAND_RECT)) {
    vec3 color2 = vec3(0.1);
    vec3 color3 = mix(color, color - color2, interpolator);
    fragment = vec4(color3, 1.0);
  
  } else if(has_flag(command, COMMAND_TEXTURE)) {
    const float edge_softness = 0.2;
    vec2 tex_uvs = vec2(uvs) / tex_dim;
    
    float sdf = 1.0 - texture(tex, tex_uvs).a;
    float alpha = 1.0 - smoothstep(0.4, 0.5 + edge_softness, sdf);

    fragment = vec4(color, alpha);
    //fragment = vec4(vec3(alpha), 1.0);
  }
}
