/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>

#include "src/opengl_utils/view_port.h"

namespace dexpert {

viewport_t::viewport_t() {
    memset(&vp_, 0, sizeof(vp_));
    vp_[2] = 1;
    vp_[3] = 1;
}

viewport_t::viewport_t(const int *vp) {
    memcpy(vp_, vp, sizeof(vp_));
}

viewport_t::viewport_t(int x, int y, int w, int h) {
    vp_[0] = x;
    vp_[1] = y;
    vp_[2] = w;
    vp_[3] = h;
}

void viewport_t::update() {
    glGetIntegerv(GL_VIEWPORT, vp_);
}

void viewport_t::update(uint32_t w, uint32_t h) {
    vp_[0] = 0;
    vp_[1] = 0;
    vp_[2] = w;
    vp_[3] = h;
}

viewport_t viewport_t::from_gl() {
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    return viewport_t(vp[0], vp[1], vp[2], vp[3]);
}

point_t viewport_t::screen_to_frame_coords(uint32_t video_w, uint32_t video_h, const point_t & screen_coords) const {
    float pixel_zoom = raster_zoom(video_w, video_h);

    if (pixel_zoom) {
        point_t coords(screen_coords);
        point_t raster = raster_coords(video_w, video_h);
        coords.x = (coords.x - raster.x / (2.0f / vp_[2])) / pixel_zoom;
        coords.y = (coords.y - raster.y / (2.0f / vp_[3])) / pixel_zoom;
        return coords;
    }

    return point_t(-1, -1);
}

point_t viewport_t::frame_to_screen_coords(uint32_t video_w, uint32_t video_h, const point_t & screen_coords) const {
    float pixel_zoom = raster_zoom(video_w, video_h);

    if (pixel_zoom) {
        point_t raster = raster_coords(video_w, video_h);
        return point_t(
            (screen_coords.x * pixel_zoom) + (raster.x / (2.0f / vp_[2])),
            (screen_coords.y * pixel_zoom) + (raster.y / (2.0f / vp_[3])));
    }

    return point_t(-1, -1);
}

float viewport_t::raster_zoom(uint32_t video_w, uint32_t video_h) const {
    const int& vp_w = vp_[2];
    const int& vp_h = vp_[3];
    if (vp_w < 1 || vp_h < 1)
        return 0.0;

    double scale;
    double scale_h;
    double scale_w;
    double zoom = 1.0;

    int tmp = video_h;

    scale = vp_w / static_cast<double>(video_w);

    zoom *= scale;
    video_h *= scale;

    if (video_h > vp_h) {
        scale = vp_h / static_cast<double>(video_h);
        zoom *= scale;
    }

    return zoom;
}

point_t viewport_t::raster_coords(uint32_t video_w, uint32_t video_h) const {
    double scale = raster_zoom(video_w, video_h);
    if (scale == 0)
        return point_t(0, 0);
    return point_t(
        (1.0f / vp_[2])  * (vp_[2] - (video_w * scale)),
        (1.0f / vp_[3])  * (vp_[3] - (video_h * scale)));
}

box_t viewport_t::screen_to_frame_coords(uint32_t video_w, uint32_t video_h, const box_t & screen_coords) const {
    return box_t(
        screen_to_frame_coords(video_w, video_h, screen_coords[0]),
        screen_to_frame_coords(video_w, video_h, screen_coords[1]),
        screen_to_frame_coords(video_w, video_h, screen_coords[2]),
        screen_to_frame_coords(video_w, video_h, screen_coords[3])
    );
}

box_t viewport_t::frame_to_screen_coords(uint32_t video_w, uint32_t video_h, const box_t & screen_coords) const {
    return box_t(
        frame_to_screen_coords(video_w, video_h, screen_coords[0]),
        frame_to_screen_coords(video_w, video_h, screen_coords[1]),
        frame_to_screen_coords(video_w, video_h, screen_coords[2]),
        frame_to_screen_coords(video_w, video_h, screen_coords[3])
    );
}

float viewport_t::fit(uint32_t *w, uint32_t *h) const {
    float fx = vp_[2];
    float fy = vp_[3];

    float xscale = fx / *w;
    float scale = fy / *h;

    if (xscale < scale)
        scale = xscale;

    uint32_t ww = *w, hh = *h;

    ww *= scale;
    hh *= scale;

    if (ww > fx) {
        scale *= (fx / ww);
        ww *= scale;
        hh *= scale;
    }

    if (hh > fy) {
        scale *= fy / hh;
        ww *= scale;
        hh *= scale;
    }

    *w = ww;
    *h = hh;

    return (1.0 / scale);
}

}  // namespace dexpert
