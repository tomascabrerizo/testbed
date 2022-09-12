
/* NOTE: Window docking algorithm testing */

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"

#include <string.h>

typedef struct Rect {
  int l, r, t, b;
} Rect;

typedef enum ContainerType {
  WINDOW_CONTAINER,
  VSPLIT_CONTAINER,
  HSPLIT_CONTAINER,
} ContainerType;

typedef struct ContainerHeader {
  ContainerType type;
  Rect rect;
} ContainerHeader;

typedef struct WindowContainer {
  ContainerHeader header;
} WindowContainer;

typedef struct VSplitContainer {
  ContainerHeader header;
  float position;
  union Container *right;
  union Container *left;
  struct VSplitContainer *next;
  struct VSplitContainer *prev;
} VSplitContainer;

typedef struct HSplitContainer {
  ContainerHeader header;
  float position;
  union Container *bottom;
  union Container *top;
  struct HSplitContainer *next;
  struct HSplitContainer *prev;
} HSplitContainer;

typedef union Container {
  ContainerHeader header;
  WindowContainer window;
  VSplitContainer vsplit;
  HSplitContainer hsplit;
} Container;

#define MAX_DOCKER 124
typedef struct Docker {
  Container root;
} Docker;

static Docker docker;
static bool init = false;


static void draw_rect(struct Render2D *render, Rect rect, V3 color) {
  render2d_draw_quad(render, rect.l, rect.t, rect.r - rect.l, rect.b - rect.t, 0, color);
}

static void window_container_init(WindowContainer *container, Rect rect) {
  container->header.type = WINDOW_CONTAINER;
  container->header.rect = rect;
}

static WindowContainer *window_container_create(Rect rect) {
  WindowContainer *container = (WindowContainer *)malloc(sizeof(WindowContainer));
  window_container_init(container, rect);
  return container;
}

static void vsplit_container_init(VSplitContainer *container, Rect rect) {
  container->header.type = VSPLIT_CONTAINER;
  container->header.rect = rect;
  container->position = 0.5f;
  container->right = 0;
  container->left  = 0;
  container->prev  = 0;
  container->next  = 0;
}

static VSplitContainer *vsplit_container_create(Rect rect) {
  VSplitContainer *container = (VSplitContainer *)malloc(sizeof(VSplitContainer));
  vsplit_container_init(container, rect);
  return container;
}

static void add_vsplit_to_vsplit_r(Container *container) {
  ASSERT(container->header.type == VSPLIT_CONTAINER);

  printf("vsplit to vsplit container\n");

  VSplitContainer *vsplit = vsplit_container_create(container->vsplit.right->header.rect);
  Rect rect = vsplit->header.rect;
  int half_dim = (rect.r - rect.l) / 2;
  Rect l = rect;
  Rect r = rect;
  l.r -= half_dim;
  r.l += half_dim;
  container->vsplit.right->header.rect = l;
  vsplit->left = container->vsplit.right;
  vsplit->right = (Container *)window_container_create(r);

  printf("inserting to linklst\n");

  core_cdll_insert_r(&container->vsplit, vsplit);

  printf("done\n");
}

static void add_vsplit_to_window_r(Container *container) {
  ASSERT(container->header.type ==  WINDOW_CONTAINER);
  
  printf("vsplit to window container\n");

  vsplit_container_init((VSplitContainer *)container, container->header.rect); /* Convert container to a vsplit */
  VSplitContainer *vsplit = &container->vsplit;
  
  int half_dim = (container->header.rect.r - container->header.rect.l) / 2;
  Rect rect_l = container->header.rect;
  Rect rect_r = container->header.rect;
  rect_l.r -= half_dim;
  rect_r.l += half_dim;

  vsplit->left = (Container *)window_container_create(rect_l);
  vsplit->right = (Container *)window_container_create(rect_r);
}

void docker_add_vsplit_r(Container *container) {
  switch(container->header.type) {
    case WINDOW_CONTAINER: { add_vsplit_to_window_r(container); } break;
    case VSPLIT_CONTAINER: { add_vsplit_to_vsplit_r(container); } break;
    case HSPLIT_CONTAINER: { ASSERT(true);/* TODO: Invalid code path */ } break;
  }
}

static void docker_init(struct CoreWindow *window) {
  memset(&docker, 0, sizeof(docker));
  Rect rect = (Rect){0, core_window_get_width(window), 0, core_window_get_height(window)};
  window_container_init((WindowContainer *)&docker.root, rect);
}

static void docker_update(struct CoreWindow *window) {
  (void)window;
}

static void container_render(struct Render2D *render, Container *container);

static void window_container_render(Render2D *render, WindowContainer *container) {
  V3 rect_color = v3(0.2f, 0.2f, 0.2f);
  draw_rect(render, container->header.rect, v3(0.1, 0.1, 0.1));
  int border = 8;
  Rect rect = container->header.rect;
  rect.l += border;
  rect.t += border;
  rect.r -= border;
  rect.b -= border;
  draw_rect(render, rect, rect_color);
}

static void vsplit_container_render(Render2D *render, VSplitContainer *container) {
  container_render(render, container->left);
  container_render(render, container->right);
}

static void container_render(struct Render2D *render, Container *container) {
  switch(container->header.type) {
    case WINDOW_CONTAINER: { window_container_render(render, &container->window); } break;
    case VSPLIT_CONTAINER: { vsplit_container_render(render, &container->vsplit); } break;
    case HSPLIT_CONTAINER: { ASSERT(true);/* TODO: Invalid code path */ } break;
  }
}

static void docker_render(struct Render2D *render) {
  container_render(render, &docker.root);
}

void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state) {
  (void)state, (void)window, (void)render; (void)draw_rect;
  if(!init) {
    docker_init(window);
    docker_add_vsplit_r(&docker.root);
    docker_add_vsplit_r(&docker.root);
    init = true;
  }
  docker_update(window);
  docker_render(render);
}
