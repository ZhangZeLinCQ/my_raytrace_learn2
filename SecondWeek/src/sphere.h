﻿#ifndef SPHERE_H
#define SPHERE_H

#include "common.h"
#include "hittable.h"

class sphere : public hittable {
public:
  sphere() {}
  // Stationary Sphere
  sphere(point3 cen, double r, shared_ptr<material> m): center1(cen), radius(r), mat_ptr(m), is_moving(false) {
    auto rvec = vec3(radius, radius, radius);
    bbox = aabb(center1 - rvec, center1 + rvec);
  }
  // Moving Sphere
  sphere(point3 _center1, point3 _center2, double _radius, shared_ptr<material> _material): center1(_center1), radius(_radius), mat_ptr(_material), is_moving(true) {
    auto rvec = vec3(radius, radius, radius);
    aabb box1(_center1 - rvec, _center1 + rvec);
    aabb box2(_center2 - rvec, _center2 + rvec);
    bbox = aabb(box1, box2); // 移动物体的剖切盒涵盖整个移动轨迹

    center_vec = _center2 - _center1;
  }
  // double t_min, double t_max ==> interval
  virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override;

  aabb bounding_box() const override { return bbox; } // 构造时已生成bbx

public:
  point3 center1;
  double radius;
  shared_ptr<material> mat_ptr;
  bool is_moving;
  vec3 center_vec;
  aabb bbox;

  point3 sphere_center(double time) const {
    // Linearly interpolate from center1 to center2 according to time, where t=0 yields
    // center1, and t=1 yields center2.
    return center1 + time * center_vec;
  }
};

bool sphere::hit(const ray& r, interval ray_t, hit_record& rec) const {
  point3 center = is_moving ? sphere_center(r.time()) : center1;
  vec3 oc = r.origin() - center;
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;
  auto discriminant = half_b * half_b - a * c;

  if (discriminant < 0) return false;
  auto sqrtd = sqrt(discriminant);
  auto root = (-half_b - sqrtd) / a;
  if (!ray_t.surrounds(root)) {
    root = (-half_b + sqrtd) / a;
    if (!ray_t.surrounds(root))
      return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  vec3 outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.mat_ptr = mat_ptr;

  return true;
}

#endif