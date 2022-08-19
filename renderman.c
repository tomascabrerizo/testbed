#include "renderman.h"

/* TODO: Implements RenderMan functions */

/* NOTE: Renderer functions */

RManRenderer *renderman_render_create(void) {
  return 0; /* TODO: Function not implemented */
}

void renderman_render_destroy(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}

void renderman_render_add_shader(RManRenderer *render, char *name, struct RManShader *shader) {
  (void)render; /* TODO: Funciton not implemented */
  (void)name;
  (void)shader;
}
void renderman_render_clear(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}
void renderman_render_draw(RManRenderer *render) {
  (void)render; /* TODO: Funciton not implemented */
}

/* Shader functions */

RManShader *renderman_shader_create(char *vert, char *frag) {
  (void)vert; /* TODO: Funciton not implemented */
  (void)frag;
  return 0;
}

void renderman_shader_destroy(RManShader *shader) {
  (void)shader; /* TODO: Funciton not implemented */
}

void renderman_shader_add_m4(RManShader *shader, char *name, M4 m) {
  (void)shader; /* TODO: Funciton not implemented */
  (void)name;
  (void)m;
}
