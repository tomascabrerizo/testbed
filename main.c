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

int main(void) {

  CoreWindow *window = core_window_create("TestBed", WINDOW_WIDTH, WINDOW_HEIGHT);

  Render2D *render = render2d_create();
  
  bool running = true;

  /* NOTE: Time test */ /* TODO: Remove, this timer is really BAD */
#define CORE_DEBUG_TIMER 0
#if CORE_DEBUG_TIMER
  struct timeval te; 
  gettimeofday(&te, NULL);
  long long last_frame = te.tv_sec*1000LL + te.tv_usec/1000; 
#endif

  while(running) {

#if CORE_DEBUG_TIMER
    gettimeofday(&te, NULL);
    long long current_frame = te.tv_sec*1000LL + te.tv_usec/1000;
    double delta_s = (float)(current_frame - last_frame)/1000.0f;
    long long fps = (int)(1 / delta_s);
    printf("ms per frame:%lf\n", delta_s);
    printf("fps per frame:%lld\n", fps);
#endif

    CoreState *state = core_state_get_state(window);
    if(state->quit) {
      printf("Quitting application\n");
      running = false;
    }
    
    if(state->resize) {
      printf("The window is resizing\n");
      glViewport(0, 0, state->width, state->height);
    }
     
    render2d_draw(render);
    core_window_swap_buffers(window);

#if CORE_DEBUG_TIMER
    last_frame = current_frame;
#endif
  }
  
  render2d_destroy(render);
  core_window_destroy(window);

  return 0;
}
