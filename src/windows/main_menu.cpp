/*
 * Copyright (C) 2023 by Rodrigo Antonio de Araujo
 */
#include <FL/Fl.H>
#include "src/windows/main_menu.h"

namespace dexpert {

MainMenu::MainMenu(int w, int h, callback_t callback) : Fl_Menu_Bar(0, 0, w, h) {
    callback_ = callback;
}

MainMenu::~MainMenu() {
}

int MainMenu::handle(int value) {
    if (value == FL_PUSH && callback_ && Fl::event_button() == FL_LEFT_MOUSE) {
        callback_();
    }

    return Fl_Menu_Bar::handle(value);
}

void MainMenu::menuItemCb(Fl_Widget *widget, void *user_data) {
    widget->deactivate();
    static_cast<menu_item_t*>(user_data)->callback();
    widget->activate();
}

void MainMenu::addItem(callback_t callback, const char *path, const char* label, const char* shortcut, int flags, xpm::xpm_t icon) {
    std::string p(path);
    p += label;

    auto menuItem = std::make_shared<menu_item_t>();
    menuItem->path = p;
    menuItem->callback = callback;

    items_.push_back(menuItem);
    this->add(menuItem->path.c_str(), shortcut, MainMenu::menuItemCb, menuItem.get(), flags);

    if (icon == xpm::no_image) {
        return;
    }

    auto flItem = const_cast<Fl_Menu_Item *>(this->find_item((menuItem->path.c_str())));

    menuItem->icon = xpm::image(icon);
    flItem->image(menuItem->icon.get());

    size_t lp = p.find_last_of('/');
    std::string lbl;
    if (lp != std::string::npos) {
        lbl = p.substr(lp + 1);
    } else {
        lbl = p;
    }

    menuItem->text = std::string(" ") + lbl;  // flItem->text;
    

    Fl_Multi_Label &ml = menuItem->label;

    ml.typea = _FL_IMAGE_LABEL;
    ml.labela = reinterpret_cast<char *>(menuItem->icon.get());
    ml.typeb = FL_NORMAL_LABEL;
    ml.labelb =  menuItem->text.c_str();
    ml.label(flItem);

    char buffer[2048] = "";
    if (this->item_pathname(buffer, sizeof(buffer) - 1, flItem) == 0) {
        menuItem->path = buffer;
    }
}

void MainMenu::enablePath(const char *path, bool enabled) {
  auto item = component(path);
  if (item != NULL) {

  }
}

Fl_Menu_Item *MainMenu::component(const std::string& path) {
    return const_cast<Fl_Menu_Item *>(find_item((path.c_str())));
}

}  // namespace dexpert
