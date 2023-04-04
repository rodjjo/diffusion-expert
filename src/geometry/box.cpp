/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#include "src/geometry/box.h"

namespace dexpert {

box_t::box_t() {
}

box_t::box_t(const point_t & p1, const point_t & p2, const point_t & p3, const point_t & p4) : p_({p1, p2, p3, p4}) {
}


box_t box_t::rotated(float angle) const {
    box_t box(*this);
    box.rotate(angle);
    return box;
}

void box_t::rotate(float angle) {
    for (int c = 0; c < 4; ++c) {
        p_[c].rotate(angle);
    }
}

void box_t::translate(float px, float py) {
    for (int i = 0; i < 4; ++i) {
        (*this)[i].x += px;
        (*this)[i].y += py;
    }
}

void box_t::scale(float scale) {
    if (scale != 1.0f) {
        for (int i = 0; i < 4; ++i) {
            p_[i].x *= scale;
            p_[i].y *= scale;
        }
    }
}

point_t box_t::center() {
    point_t r;
    for (int i = 0; i < 4; ++i) {
        r.x += p_[i].x;
        r.y += p_[i].y;
    }
    r.x /= 4;
    r.y /= 4;
    return r;
}

point_t box_t::left_top_violation(float width, float height) {
    point_t coordinate;

    if (p_[0].x < 0)
        coordinate.x = static_cast<int>(-p_[0].x);

    if (p_[0].y < 0)
        coordinate.y = static_cast<int>(-p_[0].y);

    return coordinate;
}

point_t box_t::right_bottom_violation(float width, float height) {
    point_t coordinate;

    if (p_[1].x > width)
        coordinate.x = static_cast<int>(p_[1].x - width);

    if (p_[2].y > height)
        coordinate.y = static_cast<int>(p_[2].y - height);

    return coordinate;
}

point_t box_t::size() {
    return point_t(p_[1].x - p_[0].x, p_[2].y - p_[0].y);
}

box_t box_t::occupied_area() {
    box_t result(*this);
    for (int i = 0; i < 4; ++i) {
        if (p_[i].x > result[1].x)
            result[1].x = p_[i].x;
        if (p_[i].x < result[0].x)
            result[0].x = p_[i].x;
        if (p_[i].y > result[2].y)
            result[2].y = p_[i].y;
        if (p_[i].y < result[0].y)
            result[0].y = p_[i].y;
    }

    result[1].y = result[0].y;
    result[3].x = result[0].x;
    result[3].y = result[2].y;
    result[2].x = result[1].x;

    return result;
}

void box_t::trunc_precision() {
    for (int i = 0; i < 4; ++i) {
        p_[i].trunc_precision();
    }
}

inline float sign(point_t p1, point_t p2, point_t p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}


bool box_t::contours_point(float x, float y) {
    point_t pt;
    pt.x = x;
    pt.y = y;

    bool b1, b2, b3;

    b1 = sign(pt, p_[0], p_[1]) < 0.0f;
    b2 = sign(pt, p_[1], p_[2]) < 0.0f;
    b3 = sign(pt, p_[2], p_[0]) < 0.0f;

    if (b1 == b2 && b2 == b3) {
        return true;
    }

    b1 = sign(pt, p_[0], p_[2]) < 0.0f;
    b2 = sign(pt, p_[2], p_[3]) < 0.0f;
    b3 = sign(pt, p_[3], p_[0]) < 0.0f;

    return (b1 == b2 && b2 == b3);
}

bool box_t::contours_point(const point_t & p) {
    return contours_point(p.x, p.y);
}

}  // namespace dexpert
