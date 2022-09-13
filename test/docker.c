
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

typedef enum SplitListContainerType {
  SPLIT_LIST_V, 
  SPLIT_LIST_H, 
} SplitListContainerType;

typedef struct ContainerHeader {
  ContainerType type;
  struct SplitContainer *parent_f;
  struct SplitContainer *parent_s;
  Rect rect;
} ContainerHeader;

typedef struct SplitContainer {
  ContainerHeader c;
  
  float position;
  union Container *f;
  union Container *s;

  struct SplitListContainer *parent;
  struct SplitContainer *next;
  struct SplitContainer *prev;
} SplitContainer;

typedef struct SplitListContainer {
  SplitContainer dummy;
  SplitListContainerType type;
} SplitListContainer;

typedef struct Dockable {
  ContainerHeader c;
} Dockable;

typedef union Container {
  ContainerHeader c;

  SplitContainer split;
  SplitListContainer split_list;
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

void container_set_parent_f(Container *container, SplitContainer *parent) {
  container->c.parent_f = parent;
}

SplitContainer *container_get_parent_f(Container *container) {
  return container->c.parent_f;
}

void container_set_parent_s(Container *container, SplitContainer *parent) {
  container->c.parent_s = parent;
}

SplitContainer *container_get_parent_s(Container *container) {
  return container->c.parent_s;
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

SplitListContainer *split_list_container_push(void) {
  SplitListContainer *result = (SplitListContainer *)container_push();
  container_set_type((Container *)result, CONTAINER_SPLIT_LIST);
  result->dummy.prev = &result->dummy;
  result->dummy.next = &result->dummy;
  result->dummy.parent = result;
  return result;
}

Dockable *dockable_container_push(void) {
  Dockable *result = (Dockable *)container_push();
  container_set_type((Container *)result, CONTAINER_DOCKABLE);
  return result;
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

void split_container_list_render(struct Render2D *render, SplitListContainer *container) {
  ASSERT(container_get_type((Container *)container) == CONTAINER_SPLIT_LIST);

  SplitContainer *split = container->dummy.next;
  ASSERT(split != &container->dummy);
  container_render(render, split->f);
  container_render(render, split->s);
  
  split = split->next;
  while(split != &container->dummy) {
    ASSERT(container_get_type((Container *)split) == CONTAINER_SPLIT);
    container_render(render, split->s);
    split = split->next;
  }
}

void container_render(struct Render2D *render, Container *container) {
  switch(container_get_type(container)) {
    case CONTAINER_DOCKABLE: dockable_container_render(render, (Dockable *)container); break;
    case CONTAINER_SPLIT_LIST: split_container_list_render(render, (SplitListContainer *)container); break;
    default: {ASSERT(true); /* TODO: Invalid code path */} break;
  }
}


void split_container_set_f(SplitContainer *split, Container *container) {
  split->f = container;
  container_set_parent_f(container, split->prev);
  container_set_parent_s(container, split);
}

void split_container_set_s(SplitContainer *split, Container *container) {
    split->s = container;
    container_set_parent_f(container, split);
    container_set_parent_s(container, split->next);
}

Dockable *dockable_add_vsplit_r(Dockable *container) {
  ASSERT(container_get_type((Container *)container) == CONTAINER_DOCKABLE);

  SplitContainer *split = split_container_push();
  Dockable *result = dockable_container_push();

  SplitContainer *split_prev = container_get_parent_f((Container *)container);
  SplitContainer *split_next = container_get_parent_s((Container *)container);
  
  SplitListContainer *list = 0;

  if(!split_prev && !split_next) {
    ASSERT((Container *)container == docker.root);
    list = split_list_container_push();
    list->type = SPLIT_LIST_V;
    core_cdll_push_back(&list->dummy, split);
    split_container_set_f(split, (Container *)container);
    split_container_set_s(split, (Container *)result);
    docker.root = (Container *)list;
  } else {
    ASSERT(split_prev && split_next);
    ASSERT(split_prev->parent == split_next->parent);
    list = split_prev->parent;
    if(list->type != SPLIT_LIST_V) {
      list = split_list_container_push();
      list->type = SPLIT_LIST_V;
      core_cdll_push_back(&list->dummy, split);
      SplitContainer *parent_f = container_get_parent_f((Container *)container);
      SplitContainer *parent_s = container_get_parent_s((Container *)container);
      split_container_set_s(parent_f, (Container *)list);
      split_container_set_f(parent_s, (Container *)list);
      split_container_set_f(split, (Container *)container);
      split_container_set_s(split, (Container *)result);
    } else {
      core_cdll_insert_r(split_prev, split);
      split_container_set_f(split, split_prev->s);
      split_container_set_s(split, (Container *)result);
      split_container_set_f(split_next, (Container *)result); 
    }
  }
  split->parent = list;
  
  /* TODO: Recalculate rects size */

  return result;
}


void docker_init(struct CoreWindow *window) {
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};

  Dockable *doc1 = dockable_container_push();
  container_set_rect((Container *)doc1, window_rect);
  docker.root = (Container *)doc1;
  Dockable *doc2 = dockable_add_vsplit_r(doc1); (void)doc2;

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
