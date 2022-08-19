#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "core_obj.h"
#include "core_math.h"

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
#if 0
  uint64_t i;
  for(i = 0; i < obj->i_count; i += 3) {
    int i0 = obj->i_list[i + 0] - 1;
    int i1 = obj->i_list[i + 1] - 1;
    int i2 = obj->i_list[i + 2] - 1;

    float v00 = obj->v_list[i0 * 3 + 0];
    float v01 = obj->v_list[i0 * 3 + 1];
    float v02 = obj->v_list[i0 * 3 + 2];

    float v10 = obj->v_list[i1 * 3 + 0];
    float v11 = obj->v_list[i1 * 3 + 1];
    float v12 = obj->v_list[i1 * 3 + 2];

    float v20 = obj->v_list[i2 * 3 + 0];
    float v21 = obj->v_list[i2 * 3 + 1];
    float v22 = obj->v_list[i2 * 3 + 2];
    printf("------- TRIANGLE --------\n");
    printf("V0 (%f, %f, %f)\n", v00, v01, v02);
    printf("V1 (%f, %f, %f)\n", v10, v11, v12);
    printf("V2 (%f, %f, %f)\n", v20, v21, v22);
  }
#endif

  core_obj_destroy(obj);
  
  /* NOTE: Simple math test */
  V3 a = v3(1, 2, 3);
  V3 b = v3(4, 5, 6);
  (void)a;
  (void)b;
  //V3 c = v3_add(a, b);
  //printf("x:%f, y:%f, z:%f\n", c.x, c.y, c.z);

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
