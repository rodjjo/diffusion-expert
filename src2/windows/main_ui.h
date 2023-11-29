#pragma once

#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Select_Browser.H>

#include "components/main_menu.h"
#include "components/image_panel.h"
#include "components/button.h"

namespace dfe
{

class MainWindow : public Fl_Menu_Window, public SubscriberThis {
public:
    MainWindow();
    virtual ~MainWindow();
    static int dfe_run();
    static void dfe_close();
    static void dfe_stop();
    static void dfe_showProgress();
    static void dfe_hideProgress();
    static void dfe_show_error(const char *message);

protected:
    void resize(int x, int y, int w, int h) override;
    int handle(int event) override;
    void alignComponents();
    
private:
    static void layer_cb(Fl_Widget* widget, void *cbdata);
    void layer_cb(Fl_Widget* widget);
    void dfe_handle_event(void *sender, event_id_t id, void *data) override;
    void choose_file_and_open(bool clear_layers);
    void remove_selected_layer();
    void clear_layers();
    void update_layer_list();
private:
    int16_t wheel_correct_delta_ = 0;
    uint16_t correct_keys_;
    Fl_Group *menuPanel_ = NULL;
    MainMenu *menu_ = NULL;
    std::unique_ptr<Button> removeLayer_;
    std::unique_ptr<Button> removeAllLayers_;
    ImagePanel *image_;
    Fl_Select_Browser *layers_;
    bool in_layer_callback_ = false;
};

} // namespace dfe
