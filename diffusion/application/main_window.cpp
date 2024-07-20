#include "simple-ui/win.h"
#include "simple-ui/panel.h"
#include "simple-ui/dragdrop.h"

namespace dfe
{
    void run_application() {
        auto win = dfe_ui::window_new(640, 480, "Stable Diffusion");
        auto panel = std::make_shared<dfe_ui::DragDrop>(65, 50, 180, 180);
        auto panel2 = std::make_shared<dfe_ui::DragDrop>(-5, 25, 180 + 10, 50);
        auto panel3 = std::make_shared<dfe_ui::DragDrop>(65, 180 + 51, 180, 180);
        
        panel3->drag_enabled(true);
        panel2->drop_enabled(true);
        panel->drop_enabled(true);

        panel2->set_on_accept_drop([c{panel3.get()}](dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c == comp;
        });
        panel->set_on_accept_drop([c{panel3.get()}](dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c == comp;
        });
        panel3->set_on_accept_drag([c1{panel.get()}, c2{panel2.get()}] (dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c1 == comp || c2 == comp;
        });

        panel2->bg_color(255, 100, 100, 255);
        panel->add(panel2);
        win->add(panel);
        win->add(panel3);
        win->scale(1.0);
        win->run();
    }
} // namespace dfe
