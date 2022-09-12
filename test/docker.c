
/* NOTE: Window docking algorithm testing */

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"

#include <string.h>

typedef struct Rect {
  int l, r, t, b;
} Rect;

typedef struct Rect2 {
  Rect one;
  Rect two;
} Rect2;

Rect2 rect_slice_vertical(Rect rect, float f) {
  Rect2 result;
  result.one = rect;
  result.two = rect;
  int dim = rect.r - rect.l;
  result.one.r -= (dim * (1.0f - f));
  result.two.l += (dim * f);
  return result;
}

typedef enum ContainerType {
  CONTAINER_WINDOW,
  CONTAINER_VSPLIT,
} ContainerType;

typedef struct ContainerHeader {
  ContainerType type;
  Rect rect;
} ContainerHeader;

typedef struct ContainerWindow {
  ContainerHeader header;
  struct ContainerVSplit *parent_l;
  struct ContainerVSplit *parent_r;
} ContainerWindow;

typedef struct ContainerVSplit {
  ContainerHeader header;
  float position;
  union Container *left;
  union Container *right;
  struct ContainerVSplit *next;
  struct ContainerVSplit *prev;
} ContainerVSplit;

typedef union Container {
  ContainerHeader header;
  ContainerWindow window;
  ContainerVSplit vsplit;
} Container;

#define CONTAINERS_MAX 256 
typedef struct Docker {
  Container *root;

  Container containers[CONTAINERS_MAX];
  int container_cout;
} Docker;

static Docker docker;
static bool init = false;

Container *container_push(void) {
  ASSERT(docker.container_cout < CONTAINERS_MAX);
  Container *container = docker.containers + docker.container_cout++;
  memset(container, 0, sizeof(Container));
  return container;
}

ContainerWindow *window_container_create(Rect rect) {
  ContainerWindow *container = (ContainerWindow *)container_push();
  container->header.type = CONTAINER_WINDOW;
  container->header.rect = rect;
  return container;
}

ContainerVSplit *vsplit_container_create(Rect rect) {
  ContainerVSplit *container = (ContainerVSplit *)container_push();
  container->header.type = CONTAINER_VSPLIT;
  container->header.rect = rect;
  container->position = 0.5f;
  return container;
}

void swap_window_container_for_vsplit_container(ContainerWindow *a, ContainerVSplit *b) {
  ContainerVSplit *parent_l = a->parent_l;
  ContainerVSplit *parent_r = a->parent_r;
  if(parent_l) {
    ASSERT(parent_l->header.type == CONTAINER_VSPLIT);
    ASSERT((ContainerWindow *)parent_l->right == a);
    parent_l->right = (Container *)b;
  }
  if(parent_r) {
    ASSERT(parent_r->header.type == CONTAINER_VSPLIT);
    ASSERT((ContainerWindow *)parent_r->left == a);
    parent_r->left = (Container *)b;
  }

  a->parent_l = 0;
  a->parent_r = 0;
}

void window_container_add_vsplit_r(ContainerWindow *container) {
  ASSERT(container->header.type == CONTAINER_WINDOW);

  ContainerVSplit *vsplit = vsplit_container_create(container->header.rect);
  swap_window_container_for_vsplit_container(container, vsplit);

  Rect2 rects = rect_slice_vertical(container->header.rect, vsplit->position);
  vsplit->left = (Container *)container;
  vsplit->left->window.parent_r = vsplit;

  vsplit->left->header.rect = rects.one;
  vsplit->right = (Container *)window_container_create(rects.two);
  vsplit->right->window.parent_l = vsplit;
};

void vsplit_container_add_vsplit_r(ContainerVSplit *container) {
  ASSERT(container->header.type == CONTAINER_VSPLIT);

  ContainerVSplit *vsplit = vsplit_container_create(container->header.rect);
  vsplit->prev = container;
  vsplit->next = container->next;
  container->next = vsplit;
  
  Rect2 rects = rect_slice_vertical(container->header.rect, 0.5f); 
  container->header.rect = rects.one;
  vsplit->header.rect = rects.two;
}

void container_add_vsplit_r(Container *container) {
  switch(container->header.type) {
    case CONTAINER_WINDOW: { window_container_add_vsplit_r((ContainerWindow *)container); } break;
    case CONTAINER_VSPLIT: { vsplit_container_add_vsplit_r((ContainerVSplit *)container); } break;
  }
}

static void draw_rect(struct Render2D *render, Rect rect, V3 color) {
  render2d_draw_quad(render, rect.l, rect.t, rect.r - rect.l, rect.b - rect.t, 0, color);
}

void window_container_render(struct Render2D *render, ContainerWindow *container) {
  (void)render;
  ASSERT(container->header.type == CONTAINER_WINDOW);
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

void vsplit_container_render(struct Render2D *render, ContainerVSplit *container) {
  (void)render;
  ASSERT(container->header.type == CONTAINER_VSPLIT);
}

void container_render(struct Render2D *render, Container *container) {
  switch(container->header.type) {
    case CONTAINER_WINDOW: { window_container_render(render, (ContainerWindow *)container); } break;
    case CONTAINER_VSPLIT: { vsplit_container_render(render, (ContainerVSplit *)container); } break;
  }
}

void docker_init(struct CoreWindow *window) {
  docker.container_cout = 0;
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};
  docker.root = (Container *)window_container_create(window_rect);
}

void docker_update(struct CoreWindow *window) {
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};
  docker.root->header.rect = window_rect;
}

void docker_render(struct Render2D *render) {
  container_render(render, docker.root);
}

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
