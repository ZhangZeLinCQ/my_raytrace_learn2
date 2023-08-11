#ifndef PERLIN_H
#define PERLIN_H

#include "common.h"

class perlin {
public:
  perlin() {
    ranfloat = new double[point_count];
    for (int i = 0; i < point_count; ++i) {
      ranfloat[i] = random_double();
    } // 首先生成point_count个(0-1)的double随机数

    ranvec = new vec3[point_count]; // 使用随机数组而不是float
    for (int i = 0; i < point_count; ++i) {
      ranvec[i] = unit_vector(vec3::random(-1, 1));
    }

    perm_x = perlin_generate_perm();
    perm_y = perlin_generate_perm();
    perm_z = perlin_generate_perm();
  }

  ~perlin() {
    delete[] ranfloat;
    delete[] ranvec;
    delete[] perm_x;
    delete[] perm_y;
    delete[] perm_z;
  }

  double noise(const point3& p) const {
    auto i = static_cast<int>(p.x()) & 255;
    auto j = static_cast<int>(p.y()) & 255;
    auto k = static_cast<int>(p.z()) & 255;
    // 根据p坐标获得确定的一个值，然后返回ranfloat中该值对应的随机值
    return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
  }

  double noise_tri_interpolation(const point3& p) const {
    // 获取小数部分
    auto u = p.x() - floor(p.x());
    auto v = p.y() - floor(p.y());
    auto w = p.z() - floor(p.z());
    // 向下取整
    auto i = static_cast<int>(floor(p.x()));
    auto j = static_cast<int>(floor(p.y()));
    auto k = static_cast<int>(floor(p.z()));
    double c[2][2][2];

    // 生成一组确定的8相值
    for (int di = 0; di < 2; di++)
      for (int dj = 0; dj < 2; dj++)
        for (int dk = 0; dk < 2; dk++)
          c[di][dj][dk] = ranfloat[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

    return trilinear_interp(c, u, v, w);
  }

  double noise_Hermite(const point3& p) const {
    // 获取小数部分
    auto u = p.x() - floor(p.x());
    auto v = p.y() - floor(p.y());
    auto w = p.z() - floor(p.z());
    // Hermite cubic 消除了普通三线性插值会产生的网格纹路
    u = u * u * (3 - 2 * u);
    v = v * v * (3 - 2 * v);
    w = w * w * (3 - 2 * w);

    // 向下取整
    auto i = static_cast<int>(floor(p.x()));
    auto j = static_cast<int>(floor(p.y()));
    auto k = static_cast<int>(floor(p.z()));
    double c[2][2][2];

    // 生成一组确定的8相值
    for (int di = 0; di < 2; di++)
      for (int dj = 0; dj < 2; dj++)
        for (int dk = 0; dk < 2; dk++)
          c[di][dj][dk] = ranfloat[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

    return trilinear_interp(c, u, v, w);
  }

  double noise_Perlin(const point3& p) const {
    // 获取小数部分
    auto u = p.x() - floor(p.x());
    auto v = p.y() - floor(p.y());
    auto w = p.z() - floor(p.z());
    // Hermite cubic 消除了普通三线性插值会产生的网格纹路
    u = u * u * (3 - 2 * u);
    v = v * v * (3 - 2 * v);
    w = w * w * (3 - 2 * w);

    // 向下取整
    auto i = static_cast<int>(floor(p.x()));
    auto j = static_cast<int>(floor(p.y()));
    auto k = static_cast<int>(floor(p.z()));
    vec3  c[2][2][2];

    // 生成一组确定的8相值
    for (int di = 0; di < 2; di++)
      for (int dj = 0; dj < 2; dj++)
        for (int dk = 0; dk < 2; dk++)
          c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];

    return perlin_interp(c, u, v, w);
  }

  // 湍流Turbulence，噪音的混合
  double turb(const point3& p, int depth = 7) const {
    auto accum = 0.0;
    auto temp_p = p;
    auto weight = 1.0;

    for (int i = 0; i < depth; i++) {
      accum += weight * noise_Perlin(temp_p);
      weight *= 0.5;
      temp_p *= 2;
    }

    return fabs(accum);
  }

private:
  static const int point_count = 256;
  double* ranfloat;
  vec3* ranvec;
  int* perm_x;
  int* perm_y;
  int* perm_z;

  static int* perlin_generate_perm() {
    auto p = new int[point_count];

    for (int i = 0; i < perlin::point_count; i++)
      p[i] = i;

    permute(p, point_count);

    return p;
  }

  // 洗牌算法
  static void permute(int* p, int n) { 
    for (int i = n - 1; i > 0; i--) { // 从后向前随机交换之前某个位置的数值
      int target = random_int(0, i);
      int tmp = p[i];
      p[i] = p[target];
      p[target] = tmp;
    }
  }

  // 三线性（三维线性）插值
  static double trilinear_interp(double c[2][2][2], double u, double v, double w) {
    auto accum = 0.0;
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++)
          accum += (i * u + (1 - i) * (1 - u)) * (j * v + (1 - j) * (1 - v)) * (k * w + (1 - k) * (1 - w)) * c[i][j][k];
    return accum;
  }

  // Ken Perlin 插值
  static double perlin_interp(vec3 c[2][2][2], double u, double v, double w) {
    auto uu = u * u * (3 - 2 * u);
    auto vv = v * v * (3 - 2 * v);
    auto ww = w * w * (3 - 2 * w);
    auto accum = 0.0;

    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++) {
          vec3 weight_v(u - i, v - j, w - k);
          accum += (i * uu + (1 - i) * (1 - uu))
            * (j * vv + (1 - j) * (1 - vv))
            * (k * ww + (1 - k) * (1 - ww))
            * dot(c[i][j][k], weight_v);
        }

    return accum;
  }
};

#endif