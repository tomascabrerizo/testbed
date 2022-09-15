
/* NOTE: Window docking algorithm testing */

#include "core.h"
#include "core_math.h"
#include "core_ds.h"
#include "renderman.h"

#include <string.h>

typedef struct F_Rect {
  float l, r, t, b;
} F_Rect;

void f_rect_slice_v(F_Rect rect, F_Rect *l, F_Rect *r, float position) {
  *l = rect;
  *r = rect;
  float length = rect.r - rect.l; 
  l->r -= (1.0f - position)*length;
  r->l += position*length;
}

typedef enum F_ParentContainerType {
  F_PARENT_ROOT,
  F_PARENT_SPLIT,
} F_ParentContainerType;

typedef enum F_ChildContainerType {
  F_CHILD_DRAGGABLE,
  F_CHILD_SPLIT_LIST,
} F_ChildContainerType;

typedef struct F_ContainerHeader {
  F_Rect rect;
} F_ContainerHeader;

typedef struct F_ParentContainer {
  F_ContainerHeader c;
  F_ParentContainerType type;
  struct F_ChildContainer *f;
  struct F_ChildContainer *s;
} F_ParentContainer;

typedef struct F_ChildContainer {
  F_ContainerHeader c;
  F_ChildContainerType type;
  struct F_ParentContainer *f;
  struct F_ParentContainer *s;
} F_ChildContainer;

typedef struct F_RootContainer {
  F_ParentContainer c;
} F_RootContainer;

typedef struct F_SplitContainer {
  F_ParentContainer c;
  float position;
  struct F_SplitListContainer *parent;
  struct F_SplitContainer *next;
  struct F_SplitContainer *prev;
} F_SplitContainer;

typedef struct F_SplitListContainer {
  F_ChildContainer c;
  bool vertical;
  F_SplitContainer dummy;
} F_SplitListContainer;

typedef struct F_DraggableContainer {
  F_ChildContainer c;
} F_DraggableContainer;

typedef union F_Container {
  F_ContainerHeader    header;
  F_ParentContainer    parent;
  F_ChildContainer     child;
  F_RootContainer      root;
  F_SplitContainer     split;
  F_SplitListContainer split_list;
  F_DraggableContainer draggable;
} F_Container;


#define F_DOCKER_CONTAINERS_MAX 256
typedef struct F_Docker {
  F_RootContainer *root;
  int width;
  int height;

  F_Container buffer[F_DOCKER_CONTAINERS_MAX];
  int buffer_count;
} F_Docker;

static F_Docker docker;

void f_container_header_set_rect(F_ContainerHeader *header, F_Rect rect) {
  header->rect = rect;
}

void f_parent_container_set_rect(F_ParentContainer *parent, F_Rect rect) {
  f_container_header_set_rect(&parent->c, rect);
}

void f_child_container_set_rect(F_ChildContainer *child, F_Rect rect) {
  f_container_header_set_rect(&child->c, rect);
}

void f_root_container_set_rect(F_RootContainer *root, F_Rect rect) {
  f_parent_container_set_rect(&root->c, rect);
}

void f_split_container_set_rect(F_SplitContainer *split, F_Rect rect) {
  f_parent_container_set_rect(&split->c, rect);
}

void f_split_list_container_set_rect(F_SplitListContainer *split_list, F_Rect rect) {
  f_child_container_set_rect(&split_list->c, rect);
}

void f_draggable_container_set_rect(F_DraggableContainer *drag, F_Rect rect) {
  f_child_container_set_rect(&drag->c, rect);
}

F_Rect f_container_header_get_rect(F_ContainerHeader *header) {
  return header->rect;
}

F_Rect f_parent_container_get_rect(F_ParentContainer *parent) {
  return f_container_header_get_rect(&parent->c);
}

F_Rect f_child_container_get_rect(F_ChildContainer *child) {
  return f_container_header_get_rect(&child->c);
}

F_Rect f_root_container_get_rect(F_RootContainer *root) {
  return f_parent_container_get_rect(&root->c);
}

F_Rect f_split_container_get_rect(F_SplitContainer *split) {
  return f_parent_container_get_rect(&split->c);
}

F_Rect f_split_list_container_get_rect(F_SplitListContainer *split_list) {
  return f_child_container_get_rect(&split_list->c);
}

F_Rect f_draggable_container_get_rect(F_DraggableContainer *drag) {
  return f_child_container_get_rect(&drag->c);
}

F_Rect *f_container_header_get_rect_p(F_ContainerHeader *header) {
  return &header->rect;
}

F_Rect *f_parent_container_get_rect_p(F_ParentContainer *parent) {
  return f_container_header_get_rect_p(&parent->c);
}

F_Rect *f_child_container_get_rect_p(F_ChildContainer *child) {
  return f_container_header_get_rect_p(&child->c);
}

F_Rect *f_root_container_get_rect_p(F_RootContainer *root) {
  return f_parent_container_get_rect_p(&root->c);
}

F_Rect *f_split_container_get_rect_p(F_SplitContainer *split) {
  return f_parent_container_get_rect_p(&split->c);
}

F_Rect *f_split_list_container_get_rect_p(F_SplitListContainer *split_list) {
  return f_child_container_get_rect_p(&split_list->c);
}

F_Rect *f_draggable_container_get_rect_p(F_DraggableContainer *drag) {
  return f_child_container_get_rect_p(&drag->c);
}


/* NOTE: To simplify the API functions must only set child containers (automaticaly setting the parents) */
void f_parent_container_set_child_f(F_ParentContainer *parent, F_ChildContainer *child) {
  parent->f = child;
  child->s = parent;
}

void f_parent_container_set_child_s(F_ParentContainer *parent, F_ChildContainer *child) {
  parent->s = child;
  child->f = parent;
}

void f_root_container_set_child(F_RootContainer *root, F_ChildContainer *child) {
  f_parent_container_set_child_f(&root->c, child);
  f_parent_container_set_child_s(&root->c, child);
}

F_ChildContainer *f_root_container_get_child(F_RootContainer *root) {
  ASSERT(root->c.f == root->c.s);
  return root->c.f;
}

void f_root_container_set_draggable(F_RootContainer *root, F_DraggableContainer *drag) {
  f_root_container_set_child(root, &drag->c);
  f_draggable_container_set_rect(drag, f_root_container_get_rect(root));
}

void f_root_container_set_split_list(F_RootContainer *root, F_SplitListContainer *split_list) {
  f_root_container_set_child(root, &split_list->c);
}

void f_split_container_set_child_f(F_SplitContainer *split, F_ChildContainer *child) {
  f_parent_container_set_child_f(&split->c, child);
  split->prev->c.s = child;
  child->f = &split->prev->c;
}

void f_split_container_set_child_s(F_SplitContainer *split, F_ChildContainer *child) {
  f_parent_container_set_child_s(&split->c, child);
  split->next->c.f = child;
  child->s = &split->next->c;
}

F_Container *f_container_create(void) {
  ASSERT(docker.buffer_count < F_DOCKER_CONTAINERS_MAX);
  F_Container *result = docker.buffer + docker.buffer_count++;
  memset(result, 0, sizeof(F_Container));
  return result;
}

F_ParentContainer *f_parent_container_create(F_ParentContainerType type) {
  F_ParentContainer *result = (F_ParentContainer *)f_container_create();
  result->type = type;
  return result;
}

F_ChildContainer *f_child_container_create(F_ChildContainerType type) {
  F_ChildContainer *result = (F_ChildContainer *)f_container_create();
  result->type = type;
  return result;
}

F_RootContainer *f_root_container_create(void) {
  F_RootContainer *result = (F_RootContainer *)f_parent_container_create(F_PARENT_ROOT);
  F_Rect rect = (F_Rect){ 0.0f, 1.0f, 0.0f, 1.0f };
  f_root_container_set_rect(result, rect);
  return result;
}

F_SplitContainer *f_split_container_create(float position) {
  F_SplitContainer *result = (F_SplitContainer *)f_parent_container_create(F_PARENT_SPLIT);
  result->position = position;
  return result;
}

F_SplitListContainer *f_split_list_container_create(bool vertical) {
  F_SplitListContainer *result = (F_SplitListContainer *)f_child_container_create(F_CHILD_SPLIT_LIST);
  result->vertical = vertical;
  core_cdll_init(&result->dummy);
  static F_ChildContainer child_dummy;
  f_split_container_set_child_f(&result->dummy, &child_dummy);
  f_split_container_set_child_s(&result->dummy, &child_dummy);
  result->dummy.c.type = F_PARENT_SPLIT; 
  result->dummy.parent = result;
  return result;
}

F_DraggableContainer *f_draggable_container_create(void) {
  F_DraggableContainer *result = (F_DraggableContainer *)f_child_container_create(F_CHILD_DRAGGABLE);
  return result;
}

void f_split_list_push_back(F_SplitListContainer *split_list, F_SplitContainer *split) {
  core_cdll_push_back(&split_list->dummy, split); 
  split->parent = split_list;
}

void f_split_list_insert_r(F_SplitContainer *a, F_SplitContainer *b) {
  ASSERT(a->parent);
  ASSERT(a->parent->c.type == F_CHILD_SPLIT_LIST);
  core_cdll_insert_r(a, b);
  b->parent = a->parent;
}

void f_split_container_calculate_size(F_SplitContainer *split) {
  F_Rect rect_f = f_child_container_get_rect(split->c.f);
  F_Rect rect_s = f_child_container_get_rect(split->c.s);
  F_Rect rect = (F_Rect) {rect_f.l, rect_s.r, rect_f.t, rect_s.b};
  f_split_container_set_rect(split, rect);
}

F_DraggableContainer *f_root_container_add_split_vr(F_RootContainer *root, F_SplitContainer *split) {
  F_DraggableContainer *drag = f_draggable_container_create();
  F_ChildContainer *child = f_root_container_get_child(root);
  ASSERT(child->type == F_CHILD_DRAGGABLE);
  /* NOTE: Setting containers structure */
  F_SplitListContainer *split_list = f_split_list_container_create(true);
  f_split_list_push_back(split_list, split);
  f_root_container_set_split_list(root, split_list);
  f_split_container_set_child_f(split, child);
  f_split_container_set_child_s(split, &drag->c);
  /* NOTE: Setting containers dimensions */
  F_Rect root_rect = f_root_container_get_rect(root);
  F_Rect l, r;
  f_rect_slice_v(root_rect, &l, &r, split->position);
  f_split_list_container_set_rect(split_list, root_rect);
  f_child_container_set_rect(child, l);
  f_draggable_container_set_rect(drag, r);
  f_split_container_calculate_size(split);
  return drag;
}


F_DraggableContainer *f_split_container_add_split_vr(F_SplitContainer *a, F_SplitContainer *b) {
  /* TODO: Manage the path were SplitList is and horizontal container */
  F_DraggableContainer *drag = f_draggable_container_create();
  /* NOTE: Setting containers structure */
  f_split_list_insert_r(a, b);
  f_split_container_set_child_f(b, a->c.s);
  f_split_container_set_child_s(b, &drag->c);
  /* NOTE: Setting containers dimensions */
  F_Rect rect = f_child_container_get_rect(b->prev->c.s);
  F_Rect l, r;
  f_rect_slice_v(rect, &l, &r, b->position);
  f_child_container_set_rect(b->prev->c.s, l);
  f_draggable_container_set_rect(drag, r);
  f_split_container_calculate_size(b);
  f_split_container_calculate_size(b->prev);
  f_split_container_calculate_size(b->next);
  return drag;
}

F_DraggableContainer *f_draggable_container_split_vr(F_DraggableContainer *draggable) {
  F_DraggableContainer *result = 0;
  F_SplitContainer *split = f_split_container_create(0.5f);

  F_ParentContainer *parent_f = draggable->c.f;
  F_ParentContainer *parent_s = draggable->c.s;
  ASSERT(parent_f && parent_s);
  ASSERT(parent_f->type == parent_f->type);

  switch(parent_f->type) {
    case F_PARENT_ROOT: {
      result = f_root_container_add_split_vr((F_RootContainer *)parent_f, split);
    } break;
    case F_PARENT_SPLIT: {
      result = f_split_container_add_split_vr((F_SplitContainer *)parent_f, split);
    } break;
  }
  ASSERT(result);
  return result;
}

F_DraggableContainer *f_draggable_container_split_vl(F_DraggableContainer *draggable) {
  /* TODO: Function not implemented */
  (void)draggable;
  return 0;
}

F_DraggableContainer *f_draggable_container_split_ht(F_DraggableContainer *draggable) {
  /* TODO: Function not implemented */
  (void)draggable;
  return 0;
}

F_DraggableContainer *f_draggable_container_split_hb(F_DraggableContainer *draggable) {
  /* TODO: Function not implemented */
  (void)draggable;
  return 0;
}

static void draw_rect(struct Render2D *render, int l, int r, int t, int b, V3 color) {
  render2d_draw_quad(render, l, t, r - l, b - t, 0, color);
}


void f_draggable_container_render(struct Render2D *render, F_DraggableContainer *drag);
void f_split_list_container_render(struct Render2D *render, F_SplitListContainer *split_list);

void f_child_container_render(struct Render2D *render, F_ChildContainer *child) {
  switch(child->type) {
    case F_CHILD_DRAGGABLE: { f_draggable_container_render(render, (F_DraggableContainer *)child); } break;
    case F_CHILD_SPLIT_LIST: { f_split_list_container_render(render, (F_SplitListContainer *)child); } break;
  }
}

void f_draggable_container_render(struct Render2D *render, F_DraggableContainer *drag) {
  F_Rect rect = f_draggable_container_get_rect(drag);
  int l = (int)(rect.l * docker.width);
  int r = (int)(rect.r * docker.width);
  int t = (int)(rect.t * docker.height);
  int b = (int)(rect.b * docker.height);
  draw_rect(render, l, r, t, b, v3(0.1, 0.1, 0.1));
  int border = 8;
  l += border;
  t += border;
  r -= border;
  b -= border;
  draw_rect(render, l, r, t, b, v3(0.2f, 0.2f, 0.2f));
}

void f_split_list_container_render(struct Render2D *render, F_SplitListContainer *split_list) {
  F_SplitContainer *split = split_list->dummy.next;
  ASSERT(split != &split_list->dummy);
  f_child_container_render(render, split->c.f);
  f_child_container_render(render, split->c.s);
  split = split->next;
  while(split != &split_list->dummy) {
    f_child_container_render(render, split->c.s);
    split = split->next;
  }
}

void f_root_container_render(struct Render2D *render, F_RootContainer *root) {
  F_ChildContainer *child = f_root_container_get_child(root);
  switch(child->type) {
    case F_CHILD_DRAGGABLE: { f_draggable_container_render(render, (F_DraggableContainer *)child); } break;
    case F_CHILD_SPLIT_LIST: { f_split_list_container_render(render, (F_SplitListContainer *)child); } break;
  }
}

void docker_init(void) {
  docker.root = f_root_container_create();
  F_DraggableContainer *drag = f_draggable_container_create();
  f_root_container_set_draggable(docker.root, drag);
  F_DraggableContainer *drag1 = f_draggable_container_split_vr(drag); (void)drag1;
  F_DraggableContainer *drag2 = f_draggable_container_split_vr(drag); (void)drag2;
  printf("time to render\n");
}

void docker_update(struct CoreWindow *window) {
  docker.width  = core_window_get_width(window);
  docker.height = core_window_get_height(window);
}

void docker_render(struct Render2D *render) {
  f_root_container_render(render, docker.root);
}

static bool init = false;
void frames_dock_test(struct CoreWindow *window, struct Render2D *render, struct CoreState *state) {
  (void)state, (void)window, (void)render;
  if(!init) {
    docker_init();
    init = true;
  }

  docker_update(window);
  docker_render(render);
}
