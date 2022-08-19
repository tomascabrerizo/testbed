#ifndef CORE_MATH_H
#define CORE_MATH_H

#include <math.h>

typedef struct V3 {
  float x;
  float y;
  float z;
} V3;

static inline V3 v3(float x, float y,  float z) {
  return (V3){x, y, z};
}

static inline V3 v3_add(V3 a, V3 b) {
  return (V3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline V3 v3_sub(V3 a, V3 b) {
  return (V3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline V3 v3_neg(V3 a) {
  return (V3){-a.x, -a.y, -a.z};
}

static inline V3 v3_scale(V3 a, float s) {
  return (V3){a.x*s, a.y*s, a.z*s};
}

static inline float v3_dot(V3 a, V3 b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline V3 v3_cross(V3 a, V3 b) {
  return (V3){a.y*b.z - a.z*b.y,
              a.z*b.x - a.x*b.z,
              a.x*b.y - a.y*b.x};
}

static inline float v3_length_sqr(V3 a) {
  return v3_dot(a, a);
}

static inline float v3_length(V3 a) {
  return sqrtf(v3_length_sqr(a));
}

typedef struct M4 {
  float m[16];
} M4;

static inline M4 m4_identity() {
  M4 m = (M4){{1, 0, 0, 0, 
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}};
  return m; 
}


static inline M4 m4_ortho() {
  M4 m = (M4){{1, 0, 0, 0, 
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}};
  return m; 
}

static inline M4 m4_perspective() {
  M4 m = (M4){{1, 0, 0, 0, 
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}};
  return m; 
}

static inline M4 m4_lookat() {
  M4 m = (M4){{1, 0, 0, 0, 
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}};
  return m; 
}

#endif /* CORE_MATH_H */
