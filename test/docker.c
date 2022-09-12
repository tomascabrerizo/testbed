
/* NOTE: Window docking algorithm testing */

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"

#include <string.h>


typedef struct Rect {
  int l, r, t, b;
} Rect;

static void docker_init(struct CoreWindow *window) {
  (void)window;
}

static void docker_update(struct CoreWindow *window) {
  (void)window;
}

static void docker_render(struct Render2D *render) {
  (void)render;
}



typedef struct Docker {
} Docker;

static Docker docker;
static bool init = false;

void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state) {
  (void)docker;
  (void)state, (void)window, (void)render;
  if(!init) {
    docker_init(window);
    init = true;
  }
  docker_update(window);
  docker_render(render);
}
