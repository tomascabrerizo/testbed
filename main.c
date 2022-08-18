#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "platform.h"
#include "platform_obj.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(void) {
  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);
 
  /* NOTE: OpenGL 3.3 test code */
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  unsigned int vbo;
  glGenBuffers(1, &vbo);
  
  CoreObjCtx *obj = core_obj_create("teapot.obj");
  
  printf("Vertex count:%ld\n", obj->v_count);
  printf("Indices count:%ld\n", obj->i_count);
  
  uint64_t i;
  for(i = 0; i < obj->i_count; i += 3) {
    int i0 = obj->i_list[i + 0] - 1;
    int i1 = obj->i_list[i + 1] - 1;
    int i2 = obj->i_list[i + 2] - 1;

    float t0 = obj->v_list[i0];
    float t1 = obj->v_list[i1];
    float t2 = obj->v_list[i2];
    
    printf("TRIANGLE %ld\n", i);
    printf("t0:%f, t1:%f, t2:%f\n", t0, t1, t2);
  }

  core_obj_destroy(obj);

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
