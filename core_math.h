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

static inline V3 v3_normalize(V3 a) {
  float len = v3_length(a);
  if(len != 0) {
    float inv_len = 1.0f / len;
    a = v3_scale(a, inv_len);
  }
  return a;
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

static inline M4 m4_ortho(float l, float r, float t, float b, float n, float f) {
  float rml = 1.0f / (r - l);
  float tmb = 1.0f / (t - b);
  float fmn = 1.0f / (f - n);
  M4 m = (M4){{2*rml, 0    , 0  , -((r+l)*rml), 
               0    , 2*tmb, 0  , -((t+b)*tmb),
               0    , 0    , fmn, -(n*fmn)    ,
               0    , 0    , 0  , 1           }};
  return m; 
}

static inline M4 m4_perspective(float l, float r, float t, float b, float n, float f) {
  float _2n = 2 * n;
  float rml = 1.0f / (r - l);
  float rpl = r + l;
  float tmb = 1.0f / (t - b);
  float tpb = t + b;
  float fmn = 1.0f / (f - n);
  float fpn = f + n;
  M4 m = (M4){{_2n*rml,  0     , rpl*rml   ,  0          , 
                0     , _2n*tmb, tpb*tmb   ,  0          ,
                0     ,  0     , -(fpn/fmn), -(_2n*f*fmn),
                0     ,  0     , -1        ,  0          }};
  return m; 
}

static inline M4 m4_perspective2(float fov, float aspect, float n, float f) {
    // float ymax = n * tanf(fov * 3.14159265359f / 360.0f);
    float ymax = n * tanf(fov * 0.5f);
    float xmax = ymax * aspect;
    return m4_perspective(-xmax, xmax, -ymax, ymax, n, f);
}

static inline M4 m4_lookat(V3 r, V3 u, V3 v, V3 t) {
  V3 neg_t = v3_neg(t);
  M4 m = (M4){{r.x, r.y, r.z, v3_dot(neg_t, r), 
               u.x, u.y, u.z, v3_dot(neg_t, u),
               v.x, v.y, v.z, v3_dot(neg_t, v),
               0, 0, 0,                     1}};
  return m; 
}

static inline M4 m4_lookat2(V3 pos, V3 target, V3 up) {
  /* TODO: Test this code culd be wrong */
  V3 v = v3_normalize(v3_sub(target, pos));
  V3 r = v3_normalize(v3_cross(up, v));
  return m4_lookat(r, up, v, target);
}

#endif /* CORE_MATH_H */
