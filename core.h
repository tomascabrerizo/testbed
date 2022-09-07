#ifndef CORE_H
#define CORE_H

#include "core_type.h"

typedef struct CoreWindow CoreWindow;

struct CoreWindow *core_window_create(char *name, int width, int height);
void core_window_destroy(struct CoreWindow *window);
int core_window_get_width(struct CoreWindow *window);
int core_window_get_height(struct CoreWindow *window);
void core_window_swap_buffers(struct CoreWindow *window);

void *core_read_entire_file(char *path, uint64_t *size);

typedef struct CoreState {
  /* NOTE: this bool types need to be reset each frame */
  unsigned char quit;
  
  unsigned char resize;
  int width;
  int height;

  int mouse_x;
  int mouse_y;
  bool mouse_button_down;

} CoreState;

CoreState *core_state_get_state(struct CoreWindow *window);

#endif /* CORE_H */
