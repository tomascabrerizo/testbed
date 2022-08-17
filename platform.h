#ifndef PLATFORM_H
#define PLATFORM_H

#include "platform_gl.h"

typedef struct CoreWindow CoreWindow;

struct CoreWindow *core_window_create(char *name, int width, int height);
void core_window_destroy(struct CoreWindow *window);
int core_window_get_width(struct CoreWindow *window);
int core_window_get_height(struct CoreWindow *window);
void core_window_swap_buffers(struct CoreWindow *window);

typedef struct CoreState {
  /* NOTE: this bool types need to be reset each frame */
  unsigned char quit;
  
  unsigned char resize;
  int width;
  int height;

} CoreState;

CoreState *core_state_get_state(struct CoreWindow *window);

#endif /* PLATFORM_H */
