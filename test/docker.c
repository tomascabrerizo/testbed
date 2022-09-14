
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
  float expand; /* NOTE: only dockables and SplitList use expand */
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

float container_get_expand(Container *container) {
  return container->c.expand; 
}

void container_set_expand(Container *container, float expand) {
  container->c.expand = expand;
}

void container_set_rect_and_expand(Container *container, Rect rect) {
  ASSERT((container_get_type(container) == CONTAINER_DOCKABLE) | (container_get_type(container) == CONTAINER_SPLIT_LIST));
  ASSERT(container_get_parent_f(container)->parent == container_get_parent_s(container)->parent);
  
  SplitListContainer *split_list = container_get_parent_f(container)->parent;
  
  Rect split_list_rect = container_get_rect((Container *)split_list); 
  
  float expand = 0.0f;
  switch(split_list->type) {
    case SPLIT_LIST_V: {
      expand = (float)(rect.r - rect.l)/(float)(split_list_rect.r - split_list_rect.l); 
    } break;
    case SPLIT_LIST_H: {
      expand = (float)(rect.b - rect.t)/(float)(split_list_rect.b - split_list_rect.t); 
    } break;
    default: { ASSERT(true); /* TODO: Ivalid code path */} break;
  }
  
  print_rect(rect);
  print_rect(split_list_rect);

  container_set_expand(container, expand);
  container_set_rect(container, rect);

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
  core_cdll_init(&result->dummy);
  static Container dummy_container;
  container_set_type((Container *)&result->dummy, CONTAINER_SPLIT);
  split_container_set_f(&result->dummy, &dummy_container);
  split_container_set_s(&result->dummy, &dummy_container);
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

void split_container_recalculate_rect(SplitContainer *split) {
  Rect f = container_get_rect(split->f);
  Rect s = container_get_rect(split->s);
  Rect rect = (Rect){ f.l, s.r, f.t, s.b };
  container_set_rect((Container *)split, rect);
}

Dockable *dockable_add_vsplit_r(Dockable *container) {
  ASSERT(container_get_type((Container *)container) == CONTAINER_DOCKABLE);

  SplitContainer *split = split_container_push();
  split->position = 0.5f;
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
  container_set_rect((Container *)split, container_get_rect((Container *)container));
  Rect rect = container_get_rect((Container *)split);
  print_rect(rect);
  Rect2 rects = rect_slice_vertical(rect, split->position);
  
  container_set_rect_and_expand((Container *)split->f, rects.one);
  container_set_rect_and_expand((Container *)split->s, rects.two);

  split_container_recalculate_rect(split->prev);
  split_container_recalculate_rect(split->next);
  
  return result;
}

void container_update(Container *container);

void dockable_container_update(Dockable *dockable) {
  (void)dockable; /* TODO: Not implemented */
}

void split_container_update(SplitContainer *split) {
  (void)split;
}

void split_list_container_update(SplitListContainer *split_list) {
  SplitContainer *split = split_list->dummy.next;
  while(split != &split_list->dummy) {
    split_container_update(split);
    split = split->next;
  }
}

void container_update(Container *container) {
  switch(container_get_type(container)) {
    case CONTAINER_DOCKABLE: dockable_container_update((Dockable *)container); break;
    case CONTAINER_SPLIT: split_container_update((SplitContainer *)container); break;
    case CONTAINER_SPLIT_LIST: split_list_container_update((SplitListContainer *)container); break;
  }
}

void docker_init(struct CoreWindow *window) {
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};

  Dockable *doc1 = dockable_container_push();
  container_set_rect((Container *)doc1, window_rect);
  docker.root = (Container *)doc1;
  Dockable *doc2 = dockable_add_vsplit_r(doc1); (void)doc2;
  Dockable *doc3 = dockable_add_vsplit_r(doc1); (void)doc3;

}

void docker_update(struct CoreWindow *window) {
  /* NOTE: Update root container rect on resize events */
  Rect window_rect = (Rect){0, core_window_get_width(window), 0,  core_window_get_height(window)};
  container_set_rect(docker.root, window_rect);
  container_update(docker.root);
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
