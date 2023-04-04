/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#ifndef SRC_GEOMETRY_BOX_H_
#define SRC_GEOMETRY_BOX_H_

#include "src/geometry/point.h"

namespace dexpert {

class box_t {
 public:
    box_t();
    box_t(const point_t & p1, const point_t & p2, const point_t & p3, const point_t & p4);
    // return point at index
    point_t & operator[] (int index) {
        return p_[index];
    }

    // return point at index
    const point_t & operator[] (int index) const{
        return p_[index];
    }

    // return a rotated version of the box
    box_t rotated(float angle) const;

    // rotate this box
    void rotate(float angle);

    // move the box coordinates in relation to px and py
    void translate(float px, float py);

    // resize the box
    void scale(float scale);

    // return the center of the box
    point_t center();

    // return border boundary violation (top and left coordinates). The box must not be rotated
    point_t left_top_violation(float width, float height);

    // return border boundary violation (right and bottom coordinates). The box must not be rotated
    point_t right_bottom_violation(float width, float height);

    // return the width and the height of the box. The box must not be rotated
    point_t size();

    // return the occupied area by the box
    box_t occupied_area();

    // trunc floating point of coordinates
    void trunc_precision();

    // return if the coordinates are inside of this box
    bool contours_point(float x, float y);
    bool contours_point(const point_t & p);
  private:
    point_t p_[4];
};

}  // namespace dexpert

#endif // SRC_GEOMETRY_BOX_H_