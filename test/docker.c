
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

/* NOTE: To simplify the API functions must only set child containers (automaticaly setting the parents) */

typedef struct F_ContainerHeader {
  F_Rect rect;
} F_ContainerHeader;

typedef struct F_ParentContainer {
  F_ContainerHeader c;
  struct F_ChildContainer *f;
  struct F_ChildContainer *s;
} F_ParentContainer;

typedef struct F_ChildContainer {
  F_ContainerHeader c;
  struct F_ParentContainer *f;
  struct F_ParentContainer *s;
} F_ChildContainer;

typedef struct F_RootContainer {
  F_ParentContainer c;
} F_RootContainer;

typedef struct F_SplitContainer {
  F_ParentContainer c;
  float position;
  struct F_SplitContainer *next;
  struct F_SplitContainer *prev;
} F_SplitContainer;

typedef struct F_SlpitListContainer {
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
  F_Container buffer[F_DOCKER_CONTAINERS_MAX];
  int buffer_count;
} F_Docker;

static F_Docker docker;

void f_docker_container_init(F_Container *container) {
  memset(container, 0, sizeof(F_Container));
}

F_Container *f_docker_container_push(void) {
  ASSERT(docker.buffer_count < F_DOCKER_CONTAINERS_MAX);
  F_Container *result = docker.buffer + docker.buffer_count++;
  f_docker_container_init(result);
  return result;
}

void docker_init(void) {
  (void)docker; 
}

void docker_update(struct CoreWindow *window) {
  (void)window;
}

void docker_render(struct Render2D *render) {
  (void)render;
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
