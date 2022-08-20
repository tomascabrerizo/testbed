#include <stdio.h>
#include <unistd.h>

#include "core.h"
#include "core_obj.h"
#include "core_math.h"
#include "core_hash.h"
#include "renderman.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(void) {
  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);

#if 0
  CoreObjCtx *obj = core_obj_create("teapot.obj");
  printf("Vertex count:%ld\n", obj->v_count);
  printf("Indices count:%ld\n", obj->i_count);
  core_obj_destroy(obj);
#endif
  
  /* NOTE: Hash map test */
  int keys[5]; /* NOTE: testing keys */
  CoreMap *map = core_map_create();
  core_map_add(map, (void *)&keys[0], (void *)123);
  core_map_add(map, (void *)&keys[1], (void *)1);
  core_map_add(map, (void *)&keys[2], (void *)"manuel");
  core_map_add(map, (void *)&keys[3], (void *)321);
  core_map_add(map, (void *)&keys[4], (void *)"tomi");
  
  char *tomi = core_map_get(map, &keys[4]);
  printf("%s\n", tomi);
  int _123 = (int)(uint64_t)core_map_get(map, &keys[0]);
  printf("%d\n", _123);
  
  int num = 9;
  printf("%d, aling %d\n", num, CORE_ALING(num, 8));

  core_map_destroy(map);


  RManShader *shader = renderman_shader_create("test.vert", "test.frag");
  renderman_shader_add_m4(shader, "projection", m4_perspective2(3.14f/2.0f, core_window_get_width(window)/core_window_get_height(window), 0.1f, 100.0f));

  RManRenderer *render = renderman_render_create();
  renderman_render_add_shader(render, "main", shader);
  
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

    renderman_render_clear(render);
    renderman_render_draw(render);
    core_window_swap_buffers(window);
  }
  
  renderman_shader_destroy(shader);
  renderman_render_destroy(render);
  core_window_destroy(window);

  return 0;
}
