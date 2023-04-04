/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_GEOMETRY_POINT_H_
#define SRC_GEOMETRY_POINT_H_

#define PI 3.1415926535897
#define DEGS (PI / 180.0)

namespace dexpert {

class point_t {
 public:
    point_t() : x(0), y(0) {}

    point_t(float xx, float yy) : x(xx), y(yy) {}

    // translate this coordinate to base axis
    point_t to_axis(float px, float py);

    // rotate this coordindate
    void rotate(float angle);

    // return a rotated version of this coordinate
    point_t rotated(float angle) const;

    // truncate value to int precision
    void trunc_precision();

    // return the distance to other point
    float distance_to(const point_t& target);

    // return the distance to other point
    float distance_to(float x, float y);

    // return normalized angle of the coordindate (return angle in 0 to 360 interval)
    float angle_0_360();

    // return angle of the coordindate (may return negative value)
    float angle();

 public:
    float x;
    float y;
};

}  // namespace dexpert

#endif  // SRC_GEOMETRY_POINT_H_