#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <GL/gl.h>

#include "platform.h"

int main(void) {
  CoreWindow *window = core_window_create("TestBed", 600, 400);
  
  bool running = true;
  while(running) {

    CoreState *state = core_state_get_state(window);
    if(state->quit) {
      printf("Quitting application\n");
      running = false;
    }

    /* TODO: Implements inplement keyboard and mouse input
     * Something like this:
    if(core_state_key_down(state, CORE_K_A)) {
    }
    */

    glClearColor(0, 0.5, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    core_window_swap_buffers(window);
  }
  
  core_window_destroy(window);

  return 0;
}
