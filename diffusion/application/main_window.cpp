#include "simple-ui/win.h"
#include "simple-ui/panel.h"

namespace dfe
{
    void run_application() {
        auto win = dfe_ui::window_new(640, 480, "Stable Diffusion");
        auto panel = std::make_shared<dfe_ui::Panel>(65, 50, 180, 180);
        auto panel2 = std::make_shared<dfe_ui::Panel>(-5, 25, 180 + 10, 50);
        panel2->bg_color(255, 100, 100, 255);
        panel->add(panel2);
        win->add(panel);
        win->run();
    }
} // namespace dfe
