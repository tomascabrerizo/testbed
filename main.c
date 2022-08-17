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

  /* NOTE: Tokenizer test code */
  CoreFile *obj = core_file_create("teapot.obj");
  CoreObjCtx ctx = {0};
  CoreTokenList *tokens = core_tokenize_obj_file(&ctx, obj);
  core_token_list_to_vertex_and_index_list(tokens, &ctx);

  core_token_list_destroy(tokens);
  core_file_destroy(obj);

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
