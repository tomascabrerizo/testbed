#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"
#include "core_gl.h"

#if 1
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#else
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#endif

struct CoreFont *core_font_create(char *path);
void core_font_destroy(struct CoreFont *font);

int main(void) {

  struct CoreFont *font = core_font_create("font/font.fnt");

  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);
  Render2D *render = render2d_create();
  render2d_alpha_test(render, true);
  
  bool running = true;
  while(running) {
    CoreState *state = core_state_get_state(window);
    if(state->quit) {
      printf("Quitting application\n");
      running = false;
    }
    
    if(state->resize) {
      printf("The window is resizing\n");
      glViewport(0, 0, state->width, state->height);
      render2d_set_resolution(render, state->width, state->height);
    }
    
    render2d_begin(render);

    int dim_x = 400;
    int dim_y = 200;
    int pos_x = (core_window_get_width(window) / 2) - (dim_x / 2);
    int pos_y = (core_window_get_height(window) / 2) - (dim_y / 2);
    render2d_draw_quad(render, pos_x, pos_y, dim_x, dim_y);
    render2d_draw_quad(render, 60, 0, 100, 50);

    // TODO: char id=65      x=453  y=72   width=46   height=56   xoffset=-7   yoffset=6    xadvance=48   page=0    chnl=0 
    float scale = 1.5;
    render2d_draw_texture(render, 0, core_window_get_height(window) - 56*scale, 46*scale, 56*scale);

    render2d_end(render);
    
    core_window_swap_buffers(window);
  }
  
  render2d_destroy(render);
  core_window_destroy(window);
  core_font_destroy(font);

  return 0;
}
