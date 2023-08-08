#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

class texture {
public:
  virtual ~texture() = default;

  virtual color value(double u, double v, const point3& p) const = 0; // 纹理(目前为颜色)可以通过uv(纹理坐标texture coordinates) 或 point(三维)找到
};

// 单一颜色的纹理 任何地方都返回这个颜色
class solid_color : public texture {
public:
  solid_color(color c) : color_value(c) {}

  solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

  color value(double u, double v, const point3& p) const override {
    return color_value;
  }

private:
  color color_value;
};

// solid (or spatial) texture 固态/空间纹理: depends only on the position of each point in 3D space，只和三维空间中的位置有关
class checker_texture : public texture {
public:
  // _scale 缩放因子 缩放棋盘的大小
  checker_texture(double _scale, shared_ptr<texture> _even, shared_ptr<texture> _odd) : inv_scale(1.0 / _scale), even(_even), odd(_odd) {}

  checker_texture(double _scale, color c1, color c2) : inv_scale(1.0 / _scale), even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

  // 空间纹理与uv无关，只和空间坐标p有关
  color value(double u, double v, const point3& p) const override {
    auto xInteger = static_cast<int>(std::floor(inv_scale * p.x()));
    auto yInteger = static_cast<int>(std::floor(inv_scale * p.y()));
    auto zInteger = static_cast<int>(std::floor(inv_scale * p.z()));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even->value(u, v, p) : odd->value(u, v, p);
  }

private:
  double inv_scale;
  shared_ptr<texture> even; // solid_color 返回固定颜色值
  shared_ptr<texture> odd;
};


#endif