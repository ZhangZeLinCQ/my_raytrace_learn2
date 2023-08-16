﻿#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>

class camera {
public:
  double aspect_ratio = 1.0;  // Ratio of image width over height
  int    image_width = 100;  // Rendered image width in pixel count
  int    samples_per_pixel = 10;   // Count of random samples for each pixel
  int    max_depth = 10;   // Maximum number of ray bounces into scene
  color  background;               // Scene background color

  double vfov = 90;              // Vertical view angle (field of view)
  point3 lookfrom = point3(0, 0, -1);  // Point camera is looking from
  point3 lookat = point3(0, 0, 0);   // Point camera is looking at
  vec3   vup = vec3(0, 1, 0);     // Camera-relative "up" direction

  double defocus_angle = 0;  // Variation angle of rays through each pixel
  double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

  void render(const hittable& world) {
    initialize();

    std::ofstream out("image.ppm", std::ios::out | std::ios::binary);
    out << "P6\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = 0; j < image_height; ++j) {// The rows are written out from top to bottom
      std::cerr << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
      for (int i = 0; i < image_width; ++i) { // The pixels are written out in rows with pixels left to right
        color pixel_color(0, 0, 0);
        for (int s = 0; s < samples_per_pixel; ++s) {
          ray r = get_ray(i, j); // get_ray 时 生成随机时间的线
          pixel_color += ray_color(r, max_depth, world);
        }
        write_color6(out, pixel_color, samples_per_pixel);
      }
    }
    out.close();
    std::clog << "\rDone.                 \n";
  }

private:
  int    image_height;    // Rendered image height
  point3 center;          // Camera center
  point3 pixel00_loc;     // Location of pixel 0, 0
  vec3   pixel_delta_u;   // Offset to pixel to the right
  vec3   pixel_delta_v;   // Offset to pixel below
  vec3   u, v, w;         // Camera frame basis vectors
  vec3   defocus_disk_u;  // Defocus disk horizontal radius
  vec3   defocus_disk_v;  // Defocus disk vertical radius

  void initialize() {
    image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    center = lookfrom;

    // Determine viewport dimensions.
    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2 * h * focus_dist;
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
    vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors to the next pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Calculate the camera defocus disk basis vectors.
    auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
  }

  ray get_ray(int i, int j) const {
    // Get a randomly-sampled camera ray for the pixel at location i,j, originating from
    // the camera defocus disk.

    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
    auto pixel_sample = pixel_center + pixel_sample_square();

    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;
    auto ray_time = random_double();

    return ray(ray_origin, ray_direction, ray_time);
  }

  vec3 pixel_sample_square() const {
    // Returns a random point in the square surrounding a pixel at the origin.
    auto px = -0.5 + random_double();
    auto py = -0.5 + random_double();
    return (px * pixel_delta_u) + (py * pixel_delta_v);
  }

  vec3 pixel_sample_disk(double radius) const {
    // Generate a sample from the disk of given radius around a pixel at the origin.
    auto p = radius * random_in_unit_disk();
    return (p[0] * pixel_delta_u) + (p[1] * pixel_delta_v);
  }

  point3 defocus_disk_sample() const {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }

  // 设置递归深度（光线反射次数）
  color ray_color(const ray& r, int depth, const hittable& world) const {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, interval(0.001, infinity), rec)) //0 -> 0.001 solve shadow acne problem
      return background;

    // 渲染击中物体
    ray scattered;
    color attenuation;
    color color_from_emission = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) // 自发光材质不散射光
      return color_from_emission;

    color color_from_scatter = attenuation * ray_color(scattered, depth - 1, world);
    return color_from_emission + color_from_scatter;
  }
};
#endif