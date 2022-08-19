#ifndef RENDERMAN_H
#define RENDERMAN_H
/* TODO: Renderman is the name of the pixar renderer. find a good name */
#include "core_gl.h"
#include "core_type.h"
#include "core_math.h"
/* NOTE: This library is a modern OPENGL 3.3 wraper */

struct RManShader;

typedef struct RManRenderer {
  unsigned int vao; 
}RManRenderer;

RManRenderer *renderman_render_create(void);
void renderman_render_destroy(RManRenderer *render);

void renderman_render_add_shader(RManRenderer *render, char *name, struct RManShader *shader);
void renderman_render_clear(RManRenderer *render);
void renderman_render_draw(RManRenderer *render);

typedef struct RManShader {
  unsigned int id;
} RManShader;

RManShader *renderman_shader_create(char *vert, char *frag);
void renderman_shader_destroy(RManShader *shader);

void renderman_shader_add_m4(RManShader *shader, char *name, M4 m);

#endif /* RENDERMAN_H */
