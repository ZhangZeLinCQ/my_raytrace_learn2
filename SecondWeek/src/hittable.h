#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"
#include "ray.h"

class material;

struct hit_record { 
  point3 p;
  vec3 normal; // 击中处法向量
  shared_ptr<material> mat_ptr;
  double t;
  bool front_face; 

  inline void set_face_normal(const ray& r, const vec3& outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal; // 小于0光线来自外部，大于0光线来自内部
  }
};

class hittable {
public:
  //  the hit only “counts” if t_min < t < t_max
  virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif