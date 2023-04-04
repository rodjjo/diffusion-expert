/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#include <cmath>
#include "src/geometry/point.h"

#define ALMOST_INFINITE 99999999

namespace dexpert {

point_t point_t::to_axis(float px, float py) {
    return point_t(x - px, y - py);
}

void point_t::rotate(float angle) {
    if (angle) {
        float temp = 0;
        float s = sin(angle * DEGS);
        float c = cos(angle * DEGS);
        temp = x;
        x = x * c - y * s;
        y = temp * s + y * c;
    }
}

point_t point_t::rotated(float angle) const{
    point_t p(*this);
    p.rotate(angle);
    return p;
}

void point_t::trunc_precision() {
    x = static_cast<int>(x);
    y = static_cast<int>(y);
}

float point_t::distance_to(const point_t& target) {
    return distance_to(target.x, target.y);
}

float point_t::distance_to(float x, float y) {
    float dx = x - this->x;
    float dy = y - this->y;
    dx = dx * dx;
    dy = dy * dy + dx;
    if (dy > 0)
        return sqrt(dy);
    return ALMOST_INFINITE;
}

float point_t::angle_0_360() {
    float a = angle();
    while (a < 0) a += 360;
    while (a > 360) a -= 360;
    return a;
}

float point_t::angle() {
    return (atan2(y, x) * (180.0 / PI)) - 90;
}

}  // namespace dexpert
