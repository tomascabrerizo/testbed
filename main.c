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


struct FramesUi;
struct FramesUi *f_ui_create(struct Render2D *render);
void f_ui_destroy(struct FramesUi *ui);
void f_ui_set_state(struct FramesUi *ui, struct CoreState *state);

bool f_do_button(struct FramesUi *ui, char *text, int x, int y, int w, int h);

int main(void) {


  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);
  Render2D *render = render2d_create();
  render2d_alpha_test(render, true);
  struct FramesUi *ui = f_ui_create(render);

  bool running = true;
  while(running) {
    CoreState *state = core_state_get_state(window);
    f_ui_set_state(ui, state);
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
    if(f_do_button(ui, "button1", 10, 10, 70, 35)) {
      printf("button1 clicked\n");
    }
    if(f_do_button(ui, "button2", 10, 45, 170, 135)) {
      printf("button2 clicked\n");
    }
    render2d_end(render);

    core_window_swap_buffers(window);
  }

  f_ui_destroy(ui);
  render2d_destroy(render);
  core_window_destroy(window);

  return 0;
}
