#ifndef MATERIAL_H
#define MATERIAL_H

#include "common.h"
#include "texture.h"

struct hit_record;

// scattered：生成一个散射光线scattered
// attenuation：发生散射时光线的衰减attenuation（颜色）
class material {
public:
  // 发出的光：默认不发光，发光材质另写
  virtual color emitted(double u, double v, const point3& p) const { 
    return color(0, 0, 0); // not all the non-emitting materials implement emitted(); the base class return black.
  }

  virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};

// Lambertian漫反射材质
class lambertian : public material {
public:
  lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {} // 可以传入颜色 转换为材质
  lambertian(shared_ptr<texture> a) : albedo(a) {} // 直接传入材质

  virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    // 此值可能为0
    //auto scatter_direction = rec.normal + random_in_unit_sphere();
    auto scatter_direction = rec.normal + random_unit_vector();
    //auto scatter_direction = rec.normal + random_in_hemisphere(rec.normal);

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero())
      scatter_direction = rec.normal;
    
    scattered = ray(rec.p, scatter_direction, r_in.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p); // 在此获得材质上某位置的颜色
    return true;
  }

public:
  shared_ptr<texture> albedo; // 从单一颜色变为材质（根据位置获得颜色等数据）
};

// 金属材质
class metal : public material {
public:
  metal(const color& a, double f = 0) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    //scattered = ray(rec.p, reflected);
    scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
  }

public:
  color albedo;
  double fuzz;
};

// 介质（可发生折射的材料）
class dielectric : public material {
public:
  dielectric(double index_of_refraction) : ir(index_of_refraction) {}

  virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    attenuation = color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir; // 内部或外部

    vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0; // >1 表示发生全反射，不产生折射
    vec3 direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
      direction = reflect(unit_direction, rec.normal);
    else
      direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = ray(rec.p, direction, r_in.time());
    return true;
  }

public:
  double ir; // Index of Refraction

private:
  static double reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
  }
};

// 慢反射光、一种发光材质
class diffuse_light : public material {
public:
  diffuse_light(shared_ptr<texture> a) : emit(a) {}
  diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

  // 不处理照上去的光线
  bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
    return false;
  }

  color emitted(double u, double v, const point3& p) const override {
    return emit->value(u, v, p);
  }

private:
  shared_ptr<texture> emit;
};
#endif