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
#include "frames.h"

#if 1
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#else
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#endif


/* NOTE: UI state */
bool checked;

void frames_ui_test(struct FramesUi *ui) {
  f_do_label(ui, "Label:", 20, 100);
  f_do_label(ui, "Im a Label.", 145, 100);

  f_do_label(ui, "Button:", 20, 140);
  f_do_button(ui, "Button:", 145, 135, 120, 30);


  f_do_label(ui, "Checked Box:", 20, 180);
  f_do_checkbox(ui, "checkbox", 145, 180, &checked);
}

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
    frames_dock_test(window, render, state);
    frames_ui_test(ui);

    render2d_end(render);

    core_window_swap_buffers(window);
  }

  f_ui_destroy(ui);
  render2d_destroy(render);
  core_window_destroy(window);

  return 0;
}
