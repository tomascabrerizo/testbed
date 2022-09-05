#version 330 core

out vec4 fragment;
in vec2 pos;
in vec2 dim;
in vec2 uvs;
flat in uint command;
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
uniform sampler2D tex;

void main() {
  if(has_flag(command, COMMAND_RECT)) {
    vec4 bg_color = vec4(0, 0, 0, 1);
    vec3 border_color = vec3(0.27, 0.44, 0.70);
    vec3 color = vec3(0.25, 0.37, 0.55);

    /* TODO: all this information should come in vertex attributes */
    float r = 10.0;
    float border_radius = 2.0;
    float edge_softness = 1.0; 
    float tex = 1.0;
    
    vec2 coord = uvs * dim;
    vec2 center_pos = coord - (dim* 0.5);

    float sdf = rounded_box_sdf(center_pos, dim*0.5, r); 
    
    if(sdf <= 0.0 && sdf >= -border_radius) {
      float factor = 1 - smoothstep(0.0, 1.0, sdf);
      color = mix(color, border_color, factor);
    }

    float alpha = 1 - smoothstep(0, 2*edge_softness, sdf);
    fragment = vec4(color, alpha);
  
  } else if(has_flag(command, COMMAND_TEXTURE)) {
    float edge_softness = 1.0;
    vec3 color = vec3(0.0, 1.0, 0.0);
    
    vec2 tex_dim = vec2(512);
    vec2 glyph_off = vec2(453, 72) / tex_dim;
    vec2 glyph_dim = vec2(46, 56) / tex_dim;

    vec2 tex_uvs = vec2(uvs.x, 1.0 - uvs.y);
    tex_uvs = mix(glyph_off, glyph_off + glyph_dim, tex_uvs); 
    float sdf = texture(tex, tex_uvs).a;
    float alpha = smoothstep(0.5, 0.55, 1.0 - sdf);

    fragment = vec4(color, 1.0 - alpha);
  }
}
