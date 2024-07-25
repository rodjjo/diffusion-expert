#include <stdio.h>

#include "simple-ui/win.h"
#include "simple-ui/panel.h"
#include "simple-ui/dragdrop.h"
#include "simple-ui/edit.h"
#include "simple-ui/text_box.h"
#include "simple-ui/label.h"
#include "simple-ui/button.h"

namespace dfe
{
    void run_application() {
        auto win = dfe_ui::window_new(1280, 720, "Stable Diffusion");
        auto panel = std::make_shared<dfe_ui::DragDrop>(65, 50, 180, 180);
        auto panel2 = std::make_shared<dfe_ui::DragDrop>(65, 180 + 51, 180, 180);
        auto panel3 = std::make_shared<dfe_ui::DragDrop>(-5, 25, 180 + 10, 50);

        auto editor = std::make_shared<dfe_ui::TextBox>(65 + 185, 50, 300, 50);
        auto label = std::make_shared<dfe_ui::Label>(65 + 185, 50 + 55, 300, 50, std::wstring(L"Label"));
        auto button = std::make_shared<dfe_ui::Button>(65 + 185, 50 + 55 + 55, 300, 100, std::wstring(L"Button"), dfe_ui::img_24x24_open);
        button->icon_position(dfe_ui::Button::icon_center);
        
        editor->text(std::wstring(L"Editor"));
        editor->outline_color(0xFF0000FF);

        panel3->drag_enabled(true);
        panel2->drop_enabled(true);
        panel->drop_enabled(true);

        auto on_mouse_enter = [](dfe_ui::Component *comp) {
            // printf("Mouse enter %lu\n", comp->tag());
        };

        auto on_mouse_exit = [](dfe_ui::Component *comp) {
            // printf("Mouse exit %lu\n", comp->tag());
        };

        panel->tag(1);
        panel2->tag(2);
        panel3->tag(3);

        panel->set_on_mouse_enter(on_mouse_enter);
        panel2->set_on_mouse_enter(on_mouse_enter);
        panel3->set_on_mouse_enter(on_mouse_enter);

        panel->set_on_mouse_exit(on_mouse_exit);
        panel2->set_on_mouse_exit(on_mouse_exit);
        panel3->set_on_mouse_exit(on_mouse_exit);

        panel2->set_on_accept_drop([c{panel3.get()}](dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c == comp;
        });
        panel->set_on_accept_drop([c{panel3.get()}](dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c == comp;
        });
        panel3->set_on_accept_drag([c1{panel.get()}, c2{panel2.get()}] (dfe_ui::Component *source, dfe_ui::Component *comp) {
            return c1 == comp || c2 == comp;
        });

        panel3->set_on_drag_begin([](dfe_ui::Component *comp) {
            static_cast<dfe_ui::Panel *>(comp)->fg_color(255, 128, 128, 255);
        });

        panel3->set_on_drag_end([](dfe_ui::Component *comp) {
            static_cast<dfe_ui::Panel *>(comp)->fg_color(100, 100, 100, 255);
        });

        panel2->set_on_complete_drop([](dfe_ui::Component *self, dfe_ui::Component *next) {
            self->add(next->share());
        });
        panel->set_on_complete_drop([](dfe_ui::Component *self, dfe_ui::Component *next) {
            self->add(next->share());
        });

        auto turn_green = [](dfe_ui::Component *comp) { 
            static_cast<dfe_ui::Panel *>(comp)->fg_color(128, 255, 128, 255);
        };

        auto turn_gray = [](dfe_ui::Component *comp) { 
            static_cast<dfe_ui::Panel *>(comp)->fg_color(128, 255, 128, 255);
        };

        panel->set_on_drop_begin(turn_green);
        panel2->set_on_drop_begin(turn_green);
        panel->set_on_drop_end(turn_gray);
        panel2->set_on_drop_end(turn_gray);

        panel3->bg_color(255, 100, 100, 255);
        auto l = std::make_shared<dfe_ui::Label>(5, 1,  150, 50, std::wstring(L"Drag me"));
        l->character_size(15);
        panel3->add(l);
        panel->add(panel3);

        win->add(panel);
        win->add(panel2);
        win->add(editor);
        win->add(label);
        win->add(button);

        win->scale(1.0);
        win->run();
    }
} // namespace dfe
