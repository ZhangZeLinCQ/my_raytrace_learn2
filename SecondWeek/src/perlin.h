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

    perm_x = perlin_generate_perm();
    perm_y = perlin_generate_perm();
    perm_z = perlin_generate_perm();
  }

  ~perlin() {
    delete[] ranfloat;
    delete[] perm_x;
    delete[] perm_y;
    delete[] perm_z;
  }

  double noise(const point3& p) const {
    auto i = static_cast<int>(4 * p.x()) & 255;
    auto j = static_cast<int>(4 * p.y()) & 255;
    auto k = static_cast<int>(4 * p.z()) & 255;
    // 根据p坐标获得确定的一个值，然后返回ranfloat中该值对应的随机值
    return ranfloat[perm_x[i] ^ perm_y[j] ^ perm_z[k]];
  }

private:
  static const int point_count = 256;
  double* ranfloat;
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
};

#endif