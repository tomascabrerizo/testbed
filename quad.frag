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

// from https://iquilezles.org/articles/distfunctions
float rounded_box_sdf(vec2 center_pos, vec2 half_size, float radius) {
    return length(max(abs(center_pos) - half_size + radius, 0.0)) - radius;
}

uniform int res_x;
uniform int res_y;

void main() {

  vec4 bg_color = vec4(0, 0, 0, 1);
  vec3 border_color = vec3(1, 0, 0);
  vec3 color = vec3(0.8, 0.8, 1.0);
  float r = 0.12;
  float border_radius = 0.02;

  float inv_aspect = (1 / aspect);
  vec2 coord = vec2(uvs.x*inv_aspect, uvs.y);
  vec2 size = vec2(inv_aspect, 1);
  vec2 center_pos = coord - (size * 0.5);

  float sdf = rounded_box_sdf(center_pos, size*0.5, r); 
  float sdf_mask = step(0, -sdf);
  
  float border_sdf = sdf + border_radius;
  float border_mask = step(0, border_sdf) * sdf_mask;
  float inv_border_mask = step(0, -border_sdf) * sdf_mask;
  
  
  vec4 rect = vec4(vec3(sdf_mask * inv_border_mask)*color, 1.0); 
  vec4 border = vec4(vec3(border_mask)*border_color, 1.0);
  vec4 inv_border = vec4(vec3(inv_border_mask)*border_color, 1.0);
  fragment = rect + border;// + diff; 
}
