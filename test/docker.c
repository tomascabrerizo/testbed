
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

void print_rect(Rect rect) {
  printf("l:%d, r:%d, t:%d, b:%d\n", rect.l, rect.r, rect.t, rect.b);
}

typedef enum ContainerType {
  CONTAINER_DOCKABLE,
  CONTAINER_SPLIT,
  CONTAINER_SPLIT_LIST,
} ContainerType;

typedef struct ContainerHeader {
  ContainerType type;
  Rect rect;
  struct SplitContainer *parent_l;
  struct SplitContainer *parent_r;
} ContainerHeader;

typedef struct SplitContainer {
  ContainerHeader c;
  
  float position;
  bool vertical;
  union Container *r;
  union Container *l;

  struct SplitContainer *next;
  struct SplitContainer *prev;
} SplitContainer;

typedef struct SplitContainerList {
  SplitContainer dummy;
} SplitContainerList;

typedef struct Dockable {
  ContainerHeader c;

} Dockable;

typedef union Container {
  ContainerHeader c;

  SplitContainer split;
  SplitContainerList split_list;
  Dockable dock;
} Container;

#define CONTAINERS_MAX 256
typedef struct Docker {

  Container *root;

  Container containers[CONTAINERS_MAX];
  uint64_t containers_cout;
} Docker; 

static Docker docker;

static void draw_rect(struct Render2D *render, Rect rect, V3 color) {
  render2d_draw_quad(render, rect.l, rect.t, rect.r - rect.l, rect.b - rect.t, 0, color);
}

void container_set_parent_l(Container *container, SplitContainer *parent) {
  container->c.parent_l = parent;
}

SplitContainer *container_get_parent_l(Container *container) {
  return container->c.parent_l;
}

void container_set_parent_r(Container *container, SplitContainer *parent) {
  container->c.parent_r = parent;
}

SplitContainer *container_get_parent_r(Container *container) {
  return container->c.parent_r;
}

void container_set_rect(Container *container, Rect rect) {
  container->c.rect = rect;
}

Rect container_get_rect(Container *container) {
  return container->c.rect;
}

void container_set_type(Container *container, ContainerType type) {
  container->c.type = type;
}

ContainerType container_get_type(Container *container) {
  return container->c.type;
}

Container *container_push(void) {
  ASSERT(docker.containers_cout < CONTAINERS_MAX);
  Container *result = docker.containers + docker.containers_cout++;
  memset(result, 0, sizeof(Container));
  return result;
}

SplitContainer *split_container_push(void) {
  SplitContainer *result = (SplitContainer *)container_push();
  container_set_type((Container *)result, CONTAINER_SPLIT);
  return result;
}

SplitContainerList *split_list_container_push(void) {
  SplitContainerList *result = (SplitContainerList *)container_push();
  container_set_type((Container *)result, CONTAINER_SPLIT_LIST);
  return result;
}

Dockable *dockable_container_push(void) {
  Dockable *result = (Dockable *)container_push();
  container_set_type((Container *)result, CONTAINER_DOCKABLE);
  return result;
}

Dockable *dockable_container_add_split_r(Dockable *container) {
  SplitContainer *split = split_container_push();
  if((Container *)container == docker.root) {
    docker.root = (Container *)split;
  }

  SplitContainer *parent_l = container_get_parent_l((Container *)container);
  SplitContainer *parent_r = container_get_parent_r((Container *)container);
  if(parent_l) parent_l->r = (Container *)split;
  if(parent_r) parent_r->l = (Container *)split;

  Dockable *result = dockable_container_push();
  container_set_parent_l((Container *)result, split);
  container_set_parent_r((Container *)container, split);
  split->l = (Container *)container;
  split->r = (Container *)result;
  
  split->position = 0.5f;

  Rect rect = container_get_rect((Container *)container);
  Rect2 rects = rect_slice_vertical(rect, split->position); (void)rects;
  container_set_rect((Container *)split, rect);
  container_set_rect((Container *)container, rects.one);
  container_set_rect((Container *)result, rects.two);

  return result;
}

Dockable *split_container_add_split_r(SplitContainer *container) {
  SplitContainer *split = split_container_push();
  Container *child = container->r;
  
  split->next = container->next;
  split->prev = container;
  split->prev->next = split;
  
  Dockable *result = dockable_container_push();
  split->l = child;
  split->r = (Container *)result;
  container_set_parent_r(child, split);
  container_set_parent_l((Container *)result, split);
  container_set_parent_r((Container *)result, split->next);
  if(split->next) split->next->l = (Container *)result;
  
  split->position = 0.5f;

  Rect child_rect = container_get_rect(child);
  Rect2 rects = rect_slice_vertical(child_rect, split->position);
  
  container_set_rect(child, rects.one);
  container_set_rect((Container *)result, rects.two);
  
  Rect container_rect = container_get_rect((Container *)container);
  container_rect.r -= (rects.two.r - rects.two.l);
  container_set_rect((Container *)container, container_rect);

  return result;
}

Dockable *container_add_split_r(Container *container) {
  switch(container_get_type(container)) {
    case CONTAINER_DOCKABLE: return dockable_container_add_split_r((Dockable *)container);
    case CONTAINER_SPLIT: return split_container_add_split_r((SplitContainer *)container);
    case CONTAINER_SPLIT_LIST: ASSERT(true); /* TODO: Invalid code path */;
  }
  ASSERT(true); /* TODO: Invalid code path */
  return 0;
}

void container_render(struct Render2D *render, Container *container);

void dockable_container_render(struct Render2D *render, Dockable *container) {
  ASSERT(container_get_type((Container *)container) == CONTAINER_DOCKABLE);
  V3 rect_color = v3(0.2f, 0.2f, 0.2f);
  Rect rect = container_get_rect((Container *)container);
  draw_rect(render, rect, v3(0.1, 0.1, 0.1));
  int border = 8;
  rect.l += border;
  rect.t += border;
  rect.r -= border;
  rect.b -= border;
  draw_rect(render, rect, rect_color);
}

void split_container_render(struct Render2D *render, SplitContainer *container) {
  ASSERT(container_get_type((Container *)container) == CONTAINER_SPLIT);
  container_render(render, container->l);
  container_render(render, container->r);

  SplitContainer *next = container->next;
  while(next) {
    ASSERT(container_get_type((Container *)next) == CONTAINER_SPLIT);
    container_render(render, next->r);
    next = next->next;
  }
}

void container_render(struct Render2D *render, Container *container) {
  switch(container_get_type(container)) {
    case CONTAINER_DOCKABLE: dockable_container_render(render, (Dockable *)container); break;
    case CONTAINER_SPLIT: split_container_render(render, (SplitContainer *)container); break;
    case CONTAINER_SPLIT_LIST: ASSERT(true) /* TODO: Invalid code path */; break;
  }
}

void docker_init(struct CoreWindow *window) {
  docker.root = (Container *)dockable_container_push();
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};
  container_set_rect(docker.root, window_rect);
  
  container_add_split_r(docker.root);
  container_add_split_r(docker.root);
  Dockable *container = container_add_split_r(docker.root); (void)container;
  container_add_split_r((Container *)container);
}

void docker_update(struct CoreWindow *window) {
  /* NOTE: Update root container rect on resize events */
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};
  container_set_rect(docker.root, window_rect);
}

void docker_render(struct Render2D *render) {
  container_render(render, docker.root);
}

static bool init = false;
void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state) {
  (void)state, (void)window, (void)render;
  if(!init) {
    docker_init(window);
    init = true;
  }
  docker_update(window);
  docker_render(render);
}
