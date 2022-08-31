#ifndef RENDERMAN_H
#define RENDERMAN_H
/* TODO: Renderman is the name of the pixar renderer. find a good name */
#include "core_type.h"
#include "core_math.h"
#include "core_ds.h"
/* NOTE: This library is a modern OPENGL 3.3 wraper */

/* TODO: program vtretch buffers */

typedef uint32_t RManHandle ;

typedef struct RManRenderer {
  
} RManRenderer;


typedef struct RManFrameBuffer {
  unsigned int id;
} RManFrameBuffer;


RManRenderer *renderman_render_create(void);
void renderman_render_destroy(RManRenderer *render);

RManHandle renderman_framebuffer_create(RManRenderer *render);
void renderman_famebuffer_destroy(RManRenderer *render, RManHandle handle);

void renderman_render_begin(RManRenderer *render);
void renderman_render_end(RManRenderer *render);

typedef struct RManShader {
  unsigned int id;
} RManShader;


typedef struct Vertex2D {
  float x, y;
} Vertex2D;

typedef struct Render2D {
  unsigned int vao;
  unsigned int vbo;
  unsigned int program;
} Render2D;

Render2D *render2d_create();
void render2d_destroy(Render2D *render);
void render2d_draw(Render2D *render);

#endif /* RENDERMAN_H */
