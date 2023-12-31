﻿#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include "rtw_stb_image.h"
#include "perlin.h"

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


// 以图象为材质
class image_texture : public texture {
public:
  image_texture(const char* filename) : image(filename) {}

  color value(double u, double v, const point3& p) const override {
    // If we have no texture data, then return solid cyan as a debugging aid.
    if (image.height() <= 0) return color(0, 1, 1);

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = interval(0, 1).clamp(u);
    v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

    auto i = static_cast<int>(u * image.width());
    auto j = static_cast<int>(v * image.height());
    auto pixel = image.pixel_data(i, j); // 通过uv获取图片上对应位置的rgb[0-255]值

    auto color_scale = 1.0 / 255.0;
    return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
  }

private:
  rtw_image image;
};

// 以噪声图像作为纹理
class noise_texture : public texture {
public:
  noise_texture() {}

  noise_texture(double sc = 4) : scale(sc) {}

  color value(double u, double v, const point3& p) const override {
    auto s = scale * p;
    //return color(1, 1, 1) * noise.noise(s);
    //return color(1, 1, 1) * noise.noise_Hermite(s);
    // 柏林噪声返回可能为负值，所以矫正为正数
    //return color(1, 1, 1) * 0.5 * (1.0 + noise.noise_Perlin(s));
    //return color(1, 1, 1) * noise.turb(s);
    return color(1, 1, 1) * 0.5 * (1 + sin(s.z() + 10 * noise.turb(s)));
  }

private:
  perlin noise;
  double scale;
};


#endif