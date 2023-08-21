#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"
#include "ray.h"
#include "aabb.h"

class material;

struct hit_record { 
  point3 p;
  vec3 normal; // 击中处法向量
  shared_ptr<material> mat_ptr;
  double t;

  // 光线和物体击中点的表面坐标uv
  double u;
  double v;
  bool front_face; 

  inline void set_face_normal(const ray& r, const vec3& outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal; // 小于0光线来自外部，大于0光线来自内部
  }
};

class hittable {
public:
  virtual ~hittable() = default;

  //  the hit only “counts” if t_min < t < t_max
  virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

  virtual aabb bounding_box() const = 0; // 所有可碰撞物体要实现aab方法以支持bvh查询
};

// 实例化：平移物体
class translate : public hittable {
public:
  translate(shared_ptr<hittable> p, const vec3& displacement) : object(p), offset(displacement) {
    bbox = object->bounding_box() + offset;
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    // 原型还是在原位只有一个，发现相交后移回原位计算交点（再移回去）和散射光线

    // Move the ray backwards by the offset
    ray offset_r(r.origin() - offset, r.direction(), r.time());

    // Determine where (if any) an intersection occurs along the offset ray
    if (!object->hit(offset_r, ray_t, rec))
      return false;

    // Move the intersection point forwards by the offset
    rec.p += offset;

    return true;
  }

  aabb bounding_box() const override { return bbox; }

private:
  shared_ptr<hittable> object;
  vec3 offset;
  aabb bbox;
};
#endif