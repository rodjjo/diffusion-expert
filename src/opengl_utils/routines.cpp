#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/gl.h>

#include "src/config/config.h"


namespace dexpert
{


void blur_gl_contents(int w, int h, int mouse_x, int mouse_y) {
    if (!dexpert::getConfig().getPrivacyMode()) {
        return;
    }

    const int half_area =  25;
    float fx = 2.0 / w;
    float fy = 2.0 / h;
    float hx = half_area * fx;
    float hy = half_area * fy;
    float x1 = (mouse_x * fx - hx) - 1.0;
    float y1 = 1.0 - (mouse_y * fy - hy);
    float x2 = (mouse_x * fx + hx) - 1.0;
    float y2 = 1.0 - (mouse_y * fy + hy);

    glColor4f(0.2, 0.2, 0.2, 1.0);
    glBegin(GL_QUADS);

    // left
    glVertex2f(-1.0, 1.0);
    glVertex2f(x1, 1.0);
    glVertex2f(x1, -1.0);
    glVertex2f(-1.0, -1.0);
    
    // right
    glVertex2f(1.0, 1.0);
    glVertex2f(x2, 1.0);
    glVertex2f(x2, -1.0);
    glVertex2f(1.0, -1.0);

    // center top
    glVertex2f(x1, y1);
    glVertex2f(x1, 1.0);
    glVertex2f(x2, 1.0);
    glVertex2f(x2, y1);

    // center bottom
    glVertex2f(x1, y2);
    glVertex2f(x2, y2);
    glVertex2f(x2, -1.0);
    glVertex2f(x1, -1.0);

    glEnd();
}

    
} // namespace dexpert
