#include <stdio.h>

#include "simple-ui/win.h"
#include "simple-ui/panel.h"
#include "simple-ui/dragdrop.h"
#include "simple-ui/label.h"
#include "simple-ui/button.h"
#include "simple-ui/arrow_button.h"
#include "simple-ui/scrollbar.h"
#include "simple-ui/progress.h"
#include "simple-ui/listbox.h"
#include "simple-ui/combobox.h"
#include "simple-ui/scrollbox.h"
#include "simple-ui/text_editor.h"

namespace dfe
{
    void run_application() {
        auto win = dfe_ui::window_new(1280, 720, "Stable Diffusion");
        auto scrolb = std::make_shared<dfe_ui::Scrollbox>(win.get(), 50, 50, win->w() / 2, win->h() / 2);
        scrolb->autoscroll(true);
        win->add(scrolb);

        auto panel = std::make_shared<dfe_ui::DragDrop>(win.get(), 65, 50, 180, 180);
        auto panel2 = std::make_shared<dfe_ui::DragDrop>(win.get(), 65, 180 + 51, 180, 180);
        auto panel3 = std::make_shared<dfe_ui::DragDrop>(win.get(), -5, 25, 180 + 10, 50);
        auto panel4 = std::make_shared<dfe_ui::DragDrop>(win.get(), 300, 25, 180 + 10, 50);

        auto editor = std::make_shared<dfe_ui::TextEditor>(win.get(), 65 + 185, 50, 300, 50, dfe_ui::editor_type_t::editor_text);
        auto label = std::make_shared<dfe_ui::Label>(win.get(), 65 + 185, 50 + 55, 300, 50, std::wstring(L"Label"));
        auto button = std::make_shared<dfe_ui::Button>(win.get(), 65 + 185, 50 + 55 + 55, 300, 100, std::wstring(L"Button"), dfe_ui::img_24x24_open);
        auto arrow_button = std::make_shared<dfe_ui::ArrowButton>(win.get(), 65 + 185, 270, 45, 45, dfe_ui::ArrowButton::arrom_right);
        auto scrollbar = std::make_shared<dfe_ui::Scrollbar>(win.get(), 65 + 185, 270 + 50, 280, 45, false);
        auto scrollbar2 = std::make_shared<dfe_ui::Scrollbar>(win.get(), 65 + 185, 270 + 50 + 55, 45, 280, true);
        auto progress = std::make_shared<dfe_ui::ProgressBar>(win.get(), 65 + 185 + 50, 270 + 50 + 55, 280, 45);
        auto listbox = std::make_shared<dfe_ui::Listbox>(win.get(), 65 + 185 + 50, 270 + 110 + 55, 280, 45 * 5);
        auto combo = std::make_shared<dfe_ui::Combobox>(win.get(), 65 + 185 + 310, 50, 300, 35);

        auto edito_wrap = std::make_shared<dfe_ui::TextEditor>(win.get(), combo->x() + combo->w() + 5, 50, 300, 300, dfe_ui::editor_type_t::editor_multiline_wrap);
        edito_wrap->halign(dfe_ui::text_left);
        edito_wrap->vertical_scrollbar(dfe_ui::scrollbar_auto);
        // edito_wrap->horizontal_scrollbar(dfe_ui::scrollbar_allways);
        edito_wrap->content(std::wstring(L"Esse é um texto muito longo com quebra de linha"));

        button->icon_position(dfe_ui::Button::icon_center);
        button->checked_icon(dfe_ui::img_24x24_bee);

        char buffer[1024] = "";
        for (int i = 0; i < 7; i++) {
            sprintf(buffer, "Item %04d", i);
            std::string text(buffer);
            listbox->add(std::wstring(text.begin(), text.end()));
        }
        for (int i = 0; i < 16; i++) {
            sprintf(buffer, "Item %04d", i);
            std::string text(buffer);
            combo->add(std::wstring(text.begin(), text.end()));
        }
        

        progress->progress(45);

        button->onclick([comp{panel4.get()}] (dfe_ui::Component *self){
            comp->float_on();
        });
        
        editor->content(std::wstring(L"Editor"));
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
        auto l = std::make_shared<dfe_ui::Label>(win.get(), 5, 1,  150, 50, std::wstring(L"Drag me"));
        l->character_size(15);
        panel3->add(l);
        panel->add(panel3);

        scrolb->add(panel);
        scrolb->add(panel2);
        scrolb->add(editor);
        scrolb->add(label);
        scrolb->add(button);
        scrolb->add(arrow_button);
        scrolb->add(scrollbar);
        scrolb->add(scrollbar2);
        scrolb->add(progress);
        scrolb->add(listbox);
        scrolb->add(combo);
        scrolb->add(edito_wrap);

        win->scale(1.0);

        win->run();
    }
} // namespace dfe

